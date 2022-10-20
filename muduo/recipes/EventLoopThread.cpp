//
// Created by cleon on 22-10-20.
//

#include "muduo/recipes/EventLoopThread.h"
#include "muduo/recipes/EventLoop.h"
#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::recipes;

EventLoopThread::EventLoopThread(const muduo::recipes::EventLoopThread::ThreadInitCallback& cb)
    : loop_(nullptr)
    , existing_(false)
    , thread_(boost::bind(&EventLoopThread::threadFunc, this))
    , mutex_()
    , cond_(mutex_)
    , callback_(cb) {
}

EventLoopThread::~EventLoopThread() {
    existing_ = true;
    loop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();

    {
        MutexLockGuard lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait();
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (callback_) {
        callback_(&loop);
    }

    {
        MutexLockGuard lock(mutex_);
        // loop_指针指向了一个栈上的对象，threadFunc函数退出之后，这个指针就失效了
        // threadFunc函数退出，就意味着线程退出了，EventLoopThread对象也就没有存在的价值了。
        // 因而不会有什么大的问题
        loop_ = &loop;
        cond_.notify();
    }
    loop_->loop();
}