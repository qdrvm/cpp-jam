#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>
#include <client/TracyScoped.hpp>
#include <tracy/Tracy.hpp>

#include <morum/archive_backend.hpp>
#include <morum/common.hpp>
#include <morum/db.hpp>
#include <morum/merkle_tree.hpp>

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

qtils::ByteVec random_vector(size_t min_size = 1, size_t max_size = 128) {
  std::uniform_int_distribution<size_t> dist(min_size, max_size);
  size_t size = dist(rand_engine);

  qtils::ByteVec v(size);
  fill_random(v);
  return v;
}

std::unique_ptr<morum::ArchiveTrieDb> trie_db;
morum::Hash32 last_root{};

static void BM_SetGet(benchmark::State &state) {
  rand_engine.seed(42);
  constexpr int INSERTION_NUM = 1000;

  for (auto _ : state) {
    ZoneNamedN(loop_zone, "loop", true);

    auto tree = trie_db->load_tree(last_root).value().value();

    std::vector<std::pair<morum::Hash32, qtils::ByteVec>> insertions;
    for (int i = 0; i < INSERTION_NUM; i++) {
      insertions.emplace_back(random_hash(), random_vector());
    }
    {
      ZoneNamedN(setter_zone, "set", true);
      for (auto &[k, v] : insertions) {
        tree->set(k, qtils::ByteVec{v}).value();
      }
    }
    {
      ZoneNamedN(getter_zone, "get", true);

      for (auto &[k, v] : insertions) {
        tree->get(k).value();
      }
    }
    {
      ZoneNamedN(calculate_hash_zone, "calculate_hash", true);
      trie_db->get_root_and_store(*tree).value();
    }

    FrameMark;
  }
}

BENCHMARK(BM_SetGet);

template <typename F>
struct FinalAction {
  ~FinalAction() {
    f();
  }

  F f;
};

int main(int argc, char **argv) {
  char arg0_default[] = "benchmark";
  char *args_default = arg0_default;
  if (!argv) {
    argc = 1;
    argv = &args_default;
  }

  auto db = std::shared_ptr{morum::open_db("./test_db").value()};
  FinalAction cleanup{[]() { std::filesystem::remove_all("./test_db"); }};

  trie_db = std::make_unique<morum::ArchiveTrieDb>(db);

  auto tree = trie_db->empty_tree();
  constexpr int INSERTION_NUM = 5000;
  {
    ZoneNamedN(setter_zone, "initial insertions", true);

    for (int i = 0; i < INSERTION_NUM; i++) {
      tree->set(random_hash(), qtils::ByteVec{random_vector()}).value();
    }
  }
  last_root = trie_db->get_root_and_store(*tree).value();
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();
  return 0;
}
