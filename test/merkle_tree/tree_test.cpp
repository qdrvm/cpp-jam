#include <algorithm>
#include <array>
#include <chrono>
#include <expected>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <random>
#include <ranges>
#include <span>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <rocksdb/iostats_context.h>
#include <rocksdb/perf_context.h>
#include <rocksdb/perf_level.h>
#include <rocksdb/statistics.h>
#include <client/TracyScoped.hpp>
#include <qtils/assert.hpp>
#include <qtils/bytes.hpp>
#include <tracy/Tracy.hpp>

#include <morum/archive_backend.hpp>
#include <morum/common.hpp>
#include <morum/merkle_tree.hpp>
#include <morum/nomt_backend.hpp>
#include <morum/storage_adapter.hpp>
#include <morum/tree_node.hpp>

void *operator new(std::size_t count) {
  auto ptr = malloc(count);
  TracyAlloc(ptr, count);
  return ptr;
}
void operator delete(void *ptr) noexcept {
  TracyFree(ptr);
  free(ptr);
}
void operator delete(void *ptr, std::size_t) noexcept {
  TracyFree(ptr);
  free(ptr);
}

constexpr unsigned seed = 42;
static std::mt19937_64 rand_engine{seed};

template <std::ranges::input_range R>
void fill_random(R &&span) {
  static std::uniform_int_distribution dist;

  for (auto &byte : span) {
    byte = dist(rand_engine);
  }
}

morum::Hash32 random_hash() {
  morum::Hash32 hash;
  fill_random(hash);
  return hash;
}

morum::ByteVector random_vector(size_t min_size = 1, size_t max_size = 128) {
  std::uniform_int_distribution<size_t> dist(min_size, max_size);
  size_t size = dist(rand_engine);

  morum::ByteVector v(size);
  fill_random(v);
  return v;
}

template <typename Duration>
struct NiceDuration : Duration {};

template <typename Duration>
struct std::formatter<NiceDuration<Duration>> {
  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    auto it = ctx.begin();
    return it;
  }

  template <class FmtContext>
  FmtContext::iterator format(
      NiceDuration<Duration> dur, FmtContext &ctx) const {
    auto out = ctx.out();

    if (auto n = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
        n > 10) {
      std::format_to(out, "{}s", n);
      return out;
    }
    if (auto n =
            std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
        n > 10) {
      std::format_to(out, "{}ms", n);
      return out;
    }
    if (auto n =
            std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
        n > 10) {
      std::format_to(out, "{}us", n);
      return out;
    }
    std::format_to(out, "{}ns", dur.count());
    return out;
  }
};

int main() {
  auto db = morum::open_db("/tmp/qdrvm-test-db").value();

  rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);

  auto tree_db = std::make_shared<morum::RocksDbFamilyAdapter>(
      *db, morum::ColumnFamily::TREE_NODE);
  auto value_db = std::make_shared<morum::RocksDbFamilyAdapter>(
      *db, morum::ColumnFamily::TREE_VALUE);
  auto page_db = std::make_shared<morum::RocksDbFamilyAdapter>(
      *db, morum::ColumnFamily::TREE_PAGE);
  morum::RocksDbFamilyAdapter flat_db{*db, morum::ColumnFamily::FLAT_KV};

  using Clock = std::chrono::steady_clock;
  using Dur = NiceDuration<Clock::duration>;
  struct PerStepStats {
    Dur node_reads_duration{};
    Dur value_reads_duration{};
    Dur writes_in_batch_duration{};
    Dur batch_write_duration{};
    Dur total_duration{};
    size_t new_nodes_written{};
    size_t new_values_written{};
    uint64_t new_values_size{};
  };

  std::vector<PerStepStats> stats{};

  constexpr int STEPS_NUM = 100;
  constexpr int INSERTION_NUM = 10000;

  stats.resize(STEPS_NUM);

  morum::Hash32 previous_root{};

  auto db_stats = rocksdb::CreateDBStatistics();

  std::unordered_map<morum::Hash32, morum::TreeNode> cached_nodes;

  struct TotalTime {
    Clock::duration nomt{};
    Clock::duration archive{};
  };
  std::vector<TotalTime> totals(STEPS_NUM);

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // NOMT
  /////////////////////////////////////////////////////////////////////////////////////////////////
  {
    morum::NomtDb nomt{
        std::make_shared<morum::NearlyOptimalNodeStorage>(page_db), value_db};

    for (int step = 0; step < STEPS_NUM; step++) {
      auto total_start = Clock::now();
      std::unique_ptr<morum::MerkleTree> tree;

      if (previous_root == morum::Hash32{}) {
        tree = nomt.empty_tree();
      } else {
        tree = std::move(nomt.load_tree(previous_root)->value());
      }
      std::vector<std::pair<morum::Hash32, morum::ByteVector>> insertions;
      for (int i = 0; i < INSERTION_NUM; i++) {
        insertions.emplace_back(random_hash(), random_vector());
      }
      for (auto &[k, v] : insertions) {
        [[maybe_unused]] auto res = tree->set(k, morum::ByteVector{v});
        QTILS_ASSERT_HAS_VALUE(res);
      }
      for (auto &[k, v] : insertions) {
        auto res_opt = tree->get(k);
        QTILS_ASSERT_HAS_VALUE(res_opt);
        QTILS_ASSERT(res_opt.value().has_value());
        QTILS_ASSERT_RANGE_EQ(res_opt.value().value(), v);
      }
      previous_root = nomt.get_root_and_store(*tree).value();
      std::println("{} - {} - total_duration: {}",
          step,
          previous_root,
          NiceDuration(Clock::now() - total_start));
      totals[step].nomt = Clock::now() - total_start;
    }
  }

  rand_engine.seed(42);

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Archive
  /////////////////////////////////////////////////////////////////////////////////////////////////

  previous_root = morum::ZeroHash32;
  morum::ArchiveTrieDb archive_db{tree_db, value_db};
  for (int step = 0; step < STEPS_NUM; step++) {
    ZoneNamedN(loop_zone, "loop", true);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    auto total_start = Clock::now();

    std::unique_ptr<morum::MerkleTree> tree;
    if (previous_root == morum::ZeroHash32) {
      tree = archive_db.empty_tree();
    } else {
      tree = archive_db.load_tree(previous_root).value().value();
    }
    std::vector<std::pair<morum::Hash32, morum::ByteVector>> insertions;
    for (int i = 0; i < INSERTION_NUM; i++) {
      insertions.emplace_back(random_hash(), random_vector());
    }
    {
      ZoneNamedN(setter_zone, "set", true);
      for (auto &[k, v] : insertions) {
        [[maybe_unused]] auto res = tree->set(k, morum::ByteVector{v});
        QTILS_ASSERT_HAS_VALUE(res);
      }
    }
    {
      ZoneNamedN(getter_zone, "get", true);

      for (auto &[k, v] : insertions) {
        auto res_opt = tree->get(k);
        QTILS_ASSERT_HAS_VALUE(res_opt);
        QTILS_ASSERT(res_opt.value().has_value());
        QTILS_ASSERT_RANGE_EQ(res_opt.value().value(), v);
      }
    }
    auto node_batch = tree_db->start_batch();
    auto value_batch = value_db->start_batch();
    morum::Hash32 hash;
    {
      ZoneNamedN(calculate_hash_zone, "calculate_hash", true);
      hash = tree->calculate_hash([&](const morum::TreeNode &n,
                                      qtils::ByteSpan serialized,
                                      qtils::ByteSpan hash,
                                      qtils::BitSpan<>) {
        morum::Hash32 hash_copy;
        std::ranges::copy(hash, hash_copy.begin());
        hash_copy[0] &= 0xFE;
        auto start = Clock::now();
        [[maybe_unused]] auto res = node_batch->write(hash_copy, serialized);
        if (n.is_leaf()) {
          cached_nodes.emplace(hash_copy, n);
        } else {
          auto &branch = n.as_branch();
          QTILS_ASSERT(!branch.get_left_idx().has_value()
              || branch.get_left_hash().has_value()
                  == branch.get_left_idx().has_value());
          QTILS_ASSERT(!branch.get_right_idx().has_value()
              || branch.get_right_hash().has_value()
                  == branch.get_right_idx().has_value());
          // original node may contain child node ids which are not persistent
          morum::Branch b{branch.get_left_hash(), branch.get_right_hash()};
          cached_nodes.emplace(hash_copy, b);
        }
        QTILS_ASSERT(res);
        stats[step].new_nodes_written++;
        if (n.is_leaf()) {
          auto h_or_v = n.as_leaf().hash_or_value();
          if (auto *hash = std::get_if<morum::HashRef>(&h_or_v); hash) {
            if (auto value_opt = tree->get_cached_value(hash->get());
                value_opt) {
              [[maybe_unused]] auto res =
                  value_batch->write(hash->get(), value_opt.value());
              stats[step].new_values_written++;
              stats[step].new_values_size += value_opt.value().size_bytes();
              QTILS_ASSERT(res);
            }
          }
        }
        stats[step].writes_in_batch_duration += Clock::now() - start;

        [[maybe_unused]] auto deserialized =
            morum::deserialize_node(serialized);
        if (n.is_branch()) {
          QTILS_ASSERT(deserialized.is_branch());
          QTILS_ASSERT_EQ(n.as_branch().get_left_hash(),
              deserialized.as_branch().get_left_hash());
          QTILS_ASSERT_EQ(n.as_branch().get_right_hash(),
              deserialized.as_branch().get_right_hash());
        } else {
          QTILS_ASSERT(deserialized.is_leaf());
        }
      });
    }
    hash[0] &= 0xFE;
    previous_root = hash;

    auto start = Clock::now();
    [[maybe_unused]] auto res = tree_db->write_batch(std::move(node_batch));
    QTILS_ASSERT_HAS_VALUE(res);
    [[maybe_unused]] auto res2 = value_db->write_batch(std::move(value_batch));
    QTILS_ASSERT_HAS_VALUE(res2);
    stats[step].batch_write_duration = Dur{Clock::now() - start};

    stats[step].total_duration += Clock::now() - total_start;

    std::println(
        "\r{} - {}, {}, {} nodes and {} values written, {} bytes of values, "
        "{} "
        "bytes of nodes",
        step,
        hash,
        stats[step].total_duration,
        stats[step].new_nodes_written,
        stats[step].new_values_written,
        stats[step].new_values_size,
        stats[step].new_nodes_written * sizeof(morum::Leaf));
    std::println("total_duration: {}", stats[step].total_duration);
    totals[step].archive = stats[step].total_duration;

    // std::println("node_reads_duration: {}",
    // stats[step].node_reads_duration); std::println("value_reads_duration:
    // {}", stats[step].value_reads_duration);
    // std::println("writes_in_batch_duration: {}",
    //              stats[step].writes_in_batch_duration);
    // std::println("{} - batch_write_duration: {}",
    //              step,
    //              stats[step].batch_write_duration);

    // std::println("{}", rocksdb::get_perf_context()->ToString(true));
    // std::println("{}", rocksdb::get_iostats_context()->ToString());
    std::println(
        "{}", "=========================================================");
    FrameMark;
  }
  PerStepStats avg{};
  PerStepStats max{};
  PerStepStats min{
      {Dur::max()},
      {Dur::max()},
      {Dur::max()},
      {Dur::max()},
      {Dur::max()},
  };

  for (auto &stat : stats) {
    avg.total_duration += stat.total_duration;
    avg.batch_write_duration += stat.batch_write_duration;
    avg.node_reads_duration += stat.node_reads_duration;
    avg.value_reads_duration += stat.value_reads_duration;
    avg.writes_in_batch_duration += stat.writes_in_batch_duration;

    max.total_duration = std::max(stat.total_duration, max.total_duration);
    max.batch_write_duration =
        std::max(stat.batch_write_duration, max.batch_write_duration);
    max.node_reads_duration =
        std::max(stat.node_reads_duration, max.node_reads_duration);
    max.value_reads_duration =
        std::max(stat.value_reads_duration, max.value_reads_duration);
    max.writes_in_batch_duration =
        std::max(stat.writes_in_batch_duration, max.writes_in_batch_duration);

    min.total_duration = std::min(stat.total_duration, min.total_duration);
    min.batch_write_duration =
        std::min(stat.batch_write_duration, min.batch_write_duration);
    min.node_reads_duration =
        std::min(stat.node_reads_duration, min.node_reads_duration);
    min.value_reads_duration =
        std::min(stat.value_reads_duration, min.value_reads_duration);
    min.writes_in_batch_duration =
        std::min(stat.writes_in_batch_duration, min.writes_in_batch_duration);
  }
  avg.total_duration /= stats.size();
  avg.batch_write_duration /= stats.size();
  avg.node_reads_duration /= stats.size();
  avg.value_reads_duration /= stats.size();
  avg.writes_in_batch_duration /= stats.size();

  // std::println("Avg:");
  // std::println("total_duration: {}", avg.total_duration);
  // std::println("batch_write_duration: {}", avg.batch_write_duration);
  // std::println("node_reads_duration: {}", avg.node_reads_duration);
  // std::println("value_reads_duration: {}", avg.value_reads_duration);
  // std::println("writes_in_batch_duration: {}",
  // avg.writes_in_batch_duration);

  // std::println("Max:");
  // std::println("total_duration: {}", max.total_duration);
  // std::println("batch_write_duration: {}", max.batch_write_duration);
  // std::println("node_reads_duration: {}", max.node_reads_duration);
  // std::println("value_reads_duration: {}", max.value_reads_duration);
  // std::println("writes_in_batch_duration: {}",
  // max.writes_in_batch_duration);

  // std::println("Min:");
  // std::println("total_duration: {}", min.total_duration);
  // std::println("batch_write_duration: {}", min.batch_write_duration);
  // std::println("node_reads_duration: {}", min.node_reads_duration);
  // std::println("value_reads_duration: {}", min.value_reads_duration);
  // std::println("writes_in_batch_duration: {}",
  // min.writes_in_batch_duration);

  for (int i = 0; i < STEPS_NUM; i++) {
    std::println("#{}: NOMT {} vs Archive {}: {:.1f}%",
        i,
        NiceDuration(totals[i].nomt),
        NiceDuration(totals[i].archive),
        100.0 * (double)totals[i].nomt.count()
            / (double)totals[i].archive.count());
  }
  db.reset();
}
