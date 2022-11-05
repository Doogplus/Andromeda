//
// Created by cleon on 22-11-5.
//

#ifndef ANDROMEDA_MUDUO_RECIPES_EVENTLOOPTHREADPOOL_H_
#define ANDROMEDA_MUDUO_RECIPES_EVENTLOOPTHREADPOOL_H_

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace muduo {
namespace recipes {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable {
 public:
  using ThreadInitCallback = boost::function<void(EventLoop*)>;

  EventLoopThreadPool(EventLoop* baseLoop);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());
  EventLoop* getNextLoop();

 private:
  EventLoop* baseLoop_; // 与Acceptor所属EventLoop相同
  bool started_;
  int numThreads_;
  int next_; // 新连接到来，所选择的EventLoop对象下标
  boost::ptr_vector<EventLoopThread> threads_; //IO线程列表
  std::vector<EventLoop*> loops_; // EventLoop列表
};

}
}

#endif //ANDROMEDA_MUDUO_RECIPES_EVENTLOOPTHREADPOOL_H_
