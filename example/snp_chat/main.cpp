#include <TODO_qtils/asio_buffer.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/outcome/try.hpp>
#include <fmt/format.h>
#include <qtils/append.hpp>
#include <qtils/unhex.hpp>

#include "coro/spawn.hpp"
#include "snp/connections/address.hpp"
#include "snp/connections/connection.hpp"
#include "snp/connections/connections.hpp"
#include "snp/connections/controller.hpp"
#include "snp/connections/stream.hpp"

using jam::Coro;
using jam::coroHandler;
using jam::CoroHandler;
using jam::CoroOutcome;
using jam::coroSpawn;
using jam::GenesisHash;
using jam::IoContextPtr;
using jam::crypto::ed25519::KeyPair;
using jam::snp::Address;
using jam::snp::ConnectionInfo;
using jam::snp::Connections;
using jam::snp::ConnectionsConfig;
using jam::snp::ConnectionsController;
using jam::snp::Key;
using jam::snp::ProtocolId;
using jam::snp::StreamPtr;

inline auto operator""_ed25519(const char *c, size_t s) {
  auto seed = qtils::unhex<jam::crypto::ed25519::Seed>({c, s}).value();
  return jam::crypto::ed25519::from_seed(seed);
}

std::vector<KeyPair> keys{
    "f8dfdb0f1103d9fb2905204ac32529d5f148761c4321b2865b0a40e15be75f57"_ed25519,
    "96c891b8726cb18c781aefc082dbafcb827e16c8f18f22d461e83eabd618e780"_ed25519,
    "619d5e68139f714ee8e7892ce5afd8fbe7a4172a675fea5c5a06fb94fe3d797d"_ed25519,
    "8d0c5f498a763eaa8c04861cac06289784140b4bbfa814fef898f1f4095de4a3"_ed25519,
};
Address server_address{
    Address::kLocal,
    10000,
    jam::crypto::ed25519::get_public(keys[0]),
};
ProtocolId protocol_id = ProtocolId::make(0, true).value();

size_t indexOfKey(const Key &key) {
  auto it = std::ranges::find_if(keys, [&](const KeyPair &keypair) {
    return jam::crypto::ed25519::get_public(keypair) == key;
  });
  if (it == keys.end()) {
    throw std::logic_error{"TODO: example"};
  }
  return it - keys.begin();
}

struct ChatController : ConnectionsController {
  static constexpr size_t kMaxMsg = 8;

  struct Writer {
    StreamPtr stream;
    std::deque<qtils::Bytes> queue;
    bool writing = false;
  };
  using WriterPtr = std::shared_ptr<Writer>;

  std::map<size_t, WriterPtr> writers;

  static CoroOutcome<void> write(WriterPtr writer,
                                 size_t i_msg,
                                 const std::string msg) {
    qtils::Bytes buffer;
    buffer.emplace_back(i_msg);
    qtils::append(buffer, qtils::str2byte(msg));
    writer->queue.emplace_back(buffer);
    if (writer->writing) {
      co_return outcome::success();
    }
    writer->writing = true;
    while (not writer->queue.empty()) {
      auto buffer = writer->queue.front();
      writer->queue.pop_front();
      BOOST_OUTCOME_CO_TRY(
          co_await writer->stream->write(writer->stream, buffer));
    }
    writer->writing = false;
    co_return outcome::success();
  }

  void onOpen(Key key) override {
    fmt::println("#{} (connected)", indexOfKey(key));
  }

  void onClose(Key key) override {
    fmt::println("#{} (disconnected)", indexOfKey(key));
  }

  void print(size_t i_msg, std::string msg) {
    fmt::println("#{} > {}", i_msg, msg);
  }

  Coro<void> broadcast(std::optional<size_t> i_read,
                       size_t i_msg,
                       std::string msg) {
    for (auto &[i_write, writer] : writers) {
      if (i_write == i_read) {
        continue;
      }
      co_await coroSpawn([this, i_write, writer, i_msg, msg]() -> Coro<void> {
        if (not co_await write(writer, i_msg, msg)) {
          writers.erase(i_write);
        }
      });
    }
  }

  Coro<void> onRead(size_t i_read, size_t i_msg, std::string msg) {
    print(i_msg, msg);
    co_await broadcast(i_read, i_msg, msg);
  }

  CoroOutcome<void> add(ConnectionInfo info, StreamPtr stream) {
    auto i_read = indexOfKey(info.key);
    writers.emplace(i_read, std::make_shared<Writer>(Writer{stream}));
    qtils::Bytes buffer;
    while (true) {
      BOOST_OUTCOME_CO_TRY(auto read,
                           co_await stream->read(stream, buffer, 1 + kMaxMsg));
      if (not read) {
        break;
      }
      if (buffer.size() < 1) {
        break;
      }
      auto i_msg = buffer[0];
      co_await onRead(
          i_read, i_msg, std::string{qtils::byte2str(buffer).substr(1)});
    }
    co_await stream->readFin(stream);
    co_return outcome::success();
  }
};

struct Input {
  Input(IoContextPtr io_context_ptr) : fd_{*io_context_ptr, STDIN_FILENO} {}

  Coro<std::optional<std::string>> read() {
    auto [ec, n] = co_await boost::asio::async_read_until(
        fd_, buf_, "\n", boost::asio::as_tuple(boost::asio::use_awaitable));
    if (ec) {
      co_return std::nullopt;
    }
    auto s = qtils::byte2str(qtils::asioBuffer(buf_.data()));
    auto i = s.find("\n");
    if (i != s.npos) {
      s = s.substr(0, i);
    }
    auto r = std::string{s};
    buf_.consume(buf_.size());
    co_return r;
  }

  boost::asio::posix::stream_descriptor fd_;
  boost::asio::streambuf buf_;
};

CoroOutcome<void> co_main(IoContextPtr io_context_ptr, size_t arg_i) {
  fmt::println("#{} (self)", arg_i);

  std::optional<uint16_t> listen_port;
  GenesisHash genesis;
  ConnectionsConfig config{genesis, keys.at(arg_i)};
  auto is_server = arg_i == 0;
  if (is_server) {
    config.listen_port = server_address.port;
  }
  auto connections = std::make_shared<Connections>(io_context_ptr, config);
  auto chat = std::make_shared<ChatController>();
  BOOST_OUTCOME_CO_TRY(co_await connections->init(connections, chat));
  co_await coroSpawn([io_context_ptr, arg_i, chat]() -> Coro<void> {
    Input input{io_context_ptr};
    while (true) {
      auto msg = co_await input.read();
      if (not msg.has_value()) {
        break;
      }
      msg->resize(std::min(msg->size(), ChatController::kMaxMsg));
      if (msg->empty()) {
        continue;
      }
      co_await chat->broadcast(std::nullopt, arg_i, *msg);
    }
    io_context_ptr->stop();
  });
  if (not is_server) {
    BOOST_OUTCOME_CO_TRY(
        auto connection,
        co_await connections->connect(connections, server_address));
    BOOST_OUTCOME_CO_TRY(auto stream,
                         co_await connection->open(connection, protocol_id));
    std::ignore = co_await chat->add(connection->info(), stream);
    fmt::println("(disconnected)");
    io_context_ptr->stop();
  } else {
    co_await connections->serve(
        connections,
        protocol_id,
        [chat](ConnectionInfo info, StreamPtr stream) -> CoroOutcome<void> {
          co_return co_await chat->add(info, stream);
        });
    std::optional<CoroHandler<void>> work_guard;
    co_await coroHandler<void>([&](CoroHandler<void> &&handler) {
      work_guard.emplace(std::move(handler));
    });
  }
  co_return outcome::success();
}

int main(int argc, char **argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);

  size_t arg_i = 0;
  if (argc == 2) {
    arg_i = std::atoi(argv[1]);
  }

  auto io_context_ptr = std::make_shared<boost::asio::io_context>();
  coroSpawn(*io_context_ptr, [io_context_ptr, arg_i]() -> Coro<void> {
    (co_await co_main(io_context_ptr, arg_i)).value();
  });
  io_context_ptr->run();
}
