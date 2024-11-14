/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <morum/db.hpp>

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/status.h>
#include <rocksdb/write_batch.h>
#include <qtils/assert.hpp>
#include <qtils/macro.hpp>

namespace morum {

  template <typename T>
  std::expected<T, StorageError> wrap_status(
      T &&t, const rocksdb::Status &status) {
    if (!status.ok()) {
      return std::unexpected(StorageError{status.ToString()});
    }
    return t;
  }

  std::expected<void, StorageError> wrap_status(const rocksdb::Status &status) {
    if (!status.ok()) {
      return std::unexpected(StorageError{status.ToString()});
    }
    return {};
  }

  std::string_view to_string_nocheck(ColumnFamilyId family) {
    switch (family) {
      case ColumnFamilyId::DEFAULT:
        return rocksdb::kDefaultColumnFamilyName;
      case ColumnFamilyId::TREE_NODE:
        return "tree_node";
      case ColumnFamilyId::TREE_VALUE:
        return "tree_value";
      case ColumnFamilyId::FLAT_KV:
        return "flat_kv";
      case ColumnFamilyId::TREE_PAGE:
        return "tree_page";
    }
    std::unreachable();
  }

  std::optional<std::string_view> to_string(ColumnFamilyId family) {
    switch (family) {
      case ColumnFamilyId::DEFAULT:
        return rocksdb::kDefaultColumnFamilyName;
      case ColumnFamilyId::TREE_NODE:
        return "tree_node";
      case ColumnFamilyId::TREE_VALUE:
        return "tree_value";
      case ColumnFamilyId::FLAT_KV:
        return "flat_kv";
      case ColumnFamilyId::TREE_PAGE:
        return "tree_page";
    }
    return std::nullopt;
  }

  std::expected<std::unique_ptr<RocksDb>, StorageError> open_db(
      const std::filesystem::path &path) {
    rocksdb::DB *db{};
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    // options.statistics = rocksdb::CreateDBStatistics();
    // options.stats_dump_period_sec = 1;
    // options.persist_stats_to_disk = true;

    std::vector<rocksdb::ColumnFamilyHandle *> handles;
    std::vector<rocksdb::ColumnFamilyDescriptor> desc;
    [[maybe_unused]] int family_idx = 0;
    for (auto family : column_families()) {
      // proper order is important because ColumnFamilyId serves as an index in
      // a vector of column family handles
      QTILS_ASSERT_EQ(family_idx++, static_cast<int>(family));
      desc.emplace_back(std::string{to_string_nocheck(family)},
          rocksdb::ColumnFamilyOptions{});
    }

    QTILS_UNWRAP_void(wrap_status(
        rocksdb::DB::Open(options, path.c_str(), desc, &handles, &db)));

    family_idx = 0;
    for ([[maybe_unused]] auto &handle : handles) {
      [[maybe_unused]] auto name = to_string(static_cast<ColumnFamilyId>(family_idx++));
      QTILS_ASSERT(name.has_value());
      QTILS_ASSERT_EQ(handle->GetName(), *name);
    }
    return std::unique_ptr<RocksDb>{new RocksDb{db, std::move(handles)}};
  }

  class RocksDbBatch final : public RocksDb::Batch,
                             public RocksDbColumnFamily::Batch {
   public:
    RocksDbBatch(
        std::shared_ptr<RocksDb> db, rocksdb::ColumnFamilyHandle *default_cf)
        : db{db}, default_cf{default_cf} {
      QTILS_ASSERT(db != nullptr);
      QTILS_ASSERT(default_cf != nullptr);
    }

    virtual ~RocksDbBatch() = default;

    std::expected<void, StorageError> write(
        qtils::ByteSpan key, qtils::ByteSpan value) override {
      return wrap_status(batch.Put(default_cf,
          qtils::to_string_view(key),
          qtils::to_string_view(value)));
    }

    std::expected<void, StorageError> remove(qtils::ByteSpan key) override {
      return wrap_status(batch.Delete(default_cf, qtils::to_string_view(key)));
    }

    std::expected<void, StorageError> write(ColumnFamilyId cf,
        qtils::ByteSpan key,
        qtils::ByteSpan value) override {
      return wrap_status(batch.Put(db->handles[static_cast<size_t>(cf)],
          qtils::to_string_view(key),
          qtils::to_string_view(value)));
    }

    std::expected<void, StorageError> remove(
        ColumnFamilyId cf, qtils::ByteSpan key) override {
      return wrap_status(batch.Delete(
          db->handles[static_cast<size_t>(cf)], qtils::to_string_view(key)));
    }

    std::shared_ptr<RocksDb> db;
    rocksdb::ColumnFamilyHandle *default_cf;
    rocksdb::WriteBatch batch;
  };

  RocksDb::RocksDb(
      rocksdb::DB *db, std::vector<rocksdb::ColumnFamilyHandle *> &&handles)
      : db{db}, handles{std::move(handles)} {
    QTILS_ASSERT(db != nullptr);
  }

  RocksDb::~RocksDb() {
    for (auto &handle : handles) {
      auto status = db->DestroyColumnFamilyHandle(handle);
      QTILS_ASSERT(status.ok());
    }
    auto status = db->Close();
    QTILS_ASSERT(status.ok());
    delete db;
  }

  std::shared_ptr<KeyValueStorage> RocksDb::get_column_family(
      ColumnFamilyId family) {
    return std::make_shared<RocksDbColumnFamily>(shared_from_this(), family);
  }

  std::unique_ptr<RocksDb::Batch> RocksDb::start_batch() {
    return std::make_unique<RocksDbBatch>(shared_from_this(),
        handles.at(std::to_underlying(ColumnFamilyId::DEFAULT)));
  }

  std::expected<void, StorageError> RocksDb::write_batch(
      std::unique_ptr<Batch> batch) {
    auto rocks_batch = dynamic_cast<RocksDbBatch *>(batch.get());
    QTILS_ASSERT(rocks_batch != nullptr);

    return wrap_status(db->Write(rocksdb::WriteOptions{}, &rocks_batch->batch));
  }

  RocksDbColumnFamily::RocksDbColumnFamily(
      std::shared_ptr<RocksDb> db, ColumnFamilyId family)
      : db{db}, handle{db->handles.at(std::to_underlying(family))} {}

  std::expected<void, StorageError> RocksDbColumnFamily::write(
      qtils::ByteSpan key, qtils::ByteSpan value) {
    rocksdb::WriteBatch updates;
    QTILS_UNWRAP_void(wrap_status(updates.Put(handle,
        rocksdb::Slice{qtils::to_string_view(key)},
        rocksdb::Slice{qtils::to_string_view(value)})));
    QTILS_UNWRAP_void(
        wrap_status(db->db->Write(rocksdb::WriteOptions{}, &updates)));
    return {};
  }

  std::expected<std::optional<qtils::Bytes>, StorageError>
  RocksDbColumnFamily::read(qtils::ByteSpan key) const {
    std::string value;
    auto status = db->db->Get(rocksdb::ReadOptions{},
        handle,
        rocksdb::Slice{qtils::to_string_view(key)},
        &value);
    if (status.IsNotFound()) {
      return std::nullopt;
    }
    QTILS_UNWRAP_void(wrap_status(status));
    return ByteVector{value.begin(), value.end()};
  }

  std::expected<std::optional<size_t>, StorageError>
  RocksDbColumnFamily::read_to(
      qtils::ByteSpan key, qtils::ByteSpanMut value) const {
    std::string res;
    auto status = db->db->Get(rocksdb::ReadOptions{},
        handle,
        rocksdb::Slice{qtils::to_string_view(key)},
        &res);
    if (status.IsNotFound()) {
      return std::nullopt;
    }
    QTILS_UNWRAP_void(wrap_status(status));
    std::copy_n(res.begin(), std::min(res.size(), value.size()), value.begin());
    return res.size();
  }

  std::expected<void, StorageError> RocksDbColumnFamily::remove(
      qtils::ByteSpan key) const {
    QTILS_UNWRAP_void(wrap_status(db->db->Delete(
        rocksdb::WriteOptions{}, handle, qtils::to_string_view(key))));
    return {};
  }

  std::unique_ptr<RocksDbColumnFamily::Batch>
  RocksDbColumnFamily::start_batch() {
    return std::make_unique<RocksDbBatch>(db, handle);
  }

  std::expected<void, StorageError> RocksDbColumnFamily::write_batch(
      std::unique_ptr<Batch> batch) {
    auto rocks_batch = dynamic_cast<RocksDbBatch *>(batch.get());
    QTILS_ASSERT(rocks_batch != nullptr);

    return wrap_status(
        db->db->Write(rocksdb::WriteOptions{}, &rocks_batch->batch));
  }

}  // namespace morum