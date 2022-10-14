//
// Created by cleon on 22-10-14.
//

#include "muduo/recipes/EventLoop.h"
#include "muduo/base/Logging.h"
#include <poll.h>

using namespace muduo;
using namespace recipes;

namespace {
    __thread EventLoop* t_loopInThisThread = 0;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false)
    , threadId_(CurrentThread::tid()) {
    LOG_TRACE << "EventLoop create " << this << " in thread " << threadId_;
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    LOG_TRACE << "EventLoop " << this << "start looping";
    ::poll(nullptr, 0, 5 * 1000);
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in thread_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
}