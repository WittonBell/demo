#ifndef RPC_H
#define RPC_H

#ifdef _WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#endif

#include <stddef.h>
#include <stdint.h>

#include <protobuf-c/protobuf-c.h>

#if __STDC_VERSION__ > 201710L
#undef NULL
#define NULL nullptr
#endif

// C 语言基本数据类型
typedef enum {
  DT_UNKNOWN,
  DT_S8,
  DT_U8,
  DT_S16,
  DT_U16,
  DT_S32,
  DT_U32,
  DT_S64,
  DT_U64,
  DT_S8P,   // char*
  DT_U8P,   // unsigned char*
  DT_S16P,  // short*
  DT_U16P,  // unsigned short*
  DT_S32P,
  DT_U32P,
  DT_S64P,
  DT_U64P,
  DT_S8CP,  // const char*
  DT_U8CP,  // const unsigned char*
  DT_S16CP,
  DT_U16CP,
  DT_S32CP,
  DT_U32CP,
  DT_S64CP,
  DT_U64CP,
  DT_VP,   // void*
  DT_CVP,  // const void*
  DT_PB,   // ProtoBuf-C
} data_type_t;

typedef struct {
  data_type_t type;
  union {
    char c;
    unsigned char b;
    int i;
    unsigned int u;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
    float f;
    double d;
    const char* s;
    const unsigned char* bs;
  } v;
} object_t;

#define IS_PROTOBUF_MSG(ptr)                                    \
  ((ptr) && ((ProtobufCMessage*)((size_t)(ptr)))->descriptor && \
   ((ProtobufCMessage*)((size_t)(ptr)))->descriptor->magic ==   \
       PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC)

#define DISPATCH_TYPE(PREFIX, x)                \
  _Generic((x),                                 \
      char: PREFIX##S8,                         \
      unsigned char: PREFIX##U8,                \
      short: PREFIX##S16,                       \
      unsigned short: PREFIX##U16,              \
      int: PREFIX##S32,                         \
      unsigned int: PREFIX##U32,                \
      long long: PREFIX##S64,                   \
      unsigned long long: PREFIX##U64,          \
      char*: PREFIX##S8P,                       \
      unsigned char*: PREFIX##U8P,              \
      short*: PREFIX##S16P,                     \
      unsigned short*: PREFIX##U16P,            \
      int*: PREFIX##S32P,                       \
      unsigned int*: PREFIX##U32P,              \
      long long*: PREFIX##S64P,                 \
      unsigned long long*: PREFIX##U64P,        \
      const char*: PREFIX##S8CP,                \
      const unsigned char*: PREFIX##U8CP,       \
      const short*: PREFIX##S16CP,              \
      const unsigned short*: PREFIX##U16CP,     \
      const int*: PREFIX##S32CP,                \
      const unsigned int*: PREFIX##U32CP,       \
      const long long*: PREFIX##S64CP,          \
      const unsigned long long*: PREFIX##U64CP, \
      void*: PREFIX##VP,                        \
      const void*: PREFIX##CVP,                 \
      default: PREFIX##UNKNOWN)

#define GET_TYPE(x)                                           \
  DISPATCH_TYPE(DT_, x) != DT_UNKNOWN ? DISPATCH_TYPE(DT_, x) \
  : IS_PROTOBUF_MSG(x)                ? DT_PB                 \
                                      : DT_UNKNOWN

#define GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, n, ...) n

#define COUNT_ARGS(...) \
  GET_MACRO(0, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define APPLY0(m, a) 0  // 这里定义为0，避免编译器警告
#define APPLY1(m, a) m(a)
#define APPLY2(m, a, ...) \
  m(a);                   \
  APPLY1(m, __VA_ARGS__)
#define APPLY3(m, a, ...) \
  m(a);                   \
  APPLY2(m, __VA_ARGS__)
#define APPLY4(m, a, ...) \
  m(a);                   \
  APPLY3(m, __VA_ARGS__)
#define APPLY5(m, a, ...) \
  m(a);                   \
  APPLY4(m, __VA_ARGS__)
#define APPLY6(m, a, ...) \
  m(a);                   \
  APPLY5(m, __VA_ARGS__)
#define APPLY7(m, a, ...) \
  m(a);                   \
  APPLY6(m, __VA_ARGS__)
#define APPLY8(m, a, ...) \
  m(a);                   \
  APPLY7(m, __VA_ARGS__)
#define APPLY9(m, a, ...) \
  m(a);                   \
  APPLY8(m, __VA_ARGS__)
#define APPLY10(m, a, ...) \
  m(a);                    \
  APPLY9(m, __VA_ARGS__)

#define APPLY(m, ...)                                                          \
  GET_MACRO(0, ##__VA_ARGS__, APPLY10, APPLY9, APPLY8, APPLY7, APPLY6, APPLY5, \
            APPLY4, APPLY3, APPLY2, APPLY1, APPLY0)(m, __VA_ARGS__)

// 由于数组变量ar的地址&ar与数组变量ar本身是一样的，而指针不一样，
// 所以利用这个特性来判断是否是数组
#define BUFFER_PUSH_ARG(x)                            \
  {                                                   \
    char dt = GET_TYPE(x);                            \
    buffer_append(buf, (const void*)&dt, sizeof(dt)); \
    buffer_append_dt(buf, dt, (const void*)&(x),      \
                     ((size_t)(x) == (size_t)&(x)));  \
  }

#define BUFFER_PUSH_ARGS(...)              \
  {                                        \
    uint8_t n = COUNT_ARGS(__VA_ARGS__);   \
    buffer_append_U8(buf, n);              \
    APPLY(BUFFER_PUSH_ARG, ##__VA_ARGS__); \
  }

#define RPC_MAKE_REQ(func, ...)              \
  {                                          \
    buf = buffer_new(1024);                  \
    uint16_t size = (uint16_t)strlen(func);  \
    buffer_append(buf, &size, sizeof(size)); \
    buffer_append(buf, func, size);          \
    BUFFER_PUSH_ARGS(__VA_ARGS__);           \
  }

#define RPC_MAKE_RSP(RES, ...)           \
  {                                      \
    buffer_append_U8(buf, (uint8_t)RES); \
    BUFFER_PUSH_ARGS(__VA_ARGS__);       \
  }

// 返回值类型标识
typedef enum rpc_result {
  RPC_OK = 0,
  RPC_ERR = 1,
} rpc_result_t;

// 最大消息长度限制，防止恶意攻击
#define MAX_MSG_LEN 65536
#define MAX_FUNC_NAME_LEN 256

// 简单的动态缓冲区，用于序列化
typedef struct buffer {
  char* data;
  size_t cap;
  size_t len;
} buffer_t;

buffer_t* buffer_new(size_t initial_cap);
void buffer_free(buffer_t* buf);
int buffer_append(buffer_t* buf, const void* src, size_t n);
int buffer_append_U32(buffer_t* buf, uint32_t val);
int buffer_append_U8(buffer_t* buf, uint8_t val);
int buffer_append_string(buffer_t* buf, const char* str);
int buffer_append_dt(buffer_t* buf, data_type_t dt, const void* src, int isAr);

// 反序列化读取器
typedef struct reader {
  const char* data;
  size_t len;
  size_t pos;
} reader_t;

void reader_init(reader_t* r, const char* data, size_t len);
int reader_read_uint32(reader_t* r, uint32_t* out);
int reader_read_uint8(reader_t* r, uint8_t* out);
int reader_read_bytes(reader_t* r, void* out, size_t n);

// 服务器函数原型：参数为参数数组和返回值缓冲区
typedef void (*rpc_handler_t)(reader_t* args, buffer_t* out);
typedef int (*rpc_handle_arg_t)(void* user_data,
                                data_type_t dt,
                                void* v,
                                uint16_t vlen);

// 注册函数
int rpc_server_register(const char* name, rpc_handler_t handler);

// 启动服务器（阻塞）
void rpc_server_run(int port);

// 设置网络读写超时时间
void rpc_set_timeout(uint32_t seconds);
uint32_t rpc_get_timeout();
void rpc_set_sock_timeout(SOCKET sock);

typedef struct {
  rpc_result_t res;
  object_t data;
} rpc_rsp_t;

void rpc_rsp_release(rpc_rsp_t* rsp);
// 客户端调用接口（同步）
int rpc_call(const char* host, int port, buffer_t* buf, rpc_rsp_t* result);

uint32_t rpc_get_msg_len(SOCKET sock);
int rpc_parse_req_args(reader_t* r, void* user_data, rpc_handle_arg_t handler);
int rpc_get_rsp(SOCKET sock, rpc_rsp_t* rspData);

// 读取buf_len字节网络数据
// 成功返回0
int read_sock_until(SOCKET s, char* buf, size_t buf_len);

#ifdef _WIN32
static inline int read_sock(SOCKET s, char* buf, size_t len) {
  return recv(s, buf, (int)len, 0);
}

static inline int write_sock(SOCKET s, const char* buf, size_t len) {
  return send(s, buf, (int)len, 0);
}

static inline int close_sock(SOCKET s) {
  return closesocket(s);
}
#else
static inline int read_sock(SOCKET s, char* buf, int len) {
  return read(s, buf, len);
}

static inline int write_sock(SOCKET s, const char* buf, int len) {
  return write(s, buf, len);
}

static inline int close_sock(SOCKET s) {
  return close(s);
}
#endif

#endif
