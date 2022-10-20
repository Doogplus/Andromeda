//
// Created by cleon on 22-10-17.
//

#ifndef ANDROMEDA_TIMERQUEUE_H
#define ANDROMEDA_TIMERQUEUE_H

#include <set>
#include <vector>

#include <boost/noncopyable.hpp>

#include "muduo/base/Mutex.h"
#include "muduo/base/Timestamp.h"
#include "muduo/recipes/Callbacks.h"
#include "muduo/recipes/Channel.h"

namespace muduo {
namespace recipes {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : boost::noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback& cb,
                     Timestamp when,
                     double interval);

    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;

    TimerList timers_; // 按到期时间排序

    ActiveTimerSet activeTimers_; // 按对象地址排序
    bool callingExpiredTimers_; // atomic
    ActiveTimerSet cancelingTimers_; // 保存的是被取消的定时器
};

}
}

#endif //ANDROMEDA_TIMERQUEUE_H
