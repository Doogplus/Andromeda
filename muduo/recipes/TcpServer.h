//
// Created by cleon on 22-10-23.
//

#ifndef ANDROMEDA_MUDUO_RECIPES_TCPSERVER_H_
#define ANDROMEDA_MUDUO_RECIPES_TCPSERVER_H_

#include "muduo/base/Types.h"
#include "muduo/recipes/TcpConnection.h"

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace muduo {
namespace recipes {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class TcpServer : boost::noncopyable
{
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCallback;

  //TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
  ~TcpServer();  // force out-line dtor, for scoped_ptr members.

  const string& hostport() const { return hostport_; }
  const string& name() const { return name_; }

  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb);

  /// Starts the server if it's not listening.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  // 设置连接到来或者连接关闭回调函数
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  // 设置消息到来回调函数
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const InetAddress& peerAddr);
  /// Thread safe.
  void removeConnection(const TcpConnectionPtr& conn);
  // Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  typedef std::map<string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;  // the acceptor loop
  const string hostport_;		// 服务端口
  const string name_;			// 服务名
  boost::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
  boost::scoped_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  ThreadInitCallback threadInitCallback_;
  bool started_;
  // always in loop thread
  int nextConnId_;				// 下一个连接ID
  ConnectionMap connections_;	// 连接列表
};

}
}

#endif //ANDROMEDA_MUDUO_RECIPES_TCPSERVER_H_
