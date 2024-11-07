#include <ranges>
#include <span>
#include <cstddef>

#include <qtils/assert.hpp>
#include <qtils/bytes.hpp>
#include <qtils/optional_ref.hpp>

#include <morum/common.hpp>
#include <morum/nomt_backend.hpp>
#include <morum/tree_node.hpp>

void test_node_consistency(
    morum::Page &page, qtils::BitSpan<> path, morum::RawNode node) {
  for (auto [level, bit] : path.skip_first(1) | std::views::enumerate) {
    auto &node = page.get_node_unchecked(path.subspan(0, level + 1));
    node.branch =
        morum::RawBranch{bit ? morum::ZeroHash32 : morum::Hash32{0, 1},
            bit ? morum::Hash32{1} : morum::ZeroHash32};
  }
  auto &node_place = page.get_node_unchecked(path);
  node_place = node;
  [[maybe_unused]] auto placed_node = page.get_node(path);
  QTILS_ASSERT_HAS_VALUE(placed_node);
  QTILS_ASSERT((placed_node->is_branch() && node.is_branch())
      || (placed_node->is_leaf() && node.is_leaf()));
  QTILS_ASSERT_RANGE_EQ((morum::ByteSpan{reinterpret_cast<uint8_t *>(&node),
                            sizeof(morum::RawNode)}),
      (morum::ByteSpan{
          reinterpret_cast<uint8_t *>(&*placed_node), sizeof(morum::RawNode)}));
}

int main() {
  qtils::ByteArray<5> array{0x01, 0x23, 0x45, 0x67, 0x89};
  qtils::BitSpan<> span{array};
  QTILS_ASSERT_EQ(span.get_as_byte(0, 8), 0x01);
  QTILS_ASSERT_EQ(span.get_as_byte(8, 8), 0x23);
  QTILS_ASSERT_EQ(span.get_as_byte(32, 8), 0x89);
  QTILS_ASSERT_EQ(span.get_as_byte(0, 4), 0x1);
  QTILS_ASSERT_EQ(span.get_as_byte(4, 4), 0x0);
  QTILS_ASSERT_EQ(span.get_as_byte(4, 8), 0x30);
  QTILS_ASSERT_EQ(span.get_as_byte(12, 3), 0x2);

  qtils::ByteArray<32> path{0b0000'0000};
  morum::Page page{};
  auto node1 = page.get_node(qtils::BitSpan<>{path, 0, 1});
  QTILS_ASSERT_HAS_VALUE(node1);
  node1->branch = morum::RawBranch{
      morum::ZeroHash32, morum::blake2b_256(qtils::ByteArray<4>{0, 1, 2, 3})};
  auto &node2 = page.get_node_unchecked(qtils::BitSpan<>{path, 0, 1});
  node2.leaf =
      morum::Leaf{morum::Leaf::EmbeddedTag{}, qtils::ByteArray<31>{0xAB}, {}};
  [[maybe_unused]] auto node3 = page.get_node(qtils::BitSpan<>{path, 0, 1});
  QTILS_ASSERT_HAS_VALUE(node3);
  QTILS_ASSERT(node3->is_leaf());
  QTILS_ASSERT_EQ(node3->leaf.get_key()[0], 0xAB);

  for (size_t depth = 1; depth < 6; depth++) {
    morum::Page page{};
    morum::ByteArray<32> path{0b0000'0000};
    test_node_consistency(page,
        qtils::BitSpan<>{path, 0, depth},
        morum::RawNode{
            .leaf = morum::Leaf{
                morum::Leaf::EmbeddedTag{}, morum::ByteArray<31>{0xAB}, {}}});
  }
  for (size_t depth = 1; depth < 6; depth++) {
    morum::Page page{};
    qtils::ByteArray<32> path{0b1111'1111};
    test_node_consistency(page,
        qtils::BitSpan<>{path, 0, depth},
        morum::RawNode{
            .leaf = morum::Leaf{
                morum::Leaf::EmbeddedTag{}, morum::ByteArray<31>{0xAB}, {}}});
  }
  for (size_t depth = 1; depth < 6; depth++) {
    morum::Page page{};
    qtils::ByteArray<32> path{0b1010'1010};
    test_node_consistency(page,
        qtils::BitSpan<>{path, 0, depth},
        morum::RawNode{
            .leaf = morum::Leaf{
                morum::Leaf::EmbeddedTag{}, morum::ByteArray<31>{0xAB}, {}}});
  }
}
