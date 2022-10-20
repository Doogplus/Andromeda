//
// Created by cleon on 22-10-14.
//

#ifndef ANDROMEDA_EVENTLOOP_H
#define ANDROMEDA_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Timestamp.h"
#include "muduo/recipes/Callbacks.h"
#include "muduo/recipes/TimerId.h"

#include <vector>

namespace muduo {
namespace recipes {

class Channel;
class Poller;
class TimerQueue;
/**
 * Reactor, at most one per thread
 * This is an interface class, so don't expose too much details
 */
class EventLoop : boost::noncopyable {
public:
    using Functor = boost::function<void()>;

    EventLoop();
    ~EventLoop();  // force out-line dtor, for scoped member

    /**
     * Loops forever.
     * Must be called in the same thread as certain of the object.
     */
    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }
    /**
     * @brief Run callbacks immediately in the loop thread.
     *  It wakes up the loop, and run the cb.
     *  If in the same loop thread, cb is run within the function.
     *  Safe to call from other threads.
     */
    void runInLoop(const Functor& cb);
    /**
     * @brief Queues callbacks in the loop thread.
     * Runs after finish polling.
     * Safe to call from other threads.
     * @param cb
     */
    void queueInLoop(const Functor& cb);

    TimerId runAt(const Timestamp& time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);
    void cancel(TimerId timerId);

    void wakeup() const;
    void updateChannel(Channel* channel); // 在Poller中添加或者更新通道
    void removeChannel(Channel* channel); // 从Poller中移除通道

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();
    void handleRead() const;
    void doPendingFunctors();
    void printActivateChannels() const;

    using ChanelList = std::vector<Channel*>;

    bool looping_;  // atomic
    bool quit_;  // atomic
    bool eventHandling_; // atomic
    bool callingPendingFunctors_; // atomic
    const pid_t threadId_;  // 当前对象所属线程id
    Timestamp pollReturnTime_;
    boost::scoped_ptr<Poller> poller_;
    boost::scoped_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;  // 用于eventfd
    boost::scoped_ptr<Channel> wakeupChannel_; // 该通道会纳入poller_来管理
    ChanelList activateChannels_; // Poller返回的活动通道
    Channel* currentActivateChannel_; // 当前正在处理的活动通道
    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;
};

}  // namespace recipes
}  // namespace muduo

#endif//ANDROMEDA_EVENTLOOP_H
