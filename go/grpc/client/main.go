package main

import (
	"context"
	"log"
	"t/netmsg"
	"time"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func main() {
	// 建立连接
	conn, err := grpc.NewClient("localhost:8090", grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		log.Fatalf("连接失败: %v", err)
	}
	defer conn.Close()

	// 创建客户端
	client := netmsg.NewGreeterClient(conn)

	// 发起 RPC 调用
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	resp, err := client.SayHello(ctx, &netmsg.HelloRequest{Name: "World"})
	if err != nil {
		log.Fatalf("调用失败: %v", err)
	}

	log.Printf("收到响应: %s", resp.GetMessage())
}
