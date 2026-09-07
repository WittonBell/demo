#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpc_c.h"

int rpc_call(const char* host, int port, buffer_t* buf, rpc_rsp_t* result) {
  static uint32_t callSN = 0;
  callSN++;
  // 创建socket并连接服务器
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;

  struct hostent* server = gethostbyname(host);
  if (!server) {
    close_sock(sock);
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
  addr.sin_port = htons(port);

  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close_sock(sock);
    return -1;
  }
  // 设置超时
  rpc_set_sock_timeout(sock);
  printf("client socket:%d\n", (int)sock);

  // 1. 发送magic
  uint32_t magic_num = rpc_get_magic_num();
  if (write_sock(sock, (const char*)&magic_num, sizeof(magic_num)) != sizeof(magic_num)) {
    buffer_free(buf);
    close_sock(sock);
    return -1;
  }
  // 2. 发送调用序号
  if (write_sock(sock, (const char*)&callSN, sizeof(callSN)) != sizeof(callSN)) {
    buffer_free(buf);
    close_sock(sock);
    return -1;
  }
  // 3.发送请求：先4字节长度（网络序），再数据
  if (write_sock(sock, (const char*)&buf->len, 4) != 4) {
    buffer_free(buf);
    close_sock(sock);
    return -1;
  }
  // 4. 发送内容
  if (write_sock(sock, buf->data, buf->len) != (ssize_t)buf->len) {
    buffer_free(buf);
    close_sock(sock);
    return -1;
  }
  buffer_free(buf);
  int ret = rpc_get_rsp(sock, result, callSN);
  close_sock(sock);
  return ret;
}
