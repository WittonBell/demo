#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <grpc/byte_buffer.h>
#include <grpc/byte_buffer_reader.h>
#include <grpc/credentials.h>
#include <grpc/grpc.h>
#include <grpc/grpc_security.h>
#include <grpc/status.h>
#include <grpc/support/time.h>

#include "helloworld.pb-c.h"

#define SERVER_ADDR "127.0.0.1:8090"
#define METHOD_NAME "/helloworld.Greeter/SayHello"

int main(int argc, char** argv) {
  const char* name = "World";

  if (argc > 1) {
    name = argv[1];
  }

  printf("========================================\n");
  printf("gRPC Client\n");
  printf("========================================\n");
  printf("gRPC Version: %s\n", grpc_version_string());
  printf("Server:       %s\n", SERVER_ADDR);
  printf("Method:       %s\n", METHOD_NAME);
  printf("Name:         %s\n", name);
  printf("========================================\n");

  // 1. 初始化 gRPC
  grpc_init();

  // 2. 创建 Completion Queue
  grpc_completion_queue* cq = grpc_completion_queue_create_for_next(NULL);
  if (cq == NULL) {
    (void)fprintf(stderr, "grpc_completion_queue_create_for_next failed\n");
    grpc_shutdown();
    return 1;
  }

  // 3. 创建 channel
  grpc_channel_credentials* creds = grpc_insecure_credentials_create();
  if (creds == NULL) {
    (void)fprintf(stderr, "grpc_insecure_credentials_create failed\n");
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();
    return 1;
  }
  grpc_channel* channel = grpc_channel_create(SERVER_ADDR, creds, NULL);
  // 不需要creds了，释放内存
  grpc_channel_credentials_release(creds);
  if (channel == NULL) {
    (void)fprintf(stderr, "grpc_channel_create failed\n");
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();
    return 1;
  }

  // 4. 创建 RPC Call
  grpc_slice method = grpc_slice_from_static_string(METHOD_NAME);
  // 设置超时时间，deadline 是一个绝对时间。
  gpr_timespec deadline = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME),
                                       gpr_time_from_seconds(10, GPR_TIMESPAN));
  grpc_call* call = grpc_channel_create_call(
      channel, NULL, GRPC_PROPAGATE_DEFAULTS, cq, method, NULL, deadline, NULL);
  // 释放method
  grpc_slice_unref(method);
  if (call == NULL) {
    (void)fprintf(stderr, "grpc_channel_create_call failed\n");
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();
    return 1;
  }

  // 5. 构造 HelloRequest
  Helloworld__HelloRequest request = HELLOWORLD__HELLO_REQUEST__INIT;
  request.name = (char*)name;
  size_t request_size = helloworld__hello_request__get_packed_size(&request);
  printf("Request protobuf size: %zu bytes\n", request_size);

  void* request_buf = malloc(request_size);
  if (request_buf == NULL) {
    (void)fprintf(stderr, "malloc failed\n");
    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  helloworld__hello_request__pack(&request, request_buf);

  // 6. protobuf -> grpc_byte_buffer
  grpc_slice request_slice =
      grpc_slice_from_copied_buffer(request_buf, request_size);

  free(request_buf);

  grpc_byte_buffer* request_payload =
      grpc_raw_byte_buffer_create(&request_slice, 1);
  grpc_slice_unref(request_slice);

  if (request_payload == NULL) {
    (void)fprintf(stderr, "grpc_raw_byte_buffer_create failed\n");
    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  // 7. 准备接收响应变量
  grpc_byte_buffer* response_payload = NULL;
  grpc_status_code status = GRPC_STATUS_UNKNOWN;

  // 必须是有效的 grpc_slice。不要简单依赖 calloc/未初始化内存。
  grpc_slice status_details = grpc_empty_slice();

  // 8. 初始化接收 metadata
  grpc_metadata_array initial_metadata;
  grpc_metadata_array trailing_metadata;
  grpc_metadata_array_init(&initial_metadata);
  grpc_metadata_array_init(&trailing_metadata);

  // 9. 一个 batch 完成整个 unary RPC
  grpc_op ops[6];
  memset(ops, 0, sizeof(ops));

  // 发送 initial metadata
  ops[0].op = GRPC_OP_SEND_INITIAL_METADATA;
  ops[0].data.send_initial_metadata.count = 0;

  // 发送 protobuf request
  ops[1].op = GRPC_OP_SEND_MESSAGE;
  ops[1].data.send_message.send_message = request_payload;

  // 告诉服务器：客户端请求发送完毕
  ops[2].op = GRPC_OP_SEND_CLOSE_FROM_CLIENT;

  // 接收 initial metadata 不能：recv_initial_metadata = NULL
  // 必须传：&initial_metadata
  ops[3].op = GRPC_OP_RECV_INITIAL_METADATA;
  ops[3].data.recv_initial_metadata.recv_initial_metadata = &initial_metadata;

  // 接收 protobuf response
  ops[4].op = GRPC_OP_RECV_MESSAGE;
  ops[4].data.recv_message.recv_message = &response_payload;

  // 接收 RPC status
  ops[5].op = GRPC_OP_RECV_STATUS_ON_CLIENT;
  ops[5].data.recv_status_on_client.trailing_metadata = &trailing_metadata;
  ops[5].data.recv_status_on_client.status = &status;
  ops[5].data.recv_status_on_client.status_details = &status_details;

  // 10. 启动 batch
  printf("Starting RPC batch...\n");

  grpc_call_error err = grpc_call_start_batch(call, ops, 6, (void*)1, NULL);
  if (err != GRPC_CALL_OK) {
    (void)fprintf(stderr, "grpc_call_start_batch failed: %d\n", err);

    grpc_byte_buffer_destroy(request_payload);
    grpc_slice_unref(status_details);
    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();
    return 1;
  }

  // grpc_call_start_batch() 成功以后，必须从对应的 completion queue 取
  // completion。
  printf("Waiting for RPC completion...\n");

  // 11. 等待整个 batch 完成
  grpc_event ev = grpc_completion_queue_next(cq, deadline, NULL);

  // 12. 检查 CQ 事件
  printf("CQ event received: type=%d success=%d tag=%p\n", ev.type, ev.success,
         ev.tag);

  if (ev.type == GRPC_QUEUE_TIMEOUT) {
    (void)fprintf(stderr, "grpc_completion_queue_next: TIMEOUT\n");
    grpc_call_cancel(call, NULL);
    grpc_byte_buffer_destroy(request_payload);
    grpc_slice_unref(status_details);
    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);
    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  if (ev.type == GRPC_QUEUE_SHUTDOWN) {
    (void)fprintf(stderr, "grpc_completion_queue_next: SHUTDOWN\n");

    grpc_byte_buffer_destroy(request_payload);
    grpc_slice_unref(status_details);
    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  if (ev.type != GRPC_OP_COMPLETE) {
    (void)fprintf(stderr, "Unexpected CQ event type: %d\n", ev.type);

    grpc_byte_buffer_destroy(request_payload);
    grpc_slice_unref(status_details);

    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  // 13. 检查 RPC 是否成功
  if (!ev.success) {
    (void)fprintf(stderr, "RPC batch completed with success=0\n");

    grpc_byte_buffer_destroy(request_payload);

    if (response_payload != NULL) {
      grpc_byte_buffer_destroy(response_payload);
    }

    grpc_slice_unref(status_details);
    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  // 14. 打印 RPC status
  printf("RPC status: %d\n", status);
  if (status != GRPC_STATUS_OK) {
    (void)fprintf(stderr, "RPC failed: %d\n", status);

    if (GRPC_SLICE_LENGTH(status_details) > 0) {
      (void)fprintf(stderr, "Status details: %.*s\n",
                    (int)GRPC_SLICE_LENGTH(status_details),
                    GRPC_SLICE_START_PTR(status_details));
    }

    grpc_byte_buffer_destroy(request_payload);
    if (response_payload != NULL) {
      grpc_byte_buffer_destroy(response_payload);
    }

    grpc_slice_unref(status_details);
    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();
    return 1;
  }

  // 15. grpc_byte_buffer -> protobuf
  if (response_payload == NULL) {
    (void)fprintf(stderr, "Server returned no response message\n");
    grpc_byte_buffer_destroy(request_payload);
    grpc_slice_unref(status_details);

    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  grpc_byte_buffer_reader reader;
  if (!grpc_byte_buffer_reader_init(&reader, response_payload)) {
    (void)fprintf(stderr, "grpc_byte_buffer_reader_init failed\n");

    grpc_byte_buffer_destroy(request_payload);
    grpc_byte_buffer_destroy(response_payload);

    grpc_slice_unref(status_details);
    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  grpc_slice response_slice = grpc_byte_buffer_reader_readall(&reader);
  printf("Response pb size: %zu bytes\n", GRPC_SLICE_LENGTH(response_slice));

  // 16. protobuf-c 解码
  Helloworld__HelloReply* reply =
      helloworld__hello_reply__unpack(NULL, GRPC_SLICE_LENGTH(response_slice),
                                      GRPC_SLICE_START_PTR(response_slice));
  grpc_slice_unref(response_slice);
  if (reply == NULL) {
    (void)fprintf(stderr, "helloworld__hello_reply__unpack failed\n");
    grpc_byte_buffer_destroy(request_payload);
    grpc_byte_buffer_destroy(response_payload);
    grpc_slice_unref(status_details);
    grpc_metadata_array_destroy(&initial_metadata);
    grpc_metadata_array_destroy(&trailing_metadata);

    grpc_call_unref(call);
    grpc_channel_destroy(channel);
    grpc_completion_queue_destroy(cq);
    grpc_shutdown();

    return 1;
  }

  // 17. 输出结果
  printf("\n");
  printf("========================================\n");
  printf("RPC SUCCESS\n");
  printf("========================================\n");
  printf("Request : %s\n", name);
  printf("Response: %s\n", reply->message ? reply->message : "(null)");
  printf("========================================\n");

  // 18. 释放 protobuf response
  helloworld__hello_reply__free_unpacked(reply, NULL);

  // 19. 释放资源
  grpc_byte_buffer_destroy(request_payload);
  grpc_byte_buffer_destroy(response_payload);
  grpc_slice_unref(status_details);
  grpc_metadata_array_destroy(&initial_metadata);
  grpc_metadata_array_destroy(&trailing_metadata);
  grpc_call_unref(call);
  grpc_channel_destroy(channel);

  grpc_completion_queue_shutdown(cq);
  while (1) {
    grpc_event shutdown_ev = grpc_completion_queue_next(
        cq, gpr_inf_future(GPR_CLOCK_REALTIME), NULL);
    if (shutdown_ev.type == GRPC_QUEUE_SHUTDOWN) {
      break;
    }
  }

  grpc_completion_queue_destroy(cq);
  grpc_shutdown();
  printf("Client finished.\n");
  return 0;
}
