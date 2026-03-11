#include <protobuf-c/protobuf-c.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pbc/login.pb-c.h"
#include "rpc_c.h"

// 处理函数：加法
static int add(void* user_data, data_type_t dt, void* v, uint16_t vlen) {
  switch (dt) {
    case DT_S8:
    case DT_U8:
    case DT_S16:
    case DT_U16:
    case DT_S32:
    case DT_U32:
    case DT_S64:
    case DT_U64:
      *(int64_t*)user_data += *(int64_t*)v;
      return 0;
    default:
      break;
  }
  return 1;
}
static void sum_handler(reader_t* args, buffer_t* buf) {
  int64_t result = 0;
  int res = rpc_parse_req_args(args, &result, add);
  if (res == 0) {
    // 返回成功和结果
    RPC_MAKE_RSP(RPC_OK, result);
  } else {
    const char* err = "RPC call add failed!";
    RPC_MAKE_RSP(RPC_ERR, err);
  }
}

// 处理函数：字符串拼接
static int concat(void* user_data, data_type_t dt, void* v, uint16_t vlen) {
  switch (dt) {
    case DT_S8P:
    case DT_U8P:
    case DT_S8CP:
    case DT_U8CP:
      strcat(user_data, v);
      return 0;
    default:
      break;
  }
  return 1;
}
static void concat_handler(reader_t* args, buffer_t* buf) {
  char result[256];
  result[0] = 0;
  int res = rpc_parse_req_args(args, &result, concat);
  if (res == 0) {
    RPC_MAKE_RSP(RPC_OK, result);
  } else {
    const char* err = "RPC call concat failed!";
    RPC_MAKE_RSP(RPC_ERR, err);
  }
}

static int parse_login(void* user_data,
                       data_type_t dt,
                       void* v,
                       uint16_t vlen) {
  buffer_t* buf = user_data;
  if (dt != DT_PB) {
    const char* err = "RPC call login failed!";
    RPC_MAKE_RSP(RPC_ERR, err);
    return 1;
  }
  Netmsg__ReqLogin* login = netmsg__req_login__unpack(NULL, vlen, v);
  if (login && !strncmp(login->username, "witton", sizeof("witton")) &&
      !memcmp(login->psw.data, "1", sizeof("1"))) {
    RPC_MAKE_RSP(RPC_OK);
    return 0;
  }
  const char* err = "RPC call login failed!";
  RPC_MAKE_RSP(RPC_ERR, err);
  return 1;
}
static void login_handler(reader_t* args, buffer_t* buf) {
  rpc_parse_req_args(args, buf, parse_login);
}

static void server() {
  rpc_server_register("sum", sum_handler);
  rpc_server_register("concat", concat_handler);
  rpc_server_register("login", login_handler);
  printf("Starting RPC server on port 8080...\n");
  rpc_server_run(8080);
}

// 客户端调用示例
static void rpc_call_login() {
  buffer_t* buf = NULL;
  rpc_rsp_t result;

  Netmsg__ReqLogin login;
  netmsg__req_login__init(&login);
  login.username = "witton";
  login.psw.data = (uint8_t*)"1";
  login.psw.len = 1;
  Netmsg__ReqLogin* p = &login;
  RPC_MAKE_REQ("login", p);
  if (rpc_call("127.0.0.1", 8080, buf, &result) == 0) {
    printf("call login result: %s\n", result.res == RPC_OK ? "OK" : "failed");
  } else {
    printf("add failed:%s\n", result.res == RPC_OK ? "OK" : "failed");
  }
  rpc_rsp_release(&result);
}

static void rpc_call_add() {
  buffer_t* buf = NULL;
  rpc_rsp_t result;
  int a = 10;
  int b = 20;
  RPC_MAKE_REQ("sum", a)
  if (rpc_call("127.0.0.1", 8080, buf, &result) == 0) {
    printf("%d + %d result: %s %lld\n", a, b,
           result.res == RPC_OK ? "OK" : "failed", result.data.v.ll);
  } else {
    printf("add failed:%s\n", result.res == RPC_OK ? "OK" : "failed");
  }
  rpc_rsp_release(&result);
}

static void rpc_call_concat() {
  buffer_t* buf = NULL;
  rpc_rsp_t result;
  const char* s1 = "Hello, ";
  const char* s2 = "World!";
  const char* s3 = "你好，";
  const char* s4 = "世界！";
  RPC_MAKE_REQ("concat", s1, s2, s3, s4);
  if (rpc_call("127.0.0.1", 8080, buf, &result) == 0) {
    printf("concat `%s` `%s` `%s` `%s` result: %s `%s`\n", s1, s2, s3, s4,
           result.res == RPC_OK ? "OK" : "failed", result.data.v.s);
  } else {
    printf("concat failed:%s\n", result.res == RPC_OK ? "OK" : "failed");
  }
  rpc_rsp_release(&result);
}

static void client() {
  rpc_call_login();
  rpc_call_add();
  rpc_call_concat();
}

int main(int argc, char** argv) {
  if (argc < 2) {
    (void)fprintf(stderr, "Usage: %s [server|client]\n", argv[0]);
    return 1;
  }

#ifdef _WIN32
  WSADATA wsaData;
  WORD wVersionRequested = MAKEWORD(2, 2);
  int err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
    printf("WSAStartup failed with error: %d\n", err);
    return 1;
  }
  SetConsoleOutputCP(65001);
#endif
  // 设置网络读写超时时间为30秒
  rpc_set_timeout(30);

  if (strcmp(argv[1], "server") == 0) {
    server();
  } else if (strcmp(argv[1], "client") == 0) {
    client();
  } else {
    (void)fprintf(stderr, "Unknown command\n");
    return 1;
  }

  printf("exit now\n");

#ifdef _WIN32
  WSACleanup();
#endif
  return 0;
}
