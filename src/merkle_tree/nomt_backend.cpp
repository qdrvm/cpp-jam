#include <morum/nomt_backend.hpp>

namespace morum {

  qtils::OptionalRef<RawNode> Page::get_node(qtils::BitSpan<> path) {
    QTILS_ASSERT_GREATER(path.size_bits(), 0);
    QTILS_ASSERT(path.size_bits() <= PAGE_LEVELS);
    RawNode *current = &nodes[path[0]];
    size_t offset{path[0]};
    for (size_t level = 1; level < path.size_bits(); ++level) {
      auto bit = path[level];
      if (current->is_leaf() || !current->branch.has_child(bit)) {
        return std::nullopt;
      }
      offset = offset | (bit << (level));
      size_t level_offset = (1 << (level + 1)) - 2;
      current = &nodes[level_offset + offset];
    }
    return *current;
  }

  // ignores branch nodes and just return where the corresponding node is
  // supposed to be placed
  RawNode &Page::get_node_unchecked(qtils::BitSpan<> path) {
    QTILS_ASSERT_GREATER(path.size_bits(), 0);
    QTILS_ASSERT(path.size_bits() <= PAGE_LEVELS);
    size_t offset = path.get_as_byte(0, path.size_bits());
    size_t level_offset = (1 << path.size_bits()) - 2;
    return nodes[level_offset + offset];
  }

  std::expected<std::optional<RawNode>, StorageError>
  NearlyOptimalNodeStorage::load_root_node() const {
    RawNode root{};
    QTILS_UNWRAP(auto exists,
        storage_->read_to({},
            qtils::ByteSpanMut{
                reinterpret_cast<uint8_t *>(&root), sizeof(RawNode)}));
    if (exists) {
      return root;
    }
    return std::nullopt;
  }

  std::expected<std::optional<Page>, StorageError>
  NearlyOptimalNodeStorage::load_terminal_page(qtils::BitSpan<> path) const {
    std::optional<Page> previous_page;
    for (size_t depth = 0; depth <= path.size_bits() / PAGE_LEVELS; depth++) {
      QTILS_UNWRAP(auto opt_page,
          load_page_direct(path.subspan(0, depth * PAGE_LEVELS)));
      if (!opt_page) {
        if (previous_page) {
          MORUM_TRACE(
              "load terminal page, at path {}, found at depth {} pages/{} "
              "bit",
              path,
              depth,
              depth * PAGE_LEVELS);
        } else {
          MORUM_TRACE("load terminal page, at path {}, not found",
              path,
              depth,
              depth * PAGE_LEVELS);
        }

        return previous_page;
      }
      auto &page = *opt_page;
      previous_page = page;
      auto node = page.get_node(path.subspan(depth * PAGE_LEVELS, PAGE_LEVELS));
      if (!node || node->is_leaf()) {
        MORUM_TRACE(
            "load terminal page, at path {}, found at depth {} pages/{} bit",
            path,
            depth,
            depth * PAGE_LEVELS);

        return page;
      }
    }
    QTILS_ASSERT(
        !"unreachable: all paths must end with a leaf node or a null child of a branch node");
    std::unreachable();
  }

  std::expected<std::optional<Page>, StorageError>
  NearlyOptimalNodeStorage::load_page_direct(qtils::BitSpan<> path) const {
    if (path.size_bits() == 0) {
      return std::nullopt;
    }
    std::expected<std::optional<Page>, StorageError> page{};

    auto key = get_page_key(path);
    QTILS_UNWRAP(auto res,
        storage_->read_to(key.span(),
            qtils::ByteSpanMut{
                reinterpret_cast<uint8_t *>(&**page), sizeof(Page)}));
    if (!res) {
      MORUM_TRACE("load page directly, at path {}, key {}, none found",
          path,
          key.span());
      return std::nullopt;
    }
    QTILS_ASSERT(res.value() == sizeof(Page));
    MORUM_TRACE("load page directly, at path {}, key {}, found successfully",
        path,
        key.span());
    return page;
  }

  std::expected<void, StorageError> NearlyOptimalNodeStorage::WriteBatch::set(
      qtils::BitSpan<> path, const RawNode &node) {
    if (path.size_bits() == 0) {
      root_node = node;
      return {};
    }
    qtils::OptionalRef<Page> page{};
    Page page_data{};
    auto [key, _] = get_page_key(path);
    if (auto it = page_cache.find(key); it != page_cache.end()) {
      page = it->second;
    } else {
      QTILS_UNWRAP(auto page_opt, page_storage->load_page_direct(path));
      if (page_opt) {
        page_data = *page_opt;
      }
      page = page_data;
      page_cache.emplace(key, page_data);
    }
    auto offset = PAGE_LEVELS * ((path.size_bits() - 1) / PAGE_LEVELS);
    page->get_node_unchecked(path.subspan(offset, path.size_bits() - offset)) =
        node;
    return {};
  }

  std::expected<std::optional<RawNode>, StorageError>
  NearlyOptimalNodeStorage::WriteBatch::load(
      [[maybe_unused]] const Hash32 &hash, qtils::BitSpan<> path) {
    if (path.size_bits() == 0) {
      QTILS_UNWRAP(auto root_opt, page_storage->load_root_node());
      if (!root_opt) {
        MORUM_TRACE("load root node, not found");
        return std::nullopt;
      }
      MORUM_TRACE("load root node, success");
      return *root_opt;
    }
    QTILS_UNWRAP(auto page_opt, page_storage->load_terminal_page(path));
    if (page_opt) {
      auto raw_node = page_opt->get_node(path);
      if (!raw_node) {
        MORUM_TRACE("load node hash {} path {}, not found in page", hash, path);
        return std::nullopt;
      }
      MORUM_TRACE("load node hash {} path {}, success", hash, path);
      return *raw_node;
    }
    MORUM_TRACE("load node hash {} path {}, page not found", hash, path);
    return std::nullopt;
  }

  std::expected<void, StorageError> NearlyOptimalNodeStorage::submit_batch(
      std::unique_ptr<WriteBatch> batch,
      ColumnFamilyStorage<ColumnFamilyId>::Batch &db_batch) {
    QTILS_ASSERT(batch != nullptr);
    if (batch->root_node) {
      if (batch->root_node->is_branch()) {
        QTILS_UNWRAP_void(db_batch.write(ColumnFamilyId::TREE_PAGE,
            qtils::ByteSpan{},
            serialize_branch(batch->root_node->branch.left,
                batch->root_node->branch.right)));
      } else {
        QTILS_UNWRAP_void(db_batch.write(ColumnFamilyId::TREE_PAGE,
            qtils::ByteSpan{},
            serialize_leaf(batch->root_node->leaf.get_key(),
                batch->root_node->leaf.hash_or_value())));
      }
    }

    for (auto &[key, page] : batch->page_cache) {
      QTILS_UNWRAP_void(db_batch.write(ColumnFamilyId::TREE_PAGE,
          qtils::ByteSpan{key.data(), key[0]},
          page.as_bytes()));
    }
    return {};
  }

  qtils::FixedByteVector<sizeof(Hash32) + 1>
  NearlyOptimalNodeStorage::get_page_key(qtils::BitSpan<> path) {
    path = path.subspan(0, (path.size_bits() / PAGE_LEVELS) * PAGE_LEVELS);
    qtils::FixedByteVector<sizeof(Hash32) + 1> key_storage{};
    key_storage.data[0] = path.size_bits();
    std::copy(path.begin(), path.end(), key_storage.data.begin() + 1);
    key_storage.size = path.size_bits() / 8 + 1;
    return key_storage;
  }

  std::expected<std::optional<std::unique_ptr<MerkleTree>>, StorageError>
  NomtDb::load_tree(const Hash32 &root) const {
    auto storage = std::make_unique<FlatPagedNodeStorage>();
    class NomtNodeLoader final : public NodeLoader {
     public:
      NomtNodeLoader(const NomtDb &db, FlatPagedNodeStorage &storage)
          : db{db}, storage{storage} {}

      virtual std::expected<std::optional<TreeNode>, StorageError> load(
          qtils::BitSpan<> path, const Hash32 &hash_copy) const override {
        Hash32 hash = hash_copy;
        hash[0] &= 0xFE;
        if (path.size_bits() == 0) {
          QTILS_UNWRAP(auto root_opt, db.page_storage_->load_root_node());
          if (!root_opt) {
            MORUM_TRACE("load root node, not found");
            return std::nullopt;
          }
          MORUM_TRACE("load root node, success");
          db.metrics_.nodes_loaded++;

          return raw_node_to_node(*root_opt);
        }
        QTILS_UNWRAP(auto page_opt, db.page_storage_->load_terminal_page(path));
        if (page_opt) {
          auto raw_node = page_opt->get_node(path);
          if (!raw_node) {
            MORUM_TRACE(
                "load node hash {} path {}, not found in page", hash, path);
            return std::nullopt;
          }
          MORUM_TRACE("load node hash {} path {}, success", hash, path);
          db.metrics_.nodes_loaded++;
          return raw_node_to_node(*raw_node);
        }
        MORUM_TRACE("load node hash {} path {}, page not found", hash, path);
        return std::nullopt;
      }

      const NomtDb &db;
      FlatPagedNodeStorage &storage;
    };
    auto loader = std::make_shared<NomtNodeLoader>(*this, *storage);
    QTILS_UNWRAP(auto root_node_opt, loader->load(qtils::BitSpan<>{}, root));
    if (!root_node_opt) {
      return std::nullopt;
    }
    return std::make_unique<MerkleTree>(
        std::move(storage), std::make_shared<NoopNodeLoader>());
  }

  std::unique_ptr<MerkleTree> NomtDb::empty_tree() const {
    auto storage = std::make_unique<FlatPagedNodeStorage>();

    return std::make_unique<MerkleTree>(
        std::move(storage), std::make_shared<NoopNodeLoader>());
  }

  std::expected<Hash32, StorageError> NomtDb::get_root_and_store(
      const MerkleTree &tree) {
    auto page_batch = page_storage_->start_writing();
    auto total_batch = storage_->start_batch();

    auto hash = tree.calculate_hash([&](const TreeNode &n,
                                        qtils::ByteSpan,
                                        qtils::ByteSpan hash,
                                        qtils::BitSpan<> path) {
      morum::Hash32 hash_copy;
      std::ranges::copy(hash, hash_copy.begin());
      hash_copy[0] &= 0xFE;
      RawNode raw_node{};
      if (n.is_leaf()) {
        raw_node.leaf = n.as_leaf();
      } else {
        raw_node.branch.left = n.as_branch().get_left_hash_raw();
        raw_node.branch.right = n.as_branch().get_right_hash_raw();
        QTILS_ASSERT(raw_node.branch.left != Branch::NoHash);
        QTILS_ASSERT(raw_node.branch.right != Branch::NoHash);
      }
      MORUM_TRACE("store node hash {} path {}", hash, path);
      [[maybe_unused]] auto res = page_batch->set(path, raw_node);
      QTILS_ASSERT(res);

      if (n.is_leaf()) {
        auto h_or_v = n.as_leaf().hash_or_value();
        if (auto *hash = std::get_if<morum::HashRef>(&h_or_v); hash) {
          if (auto value_opt = tree.get_cached_value(hash->get()); value_opt) {
            total_batch
                ->write(
                    ColumnFamilyId::TREE_VALUE, hash->get(), value_opt.value())
                .value();
          }
        }
      }
    });
    hash[0] &= 0xFE;

    auto page_batch_start = Clock::now();
    [[maybe_unused]] auto res =
        page_storage_->submit_batch(std::move(page_batch), *total_batch);
    QTILS_ASSERT_HAS_VALUE(res);
    metrics_.page_batch_write_duration += Clock::now() - page_batch_start;

    auto value_batch_start = Clock::now();
    [[maybe_unused]] auto res2 = storage_->write_batch(std::move(total_batch));
    QTILS_ASSERT_HAS_VALUE(res2);
    metrics_.value_batch_write_duration += Clock::now() - value_batch_start;

    return hash;
  }
}  // namespace morum