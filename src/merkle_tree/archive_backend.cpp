#include <morum/archive_backend.hpp>

namespace morum {

  std::expected<std::optional<TreeNode>, StorageError> ArchiveNodeLoader::load(
      qtils::BitSpan<>, const Hash32 &hash) const {
    Hash32 hash_copy = hash;
    hash_copy[0] &= 0xFE;
    ByteArray<sizeof(Leaf)>
        node_data;  // in-memory branches contain non-serializable metadata,
                    // so Leaf's size is used
    QTILS_UNWRAP(auto res, node_storage->read_to(hash_copy, node_data));
    if (!res) {
      return std::nullopt;
    }
    QTILS_ASSERT_EQ(*res, sizeof(Leaf));
    return deserialize_node(node_data);
  }

  ArchiveTrieDb::ArchiveTrieDb(
        std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage)
        : storage_{storage},
          node_storage_{storage_->get_column_family(ColumnFamilyId::TREE_NODE)},
          value_storage_{storage_->get_column_family(ColumnFamilyId::TREE_VALUE)} {
      QTILS_ASSERT(storage_ != nullptr);
      QTILS_ASSERT(node_storage_ != nullptr);
      QTILS_ASSERT(value_storage_ != nullptr);
    }

    std::expected<std::optional<std::unique_ptr<MerkleTree>>, StorageError>
    ArchiveTrieDb::load_tree(const Hash32 &root_hash) const {
      auto root_bytes_res = node_storage_->read(root_hash);
      QTILS_ASSERT_HAS_VALUE(root_bytes_res);
      QTILS_ASSERT(root_bytes_res->has_value());
      auto root_bytes = **root_bytes_res;
      auto root = morum::deserialize_node(root_bytes);
      return std::make_unique<MerkleTree>(root,
          std::make_unique<FlatPagedNodeStorage>(),
          std::make_shared<ArchiveNodeLoader>(node_storage_));
    }

    std::unique_ptr<MerkleTree> ArchiveTrieDb::empty_tree() const {
      return std::make_unique<MerkleTree>(
          std::make_unique<FlatPagedNodeStorage>(),
          std::make_shared<NoopNodeLoader>());
    }

    std::expected<Hash32, StorageError> ArchiveTrieDb::get_root_and_store(
        const MerkleTree &tree) {
      auto batch = storage_->start_batch();

      auto hash = tree.calculate_hash([&](const morum::TreeNode &n,
                                          qtils::ByteSpan serialized,
                                          qtils::ByteSpan hash,
                                          qtils::BitSpan<>) {
        morum::Hash32 hash_copy;
        std::ranges::copy(hash, hash_copy.begin());
        hash_copy[0] &= 0xFE;
        [[maybe_unused]] auto res =
            batch->write(ColumnFamilyId::TREE_NODE, hash_copy, serialized);
        QTILS_ASSERT(res);
        if (n.is_leaf()) {
          // cached_nodes_.emplace(hash_copy, n);
        } else {
          auto &branch = n.as_branch();
          // original node may contain child node ids which are not persistent
          morum::Branch b{branch.get_left_hash(), branch.get_right_hash()};
        }
        if (n.is_leaf()) {
          auto h_or_v = n.as_leaf().hash_or_value();
          if (auto *hash = std::get_if<morum::HashRef>(&h_or_v); hash) {
            if (auto value_opt = tree.get_cached_value(hash->get());
                value_opt) {
              batch
                  ->write(ColumnFamilyId::TREE_VALUE,
                      hash->get(),
                      value_opt.value())
                  .value();
            }
          }
        }
      });
      hash[0] &= 0xFE;

      [[maybe_unused]] auto res = storage_->write_batch(std::move(batch));
      QTILS_ASSERT_HAS_VALUE(res);

      return hash;
    }
}  // namespace morum