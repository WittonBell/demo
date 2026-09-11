#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <grpc/byte_buffer.h>
#include <grpc/byte_buffer_reader.h>
#include <grpc/credentials.h>
#include <grpc/grpc.h>
#include <grpc/grpc_security.h>
#include <grpc/impl/codegen/status.h>
#include <grpc/support/time.h>

#include "helloworld.pb-c.h"


#define SERVER_ADDR "0.0.0.0:8090"
#define METHOD_NAME "/helloworld.Greeter/SayHello"

typedef enum {
  TAG_NEW_CALL = 0, // 新调用到达
  TAG_FINISH = 1    // 发送响应完成
} tag_type;

typedef struct {
  tag_type type;
  grpc_call* call;
  gpr_timespec deadline;
  grpc_metadata_array metadata;
  grpc_byte_buffer* payload;
} server_tag;

// 发起一次“等待新调用”的异步请求
static void request_new_call(grpc_server* server,
                             void* registered_method,
                             grpc_completion_queue* cq) {
  server_tag* tag = (server_tag*)calloc(1, sizeof(server_tag));
  tag->type = TAG_NEW_CALL;
  tag->payload = NULL;
  grpc_metadata_array_init(&tag->metadata);

  grpc_call_error err = grpc_server_request_registered_call(
      server, registered_method, &tag->call, &tag->deadline, &tag->metadata,
      &tag->payload, cq,  // cq_bound_to_call
      cq,                 /// cq_for_notification
      tag);

  if (err != GRPC_CALL_OK) {
    (void)fprintf(stderr, "grpc_server_request_registered_call failed: %d\n",
                  err);
    grpc_metadata_array_destroy(&tag->metadata);
    free(tag);
  }
}

static void handle_new_call(server_tag* tag) {
  // 反序列化请求
  Helloworld__HelloRequest* request = NULL;
  if (tag->payload != NULL) {
    grpc_byte_buffer_reader reader;
    if (grpc_byte_buffer_reader_init(&reader, tag->payload)) {
      grpc_slice slice = grpc_byte_buffer_reader_readall(&reader);
      request = helloworld__hello_request__unpack(
          NULL, GRPC_SLICE_LENGTH(slice), GRPC_SLICE_START_PTR(slice));
      grpc_slice_unref(slice);
    }
    grpc_byte_buffer_destroy(tag->payload);
    tag->payload = NULL;
  }

  const char* name = (request && request->name) ? request->name : "unknown";
  printf("Received request from: %s\n", name);

  // 构造响应
  Helloworld__HelloReply reply = HELLOWORLD__HELLO_REPLY__INIT;
  char response_msg[256];
  (void)snprintf(response_msg, sizeof(response_msg), "Hello, %s!", name);
  reply.message = response_msg;

  size_t reply_size = helloworld__hello_reply__get_packed_size(&reply);
  void* reply_buf = malloc(reply_size);
  helloworld__hello_reply__pack(&reply, reply_buf);

  grpc_slice reply_slice = grpc_slice_from_copied_buffer(reply_buf, reply_size);
  grpc_byte_buffer* reply_bb = grpc_raw_byte_buffer_create(&reply_slice, 1);

  // 组装并提交发送批次
  grpc_op ops[3];
  memset(ops, 0, sizeof(ops));

  ops[0].op = GRPC_OP_SEND_INITIAL_METADATA;
  ops[1].op = GRPC_OP_SEND_MESSAGE;
  ops[1].data.send_message.send_message = reply_bb;
  ops[2].op = GRPC_OP_SEND_STATUS_FROM_SERVER;
  ops[2].data.send_status_from_server.status = GRPC_STATUS_OK;
  ops[2].data.send_status_from_server.trailing_metadata_count = 0;

  server_tag* finish_tag = (server_tag*)calloc(1, sizeof(server_tag));
  finish_tag->type = TAG_FINISH;
  finish_tag->call = tag->call;

  grpc_call_error err =
      grpc_call_start_batch(tag->call, ops, 3, finish_tag, NULL);
  if (err != GRPC_CALL_OK) {
    (void)fprintf(stderr, "grpc_call_start_batch failed: %d\n", err);
    grpc_call_unref(finish_tag->call);
    free(finish_tag);
  }
  // 清理本次请求相关的临时资源
  if (request != NULL) {
    helloworld__hello_request__free_unpacked(request, NULL);
  }
  grpc_slice_unref(reply_slice);
  grpc_byte_buffer_destroy(reply_bb);
  free(reply_buf);

  grpc_metadata_array_destroy(&tag->metadata);
  free(tag);
}

int main(int argc, char* argv[]) {
  // 1. 初始化 gRPC 运行时
  grpc_init();
  printf("gRPC Version: %s\n", grpc_version_string());
  // 2. 创建完成队列与服务端
  grpc_completion_queue* cq = grpc_completion_queue_create_for_next(NULL);
  grpc_server* server = grpc_server_create(NULL, NULL);

  grpc_server_credentials* server_creds =
      grpc_insecure_server_credentials_create();
  // 3. 添加明文 HTTP/2 端口 (替代凭证创建步骤)
  int port = grpc_server_add_http2_port(server, SERVER_ADDR, server_creds);
  if (port <= 0) {
    (void)fprintf(stderr, "Failed to bind to %s\n", SERVER_ADDR);
    grpc_server_destroy(server);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();
    return 1;
  }

  // 4. 将完成队列注册到服务端
  grpc_server_register_completion_queue(server, cq, NULL);

  // 5. 注册服务方法
  void* registered_method = grpc_server_register_method(
      server, METHOD_NAME, NULL, GRPC_SRM_PAYLOAD_READ_INITIAL_BYTE_BUFFER, 0);

  if (registered_method == NULL) {
    (void)fprintf(stderr, "Failed to register method %s\n", METHOD_NAME);
    grpc_server_destroy(server);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();
    return 1;
  }

  // 6. 启动服务端
  grpc_server_start(server);
  printf("Server listening on port %d\n", port);

  // 7. 发起第一次等待新调用的请求
  request_new_call(server, registered_method, cq);

  // 8. 主事件循环
  while (1) {
    grpc_event ev = grpc_completion_queue_next(
        cq, gpr_inf_future(GPR_CLOCK_REALTIME), NULL);
    if (ev.type == GRPC_QUEUE_TIMEOUT) {
      continue;
    }
    if (ev.type == GRPC_QUEUE_SHUTDOWN) {
      break;
    }

    server_tag* tag = (server_tag*)ev.tag;
    // 有新调用到达
    if (tag->type == TAG_NEW_CALL) {
      if (!ev.success) {
        grpc_metadata_array_destroy(&tag->metadata);
        free(tag);
        break;
      }
      handle_new_call(tag);
      // 立刻发起下一次等待新调用的请求
      request_new_call(server, registered_method, cq);
    }
    // 情况二：发送响应完成
    else if (tag->type == TAG_FINISH) {
      grpc_call_unref(tag->call);
      free(tag);
    }
  }

  // 9. 优雅关闭 (不再需要释放凭证)
  grpc_server_shutdown_and_notify(server, cq, NULL);
  grpc_completion_queue_shutdown(cq);

  while (
      grpc_completion_queue_next(cq, gpr_inf_future(GPR_CLOCK_REALTIME), NULL)
          .type != GRPC_QUEUE_SHUTDOWN) {
    // 继续等待
  }

  grpc_completion_queue_destroy(cq);
  grpc_server_destroy(server);
  grpc_shutdown();

  return 0;
}
