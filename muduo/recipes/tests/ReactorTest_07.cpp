//
// Created by cleon on 22-10-20.
//

#include "muduo/recipes/EventLoop.h"
#include "muduo/recipes/Acceptor.h"
#include "muduo/recipes/InetAddress.h"
#include "muduo/recipes/SocketsOps.h"

#include <stdio.h>

using namespace muduo;
using namespace muduo::recipes;

void newConnection(int sockfd, const InetAddress& peerAddr) {
  printf("new connection: %s\n", peerAddr.toIpPort().c_str());
  ssize_t n = ::write(sockfd, "How are you?\n", 13);
  (void)n;
  sockets::close(sockfd);
}

int main() {
  printf("main(): pid = %d\n", ::getpid());
  InetAddress listenAddr(8888);
  EventLoop loop;

  Acceptor acceptor(&loop, listenAddr);
  acceptor.setNewConnectionCallback(newConnection);
  acceptor.listen();

  loop.loop();

  return 0;
}
