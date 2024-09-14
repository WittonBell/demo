#pragma once
#include "google/protobuf/message.h"

bool initNet(unsigned short port);

template <typename T>
void RegNetMsg(uint16_t msgId, void (*handler)(int fd, T &msg)) {
  void regNetMsg(uint16_t msgId,
                 std::shared_ptr<google::protobuf::Message> && msg,
                 void (*handler)(int fd, google::protobuf::Message &msg));
  regNetMsg(msgId, std::make_shared<T>(),
            (void (*)(int fd, google::protobuf::Message &msg))handler);
}