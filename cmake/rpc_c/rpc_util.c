#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpc_c.h"

// RPC超时时间
static uint32_t g_rpc_timeout_sec = 0;

void rpc_set_timeout(uint32_t seconds) {
  g_rpc_timeout_sec = seconds;
}

uint32_t rpc_get_timeout() {
  return g_rpc_timeout_sec;
}

#ifdef _WIN32
void set_sock_timeout(SOCKET sock, uint32_t second) {
  if (second > 0) {
    DWORD tv = second * 1000;  // 在Windows下的单位是毫秒
    int n =
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    if (n < 0) {
      perror("set timeout err");
    }
    n = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
    if (n < 0) {
      perror("set timeout err");
    }
  }
}
#else
void set_sock_timeout(SOCKET sock, uint32_t second) {
  if (second > 0) {
    struct timeval tv;
    tv.tv_sec = second;
    tv.tv_usec = 0;
    int n =
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    if (n < 0) {
      perror("set timeout err");
    }
    n = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
    if (n < 0) {
      perror("set timeout err");
    }
  }
}
#endif

void rpc_set_sock_timeout(SOCKET sock) {
  set_sock_timeout(sock, g_rpc_timeout_sec);
}

buffer_t* buffer_new(size_t initial_cap) {
  buffer_t* buf = malloc(sizeof(buffer_t));
  if (!buf) {
    return NULL;
  }
  buf->data = malloc(initial_cap);
  if (!buf->data) {
    free(buf);
    return NULL;
  }
  buf->cap = initial_cap;
  buf->len = 0;
  return buf;
}

void buffer_free(buffer_t* buf) {
  if (buf) {
    free(buf->data);
    free(buf);
  }
}

int buffer_append(buffer_t* buf, const void* src, size_t n) {
  if (buf->len + n > buf->cap) {
    size_t new_cap = buf->cap * 2;
    if (new_cap < buf->len + n) {
      new_cap = buf->len + n;
    }
    char* new_data = realloc(buf->data, new_cap);
    if (!new_data) {
      return -1;
    }
    buf->data = new_data;
    buf->cap = new_cap;
  }
  memcpy(buf->data + buf->len, src, n);
  buf->len += n;
  return 0;
}

int buffer_append_U32(buffer_t* buf, uint32_t val) {
  return buffer_append(buf, &val, sizeof(val));
}

int buffer_append_U8(buffer_t* buf, uint8_t val) {
  return buffer_append(buf, &val, sizeof(val));
}

int buffer_append_string(buffer_t* buf, const char* str) {
  size_t len = strlen(str);
  if (buffer_append_U32(buf, len) < 0) {
    return -1;
  }
  return buffer_append(buf, str, len);
}

int buffer_append_dt(buffer_t* buf, data_type_t dt, const void* src, int isAr) {
  int ret = -1;
  switch (dt) {
    case DT_S8P:
    case DT_U8P:
    case DT_S8CP:
    case DT_U8CP: {
      const char* str = *(const char**)src;
      if (isAr) {
        // 如果是数组，则其地址与数组本身是一样的，所以不需要取地址所在的值
        str = src;
      }
      uint16_t len = (uint16_t)strlen(str);
      ret = buffer_append(buf, &len, sizeof(len));
      if (ret) {
        break;
      }
      ret = buffer_append(buf, str, len);
    } break;
    case DT_S8:
    case DT_U8:
      ret = buffer_append(buf, src, sizeof(uint8_t));
      break;
    case DT_S16:
    case DT_U16:
      ret = buffer_append(buf, src, sizeof(uint16_t));
      break;
    case DT_S32:
    case DT_U32:
      ret = buffer_append(buf, src, sizeof(uint32_t));
      break;
    case DT_S64:
    case DT_U64:
      ret = buffer_append(buf, src, sizeof(uint64_t));
      break;
    case DT_PB: {
      const ProtobufCMessage* pb = *(const ProtobufCMessage**)src;
      size_t size = protobuf_c_message_get_packed_size(pb);
      uint8_t* out = malloc(size);
      if (out == NULL) {
        return -2;
      }
      size_t res = protobuf_c_message_pack(pb, out);
      ret = buffer_append(buf, &res, sizeof(uint16_t));
      if (ret) {
        free(out);
        return ret;
      }
      ret = buffer_append(buf, out, res);
      free(out);
    } break;
    default:
      assert(false);  // 不支持的类型
      break;
  }
  return ret;
}

void reader_init(reader_t* r, const char* data, size_t len) {
  r->data = data;
  r->len = len;
  r->pos = 0;
}

int reader_read_uint32(reader_t* r, uint32_t* out) {
  if (r->pos + 4 > r->len) {
    return -1;
  }
  uint32_t net = 0;
  memcpy(&net, r->data + r->pos, 4);
  *out = net;
  r->pos += 4;
  return 0;
}

int reader_read_uint8(reader_t* r, uint8_t* out) {
  if (r->pos + 1 > r->len) {
    return -1;
  }
  *out = r->data[r->pos];
  r->pos += 1;
  return 0;
}

int reader_read_bytes(reader_t* r, void* out, size_t n) {
  if (r->pos + n > r->len) {
    return -1;
  }
  memcpy(out, r->data + r->pos, n);
  r->pos += n;
  return 0;
}

int read_sock_until(SOCKET s, char* buf, size_t buf_len) {
  size_t remain = buf_len;
  char* p = buf;
  while (remain > 0) {
    int n = read_sock(s, p, remain);
    if (n <= 0) {
      return -1;
    }
    remain -= n;
    p += n;
  }
  return 0;
}

static int parse_arg(reader_t* r, void* user_data, rpc_handle_arg_t handler) {
  data_type_t dt = 0;
  reader_read_bytes(r, &dt, sizeof(uint8_t));
  uint16_t len = 0;
  switch (dt) {
    case DT_S8:
    case DT_U8:
      len = sizeof(uint8_t);
      break;
    case DT_S16:
    case DT_U16:
      len = sizeof(uint16_t);
      break;
    case DT_S32:
    case DT_U32:
      len = sizeof(uint32_t);
      break;
    case DT_S64:
    case DT_U64:
      len = sizeof(uint64_t);
      break;
    default:
      break;
  }
  if (len > 0) {
    uint64_t v = 0;
    if (reader_read_bytes(r, &v, len)) {
      return 1;
    }
    if (handler(user_data, dt, &v, len)) {
      return 1;
    }
    return 0;
  }
  switch (dt) {
    case DT_PB:
    case DT_S8P:
    case DT_U8P:
    case DT_S8CP:
    case DT_U8CP: {
      if (reader_read_bytes(r, &len, sizeof(uint16_t))) {
        return 1;
      }
      char* buffer = malloc(len + 1);
      if (reader_read_bytes(r, buffer, len)) {
        return 1;
      }
      buffer[len] = 0;
      if (handler(user_data, dt, buffer, len)) {
        return 1;
      }
      return 0;
    }
    default:
      return 1;
  }
  return 0;
}

int rpc_parse_req_args(reader_t* r, void* user_data, rpc_handle_arg_t handler) {
  uint8_t narg = 0;
  reader_read_bytes(r, &narg, sizeof(narg));
  for (int i = 0; i < narg; ++i) {
    if (parse_arg(r, user_data, handler)) {
      return i + 1;
    }
  }
  return 0;
}

static int rsp_base_data(void* user_data,
                         data_type_t dt,
                         void* v,
                         uint16_t vlen) {
  object_t* bd = user_data;
  switch (dt) {
    case DT_S8:
    case DT_U8:
    case DT_S16:
    case DT_U16:
    case DT_S32:
    case DT_U32:
    case DT_S64:
    case DT_U64:
      bd->type = dt;
      bd->v.ll = *(int64_t*)v;
      return 0;
    case DT_S8P:
    case DT_U8P:
    case DT_S8CP:
    case DT_U8CP:
      bd->type = dt;
      bd->v.s = v;
      return 0;
    default:
      break;
  }
  return 1;
}

uint32_t rpc_get_msg_len(SOCKET sock) {
  uint32_t len = 0;
  if (read_sock(sock, (char*)&len, 4) != 4) {
    return 0;
  }
  return len;
}

int rpc_get_rsp(SOCKET sock, rpc_rsp_t* rspData) {
  uint32_t resp_len = rpc_get_msg_len(sock);
  if (resp_len == 0) {
    return -1;
  }

  // 读取响应体
  char* rsp = malloc(resp_len);
  if (!rsp) {
    return -1;
  }
  if (read_sock_until(sock, rsp, resp_len) != 0) {
    free(rsp);
    return -1;
  }

  reader_t r;
  reader_init(&r, rsp, resp_len);
  uint8_t result_type = 0;
  if (reader_read_uint8(&r, &result_type) < 0) {
    free(rsp);
    return -1;
  }
  rspData->res = result_type;
  int ret = rpc_parse_req_args(&r, &rspData->data, rsp_base_data);
  free(rsp);
  return ret;
}

void rpc_rsp_release(rpc_rsp_t* rsp) {
  switch (rsp->data.type) {
    case DT_S8P:
    case DT_U8P:
    case DT_S8CP:
    case DT_U8CP:
      free((void*)rsp->data.v.s);
      rsp->data.v.s = NULL;
      break;
    default:
      break;
  }
  rsp->data.type = 0;
}
