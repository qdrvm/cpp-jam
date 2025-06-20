#pragma once

#include <expected>
#include <filesystem>
#include <memory>

#include <morum/common.hpp>

namespace rocksdb {
  class DB;
  class ColumnFamilyHandle;
}  // namespace rocksdb

namespace morum {

  /**
   * An interface class for various database back-ends.
   */
  class KeyValueStorage {
   public:
    virtual ~KeyValueStorage() = default;

    virtual std::expected<void, StorageError> write(
        qtils::ByteView key, qtils::ByteView value) = 0;

    virtual std::expected<std::optional<qtils::ByteVec>, StorageError> read(
        qtils::ByteView key) const = 0;

    virtual std::expected<std::optional<size_t>, StorageError> read_to(
        qtils::ByteView key, qtils::ByteSpanMut value) const = 0;

    virtual std::expected<void, StorageError> remove(
        qtils::ByteView key) const = 0;

    /**
     * Batches are supposed to provide a way to write several entries
     * atomically.
     */
    class Batch {
     public:
      virtual ~Batch() = default;

      virtual std::expected<void, StorageError> write(
          qtils::ByteView key, qtils::ByteView value) = 0;
      virtual std::expected<void, StorageError> remove(qtils::ByteView key) = 0;
    };

    virtual std::unique_ptr<Batch> start_batch() = 0;
    virtual std::expected<void, StorageError> write_batch(
        std::unique_ptr<Batch> batch) = 0;
  };

  template <std::regular ColumnFamilyId>
  class ColumnFamilyStorage {
   public:
    virtual ~ColumnFamilyStorage() = default;

    virtual std::shared_ptr<KeyValueStorage> get_column_family(
        ColumnFamilyId) = 0;

    /**
     * Batches are supposed to provide a way to write several entries
     * atomically.
     */
    class Batch {
     public:
      virtual ~Batch() = default;

      virtual std::expected<void, StorageError> write(
          ColumnFamilyId cf, qtils::ByteView key, qtils::ByteView value) = 0;

      virtual std::expected<void, StorageError> remove(
          ColumnFamilyId cf, qtils::ByteView key) = 0;
    };

    virtual std::unique_ptr<Batch> start_batch() = 0;
    virtual std::expected<void, StorageError> write_batch(
        std::unique_ptr<Batch> batch) = 0;
  };

  enum class ColumnFamilyId {
    DEFAULT,
    TREE_NODE,
    TREE_VALUE,
    FLAT_KV,
    TREE_PAGE,
  };

  std::string_view to_string_nocheck(ColumnFamilyId family);
  std::optional<std::string_view> to_string(ColumnFamilyId family);

  inline std::array<ColumnFamilyId, 5> column_families() {
    return {
        ColumnFamilyId::DEFAULT,
        ColumnFamilyId::TREE_NODE,
        ColumnFamilyId::TREE_VALUE,
        ColumnFamilyId::FLAT_KV,
        ColumnFamilyId::TREE_PAGE,
    };
  }

  class RocksDb final : public ColumnFamilyStorage<ColumnFamilyId>,
                        public std::enable_shared_from_this<RocksDb> {
   public:
    using Batch = ColumnFamilyStorage<ColumnFamilyId>::Batch;

    ~RocksDb() override;

    virtual std::shared_ptr<KeyValueStorage> get_column_family(
        ColumnFamilyId) override;

    virtual std::unique_ptr<Batch> start_batch() override;
    virtual std::expected<void, StorageError> write_batch(
        std::unique_ptr<Batch> batch) override;

   private:
    friend std::expected<std::unique_ptr<RocksDb>, StorageError> open_db(
        const std::filesystem::path &path);
    friend class RocksDbColumnFamily;
    friend class RocksDbBatch;

    RocksDb(
        rocksdb::DB *db, std::vector<rocksdb::ColumnFamilyHandle *> &&handles);
    rocksdb::DB *db;
    std::vector<rocksdb::ColumnFamilyHandle *> handles;
  };

  std::expected<std::unique_ptr<RocksDb>, StorageError> open_db(
      const std::filesystem::path &path);

  class RocksDbColumnFamily final : public KeyValueStorage {
   public:
    RocksDbColumnFamily(std::shared_ptr<RocksDb> db, ColumnFamilyId family);

    virtual std::expected<void, StorageError> write(
        qtils::ByteView key, qtils::ByteView value) override;

    virtual std::expected<std::optional<qtils::ByteVec>, StorageError> read(
        qtils::ByteView key) const override;

    virtual std::expected<std::optional<size_t>, StorageError> read_to(
        qtils::ByteView key, qtils::ByteSpanMut value) const override;

    virtual std::expected<void, StorageError> remove(
        qtils::ByteView key) const override;

    virtual std::unique_ptr<Batch> start_batch() override;

    virtual std::expected<void, StorageError> write_batch(
        std::unique_ptr<Batch> batch) override;

   private:
    std::shared_ptr<RocksDb> db;
    rocksdb::ColumnFamilyHandle *handle;
  };

}  // namespace morum