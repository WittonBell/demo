#include "net.h"
#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::redirect_error;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;

struct MsgHandler {
  std::shared_ptr<google::protobuf::Message> msg;
  void (*handler)(int fd, google::protobuf::Message &msg);
};
static std::map<uint16_t, MsgHandler> mapMsgHandler;

class session : public std::enable_shared_from_this<session> {
public:
  session(tcp::socket &&socket)
      : m_socket(std::move(socket)), m_timer(m_socket.get_executor()) {
    m_timer.expires_at(std::chrono::steady_clock::time_point::max());
  }

  void start() {
    co_spawn(
        m_socket.get_executor(),
        [self = shared_from_this()] { return self->reader(); }, detached);
    co_spawn(
        m_socket.get_executor(),
        [self = shared_from_this()] { return self->writer(); }, detached);
  }

private:
  awaitable<void> reader() {
    try {
      uint16_t len = 0;
      co_await boost::asio::async_read(
          m_socket, boost::asio::buffer(&len, sizeof(len)), use_awaitable);
      char data[65536];
      co_await boost::asio::async_read(m_socket, boost::asio::buffer(data, len),
                                       use_awaitable);
      uint16_t cmdId = *(uint16_t *)data;
      std::cout << std::format("Recv Len:{0} CmdID:{1}", len, cmdId)
                << std::endl;
      auto iter = mapMsgHandler.find(cmdId);
      if (iter != mapMsgHandler.end()) {
        auto msg = iter->second.msg;
        msg->ParseFromArray(&data[sizeof(uint16_t)], len - sizeof(uint16_t));
        SOCKET fd = m_socket.native_handle();
        iter->second.handler(fd, *msg);
      } else {
        std::cout << std::format("unimplement msg handler, msgId:{0}", cmdId)
                  << std::endl;
      }
    } catch (std::exception &) {
      stop();
    }
  }

  awaitable<void> writer() {
    try {
      while (m_socket.is_open()) {
        if (m_write_msgs.empty()) {
          boost::system::error_code ec;
          co_await m_timer.async_wait(redirect_error(use_awaitable, ec));
        } else {
          co_await boost::asio::async_write(
              m_socket, boost::asio::buffer(m_write_msgs.front()),
              use_awaitable);
          m_write_msgs.pop_front();
        }
      }
    } catch (std::exception &) {
      stop();
    }
  }

  void stop() {
    m_socket.close();
    m_timer.cancel();
  }

private:
  tcp::socket m_socket;
  boost::asio::steady_timer m_timer;
  std::deque<std::string> m_write_msgs;
};

awaitable<void> listener(tcp::acceptor acceptor) {
  for (;;) {
    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
    std::make_shared<session>(std::move(socket))->start();
  }
}

bool initNet(unsigned short port) {
  try {
    auto concurrency_hint = std::thread::hardware_concurrency();
    boost::asio::io_context io_context(concurrency_hint);
    co_spawn(io_context, listener(tcp::acceptor(io_context, {tcp::v4(), port})),
             detached);
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });
    io_context.run();
  } catch (std::exception &e) {
    return false;
  }
  return true;
}

void regNetMsg(uint16_t cmdId, std::shared_ptr<google::protobuf::Message> &&msg,
               void (*handler)(int fd, google::protobuf::Message &)) {
  mapMsgHandler[cmdId] = MsgHandler(msg, handler);
}