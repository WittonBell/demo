#include <grpcpp/grpcpp.h>
#include "hello.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using std::string, std::cout, std::unique_ptr;

namespace {

class GreeterService : public Greeter::Service {
 public:
  Status SayHello(ServerContext* context,
                  const HelloRequest* request,
                  HelloReply* response) override {
	cout << "recv msg: " << request->name() << "\n";
    string prefix("Hello ");
    response->set_message(prefix + request->name());
    return Status::OK;
  }
};

}  // namespace

int main() {
  std::string server_address("0.0.0.0:8090");
  GreeterService service;

  ServerBuilder builder;
  // 监听端口，使用不安全连接（仅用于测试）
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // 注册服务
  builder.RegisterService(&service);

  // 启动服务
  unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << server_address << "\n";

  // 阻塞等待
  server->Wait();
  return 0;
}
