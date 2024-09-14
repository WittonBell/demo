#include "net/net.h"
#include "pb/cpp/login.pb.h"
#include "pb/cpp/msgid.pb.h"
#include <boost/asio.hpp>
#include <format>
#include <iostream>
#include <stdio.h>
#include <string>

void login(int fd, netmsg::pbReqLogin &login) {
  std::cout << std::format("Socket:{2} User:{0} psw:{1}", login.username(), login.psw(), fd) << std::endl;
}

int main(int argc, char **argv) {
  RegNetMsg(netmsg::ReqLogin, login);
  initNet(6666);
  printf("test\n");
}
