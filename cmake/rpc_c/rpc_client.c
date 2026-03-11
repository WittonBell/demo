#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpc_c.h"

int rpc_call(const char* host, int port, buffer_t* buf, rpc_rsp_t* result) {
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

  // 发送请求：先4字节长度（网络序），再数据
  int res = write_sock(sock, (const char*)&buf->len, 4);
  if (res != 4) {
    buffer_free(buf);
    close_sock(sock);
    return -1;
  }
  if (write_sock(sock, buf->data, buf->len) != (ssize_t)buf->len) {
    buffer_free(buf);
    close_sock(sock);
    return -1;
  }
  buffer_free(buf);
  int ret = rpc_get_rsp(sock, result);
  close_sock(sock);
  return ret;
}
