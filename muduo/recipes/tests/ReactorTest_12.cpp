//
// Created by cleon on 22-10-20.
//

#include "muduo/recipes/EventLoop.h"
#include "muduo/recipes/TcpServer.h"
#include "muduo/recipes/InetAddress.h"

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::recipes;

class TestServer {
 public:
  TestServer(EventLoop* loop,
             const InetAddress& listenAddr)
      : loop_(loop),
        server_(loop, listenAddr, "TestServer") {
    server_.setConnectionCallback(
        boost::bind(&TestServer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&TestServer::onMessage, this, _1, _2, _3));
    msg1.resize(100);
    msg2.resize(100);
    std::fill(msg1.begin(), msg1.end(), 'A');
    std::fill(msg2.begin(), msg2.end(), 'B');
  }

  void start() {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
      printf("onConnection(): new connection [%s] from %s\n",
             conn->name().c_str(),
             conn->peerAddress().toIpPort().c_str());
      conn->send(msg1);
      conn->send(msg2);
      conn->shutdown();
    } else {
      printf("onConnection(): connection [%s] is down\n",
             conn->name().c_str());
    }
  }

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime) {
    string msg(buf->retrieveAllAsString());
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           msg.size(), conn->name().c_str(),
           receiveTime.toFormattedString().c_str());
    conn->send(msg);
  }

  EventLoop* loop_;
  TcpServer server_;

  muduo::string msg1;
  muduo::string msg2;
};

int main() {
  printf("main(): pid = %d\n", getpid());

  InetAddress listenAddr(8888);
  EventLoop loop;

  TestServer server(&loop, listenAddr);
  server.start();

  loop.loop();
}