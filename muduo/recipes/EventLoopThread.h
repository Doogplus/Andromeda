//
// Created by cleon on 22-10-20.
//

#ifndef ANDROMEDA_EVENTLOOPTHREAD_H
#define ANDROMEDA_EVENTLOOPTHREAD_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace muduo {
namespace recipes {

class EventLoop;

class EventLoopThread : boost::noncopyable {
public:
    using ThreadInitCallback = boost::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop* startLoop(); // start thread, this thread become IO thread

private:
    void threadFunc(); // 线程函数

    EventLoop* loop_; // loop_指向一个EventLoop object
    bool existing_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    ThreadInitCallback callback_;
};

}
}

#endif //ANDROMEDA_EVENTLOOPTHREAD_H
