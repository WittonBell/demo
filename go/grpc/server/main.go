package main

import (
	"context"
	"log"
	"net"
	"t/netmsg"

	"google.golang.org/grpc"
)

// server 用于实现 Greeter 服务
type server struct {
	netmsg.UnimplementedGreeterServer
}

// SayHello 实现 GreeterServer 接口
func (s *server) SayHello(ctx context.Context, req *netmsg.HelloRequest) (*netmsg.HelloReply, error) {
	log.Printf("收到请求: %s", req.GetName())
	return &netmsg.HelloReply{Message: "Hello " + req.GetName()}, nil
}

func main() {
	listen, err := net.Listen("tcp", ":8090")
	if err != nil {
		log.Fatalf("监听失败: %v", err)
	}
	s := grpc.NewServer()
	netmsg.RegisterGreeterServer(s, new(server))
	log.Printf("服务端启动，监听 :8090")
	if err := s.Serve(listen); err != nil {
		log.Fatalf("服务启动失败: %v", err)
	}
}
