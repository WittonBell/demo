#include <grpcpp/grpcpp.h>
#include "hello.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using std::string, std::cout, std::shared_ptr;

namespace {
class GreeterClient {
 public:
  explicit GreeterClient(const std::shared_ptr<Channel>& channel)
      : stub_(Greeter::NewStub(channel)) {}

  // 发起 RPC 调用
  std::string SayHello(const std::string& user) {
    // 构造请求
    HelloRequest request;
    request.set_name(user);

    // 响应容器
    HelloReply reply;

    // 客户端上下文
    ClientContext context;

    // 发起 RPC 调用
    Status status = stub_->SayHello(&context, request, &reply);

    if (status.ok()) {
      return reply.message();
    }
    std::cout << "RPC failed: " << status.error_code() << ": "
              << status.error_message() << "\n";
    return "RPC failed";
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

} // namespace

int main() {
  // 连接服务器
  std::string target_str("localhost:8090");
  auto channel =
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials());

  GreeterClient greeter(channel);

  string user("world");
  string reply = greeter.SayHello(user);

  cout << "Greeter received: " << reply << "\n";

  return 0;
}
