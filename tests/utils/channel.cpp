#include "utils/channel.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <thread>

using namespace std::chrono_literals;

TEST(ChannelTest, SendAndReceiveValue) {
  auto [recv, send] = Channel<int>::create_channel<int>();

  send.set(42);
  auto value = recv.wait();

  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value.value(), 42);
}

TEST(ChannelTest, SendLValue) {
  auto [recv, send] = Channel<int>::create_channel<int>();

  int x = 123;
  send.set(x);
  auto value = recv.wait();

  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value.value(), 123);
}

TEST(ChannelTest, SenderDestructionNotifiesReceiver) {
  std::optional<Channel<int>::Receiver> recv;
  std::optional<Channel<int>::Sender> send;

  std::tie(recv, send) = Channel<int>::create_channel<int>();

  std::optional<int> result;

  std::thread t([&]() { result = recv->wait(); });

  std::this_thread::sleep_for(50ms);
  send.reset();

  t.join();

  EXPECT_FALSE(result.has_value());
}

TEST(ChannelTest, MultipleSendKeepsOneValue) {
  auto [recv, send] = Channel<int>::create_channel<int>();

  send.set(1);
  send.set(2);

  auto value = recv.wait();
  ASSERT_TRUE(value.has_value());
  EXPECT_TRUE(value.value() == 1 || value.value() == 2);
}

TEST(ChannelTest, ReceiverDestructionUnregistersSender) {
  std::optional<Channel<int>::Receiver> recv;
  std::optional<Channel<int>::Sender> send;

  std::tie(recv, send) = Channel<int>::create_channel<int>();

  recv.reset();

  EXPECT_NO_THROW(send->set(999));
}
