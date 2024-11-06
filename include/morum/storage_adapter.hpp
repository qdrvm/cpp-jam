#pragma once

#include <expected>
#include <filesystem>
#include <generator>
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
  class StorageAdapter {
   public:
    virtual ~StorageAdapter() = default;

    virtual std::expected<void, StorageError> write(
        qtils::ByteSpan key, qtils::ByteSpan value) = 0;

    virtual std::expected<std::optional<ByteVector>, StorageError> read(
        qtils::ByteSpan key) const = 0;

    virtual std::expected<std::optional<size_t>, StorageError> read_to(
        qtils::ByteSpan key, qtils::ByteSpanMut value) const = 0;

    virtual std::expected<void, StorageError> remove(qtils::ByteSpan key) const = 0;

    /**
     * Batches are supposed to provide a way to write several entries atomically.
     */
    class Batch {
     public:
      virtual ~Batch() = default;

      virtual std::expected<void, StorageError> write(
          qtils::ByteSpan key, qtils::ByteSpan value) = 0;
      virtual std::expected<void, StorageError> remove(qtils::ByteSpan key) = 0;
    };

    virtual std::unique_ptr<Batch> start_batch() = 0;
    virtual std::expected<void, StorageError> write_batch(
        std::unique_ptr<Batch> batch) = 0;

   private:
  };

  enum class ColumnFamily {
    DEFAULT,
    TREE_NODE,
    TREE_VALUE,
    FLAT_KV,
    TREE_PAGE,
  };

  std::string_view to_string(ColumnFamily family);

  inline std::generator<ColumnFamily> column_families() {
    co_yield ColumnFamily::DEFAULT;
    co_yield ColumnFamily::TREE_NODE;
    co_yield ColumnFamily::TREE_VALUE;
    co_yield ColumnFamily::FLAT_KV;
    co_yield ColumnFamily::TREE_PAGE;
  }

  struct RocksDbAdapter {
    ~RocksDbAdapter();
    rocksdb::DB *db;
    std::vector<rocksdb::ColumnFamilyHandle *> handles;
  };

  std::expected<std::unique_ptr<RocksDbAdapter>, StorageError> open_db(
      const std::filesystem::path &path);

  class RocksDbFamilyAdapter : public StorageAdapter {
   public:
    RocksDbFamilyAdapter(const RocksDbAdapter &adapter, ColumnFamily family);

    virtual std::expected<void, StorageError> write(
        qtils::ByteSpan key, qtils::ByteSpan value) override;

    virtual std::expected<std::optional<ByteVector>, StorageError> read(
        qtils::ByteSpan key) const override;

    virtual std::expected<std::optional<size_t>, StorageError> read_to(
        qtils::ByteSpan key, qtils::ByteSpanMut value) const override;

    virtual std::expected<void, StorageError> remove(
        qtils::ByteSpan key) const override;

    virtual std::unique_ptr<Batch> start_batch() override;

    virtual std::expected<void, StorageError> write_batch(
        std::unique_ptr<Batch> batch) override;

   private:
    rocksdb::DB *db;
    rocksdb::ColumnFamilyHandle *handle;
  };

}  // namespace morum