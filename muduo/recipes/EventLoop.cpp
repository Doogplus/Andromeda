//
// Created by cleon on 22-10-14.
//

#include "muduo/recipes/EventLoop.h"
#include "muduo/base/Logging.h"
#include "muduo/recipes/Channel.h"
#include "muduo/recipes/Poller.h"
#include "muduo/recipes/TimerQueue.h"

using namespace muduo;
using namespace recipes;

namespace {
    // 当前线程EventLoop object pointer
    // thread locally stored
    __thread EventLoop* t_loopInThisThread = nullptr;

    const int kPollTimeMs = 10000;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , eventHandling_(false)
      , threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , timerQueue_(new TimerQueue(this))
    , currentActivateChannel_(nullptr) {
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

// Only be called in thread create the object
// can not call across thread
void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    LOG_TRACE << "EventLoop " << this << "start looping";

    // ::poll(nullptr, 0, 5 * 1000);
    while (!quit_) {
        activateChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activateChannels_);
        if (Logger::logLevel() <= Logger::TRACE) {
            printActivateChannels();
        }
        eventHandling_ = true;
        for (auto & activateChannel : activateChannels_) {
            currentActivateChannel_ = activateChannel;
            currentActivateChannel_->handleEvent(pollReturnTime_);
        }
        currentActivateChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        // wakeup();
    }
}

void EventLoop::runInLoop(const muduo::recipes::EventLoop::Functor& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const muduo::recipes::EventLoop::Functor& cb) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) {
   Timestamp time(addTime(Timestamp::now(), interval));
   return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::cancel(TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(muduo::recipes::Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(muduo::recipes::Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_) {
        assert(currentActivateChannel_ == channel ||
               std::find(activateChannels_.begin(),
                         activateChannels_.end(), channel) == activateChannels_.end());
    }
    poller_->removeChannel(channel);
}

void EventLoop::wakeup() const {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() write " << n << " bytes instead of 8";
    }
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in thread_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::handleRead() const {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (auto & functor : functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActivateChannels() const {
    for (auto& activateChannel : activateChannels_) {
        LOG_TRACE << "{" << activateChannel->reventsToString() << "}";
    }
}
