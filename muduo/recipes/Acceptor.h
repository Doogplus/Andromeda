//
// Created by cleon on 22-10-22.
//

#ifndef ANDROMEDA_ACCEPTOR_H
#define ANDROMEDA_ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "muduo/recipes/Channel.h"
#include "muduo/recipes/Socket.h"

namespace muduo {
namespace recipes {

class EventLoop;
class InetAddress;

/**
 * @brief Acceptor of incoming connections.
 */
class Acceptor : boost::noncopyable {
 public:
  using NewConnectionCallback = boost::function<void(int sockfd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }
  bool listening() const { return listening_; }
  void listen();

 private:
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listening_;
  int idleFd_;
};

}
}

#endif //ANDROMEDA_ACCEPTOR_H
