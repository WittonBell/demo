#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 4096

int main() {
  struct io_uring ring;
  struct io_uring_sqe* sqe;
  struct io_uring_cqe* cqe;
  char buf[BUF_SIZE];
  int fd, ret;

  // 初始化 io_uring，队列深度为 16
  ret = io_uring_queue_init(16, &ring, 0);
  if (ret < 0) {
    fprintf(stderr, "queue_init failed: %s\n", strerror(-ret));
    return 1;
  }
  char* p = getcwd(buf, sizeof(buf));
  // 打开文件
  fd = open("../../../src/CMakeLists.txt", O_RDONLY);
  if (fd < 0) {
    perror("open");
    io_uring_queue_exit(&ring);
    return 1;
  }

  // 获取一个 SQE
  sqe = io_uring_get_sqe(&ring);
  if (!sqe) {
    fprintf(stderr, "get_sqe failed\n");
    close(fd);
    io_uring_queue_exit(&ring);
    return 1;
  }

  // 准备读取操作
  io_uring_prep_read(sqe, fd, buf, BUF_SIZE, 0);  // 从偏移量0开始读取

  // 提交请求
  ret = io_uring_submit(&ring);
  if (ret < 0) {
    fprintf(stderr, "submit failed: %s\n", strerror(-ret));
    close(fd);
    io_uring_queue_exit(&ring);
    return 1;
  }

  // 等待完成事件
  ret = io_uring_wait_cqe(&ring, &cqe);
  if (ret < 0) {
    fprintf(stderr, "wait_cqe failed: %s\n", strerror(-ret));
    close(fd);
    io_uring_queue_exit(&ring);
    return 1;
  }

  // 处理完成事件
  if (cqe->res < 0) {
    // 读取操作出错
    fprintf(stderr, "Async read failed: %s\n", strerror(-cqe->res));
  } else {
    // 读取成功，打印读取的字节和数据
    printf("Read %d bytes:\n", cqe->res);
    printf("%.*s\n", cqe->res, buf);  // 安全地打印读取的内容
  }

  // 标记 CQE 已处理
  io_uring_cqe_seen(&ring, cqe);

  // 清理资源
  close(fd);
  io_uring_queue_exit(&ring);

  return 0;
}