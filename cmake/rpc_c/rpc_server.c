#include <pthread.h>  // 用于简单的互斥锁
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "rpc_c.h"

#define MAX_HANDLERS 128

typedef struct {
  char name[MAX_FUNC_NAME_LEN];
  rpc_handler_t handler;
} handler_entry;

static handler_entry handlers[MAX_HANDLERS];
static int handler_count = 0;
static pthread_mutex_t handlers_lock = PTHREAD_MUTEX_INITIALIZER;

int rpc_server_register(const char* name, rpc_handler_t handler) {
  pthread_mutex_lock(&handlers_lock);
  if (handler_count >= MAX_HANDLERS) {
    pthread_mutex_unlock(&handlers_lock);
    return -1;
  }
  strncpy(handlers[handler_count].name, name, MAX_FUNC_NAME_LEN - 1);
  handlers[handler_count].name[MAX_FUNC_NAME_LEN - 1] = '\0';
  handlers[handler_count].handler = handler;
  handler_count++;
  pthread_mutex_unlock(&handlers_lock);
  return 0;
}

static rpc_handler_t find_handler(const char* name) {
  pthread_mutex_lock(&handlers_lock);
  for (int i = 0; i < handler_count; i++) {
    if (strcmp(handlers[i].name, name) == 0) {
      pthread_mutex_unlock(&handlers_lock);
      return handlers[i].handler;
    }
  }
  pthread_mutex_unlock(&handlers_lock);
  return NULL;
}

/* 处理一个客户端连接 */
static void handle_client(SOCKET client_fd) {
  printf("accept socket:%d\n", (int)client_fd);
  // 设置超时
  rpc_set_sock_timeout(client_fd);

  uint32_t msg_len = rpc_get_msg_len(client_fd);
  if (msg_len == 0 || msg_len > MAX_MSG_LEN) {
    close_sock(client_fd);
    return;
  }

  // 读取消息体
  char* msg = malloc(msg_len);
  if (!msg) {
    close_sock(client_fd);
    return;
  }
  if (read_sock_until(client_fd, msg, msg_len)) {
    free(msg);
    close_sock(client_fd);
    return;
  }
  // 解析请求
  reader_t r;
  reader_init(&r, msg, msg_len);
  uint16_t name_len = 0;
  if (reader_read_bytes(&r, &name_len, sizeof(uint16_t)) < 0 ||
      name_len >= MAX_FUNC_NAME_LEN) {
    free(msg);
    close_sock(client_fd);
    return;
  }
  char func_name[MAX_FUNC_NAME_LEN];
  if (reader_read_bytes(&r, func_name, name_len) < 0) {
    free(msg);
    close_sock(client_fd);
    return;
  }
  func_name[name_len] = '\0';

  // 查找处理函数
  rpc_handler_t handler = find_handler(func_name);
  buffer_t* out_buf = buffer_new(256);
  if (!handler) {
    // 未找到函数，返回错误
    buffer_append_U8(out_buf, RPC_ERR);
    const char* err = "Function not found";
    buffer_append_U32(out_buf, strlen(err));
    buffer_append(out_buf, err, strlen(err));
  } else {
    // 调用处理函数
    handler(&r, out_buf);
  }

  // 发送响应：先发送长度（4字节网络序），再发送数据
  uint32_t out_len = out_buf->len;
  write_sock(client_fd, (const char*)&out_len, 4);
  write_sock(client_fd, out_buf->data, out_buf->len);

  free(msg);
  buffer_free(out_buf);
  close_sock(client_fd);
}

static bool is_running = false;
void on_sig(int sig) {
  is_running = false;
}

void rpc_server_run(int port) {
  SOCKET listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    perror("socket");
    return;
  }

  int opt = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt,
             sizeof(opt));

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close_sock(listen_fd);
    return;
  }

  if (listen(listen_fd, 5) < 0) {
    perror("listen");
    close_sock(listen_fd);
    return;
  }

  (void)signal(SIGINT, on_sig);
  (void)signal(SIGTERM, on_sig);

  is_running = true;

  fd_set read_fds;
  SOCKET max_fd = listen_fd;
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  while (is_running) {
    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &read_fds);
    // 简单起见，只处理监听套接字，接受连接后立即处理，处理完关闭
    // 也可以使用select同时处理多个客户端，这里简化：每接受一个连接就串行处理
    if (select(max_fd + 1, &read_fds, NULL, NULL, &tv) < 0) {
      perror("select");
      break;
    }
    if (FD_ISSET(listen_fd, &read_fds)) {
      struct sockaddr_in client_addr;
      int client_len = sizeof(client_addr);
      int client_fd =
          accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
      if (client_fd >= 0) {
        // 处理客户端，然后关闭
        handle_client(client_fd);
      }
    }
  }
  close_sock(listen_fd);
  printf("%s", "server exit ...\n");
}
