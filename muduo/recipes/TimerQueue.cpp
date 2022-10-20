//
// Created by cleon on 22-10-17.
//

#define __STDC_LIMIT_MACROS
#include "muduo/recipes/TimerQueue.h"
#include "muduo/base/Logging.h"
#include "muduo/recipes/EventLoop.h"
#include "muduo/recipes/Timer.h"
#include "muduo/recipes/TimerId.h"

#include <boost/bind.hpp>
#include <sys/timerfd.h>

namespace muduo {
namespace recipes {

namespace detail {
int createTimerFd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100) { microseconds = 100; }
    struct timespec ts{};
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany) {
        LOG_ERROR << "TimerQueue::handleRead reads " << n << " bytes instead of 8";
    }
}

void resetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec new_value{};
    struct itimerspec old_value{};
    bzero(&new_value, sizeof new_value);
    bzero(&old_value, sizeof old_value);
    new_value.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret < 0) {
        LOG_ERROR << "timerfd_settime()";
    }
}

}  // namespace detail

using namespace muduo;
using namespace muduo::recipes;
using namespace muduo::recipes::detail;

TimerQueue::TimerQueue(muduo::recipes::EventLoop* loop)
    : loop_(loop)
    , timerfd_(createTimerFd())
    , timerfdChannel_(loop, timerfd_)
    , timers_()
    , callingExpiredTimers_(false) {
    timerfdChannel_.setReadCallback(boost::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
    for (const auto & timer : timers_) {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(const muduo::recipes::TimerCallback& cb, muduo::Timestamp when, double interval) {
    auto* timer = new Timer(cb, when, interval);
    // addTimerInLoop(timer);
    loop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop, this, timer));
    return {timer, timer->sequence()};
}

void TimerQueue::cancel(muduo::recipes::TimerId timerId) {
    loop_->runInLoop(boost::bind(&TimerQueue::cancelInLoop, this, timerId));
    // cancelInLoop(timerId);
}

void TimerQueue::addTimerInLoop(muduo::recipes::Timer* timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(muduo::recipes::TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());

    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    auto it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        (void)n;
        delete it->first;
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {
        // 已经到期，并且正在调用回调函数的定时器
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now); // 清除该事件，避免一直触发
    std::vector<Entry> expired = getExpired(now);
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (auto& ex : expired) {
        ex.second->run();
    }
    callingExpiredTimers_ = false;

    // 不是一次性定时器，需要重启
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(muduo::Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (auto it = expired.begin(); it != expired.end(); ++it) {
        ActiveTimer timer(it->second, it->second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void)n;
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, muduo::Timestamp now) {
    Timestamp next_expired;
    for (auto it = expired.begin(); it != expired.end(); ++it) {
        ActiveTimer timer(it->second, it->second->sequence());
        if (it->second->repeat() &&
            cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it->second->restart(now);
            insert(it->second);
        } else {
            delete it->second;
        }
    }

    if (!timers_.empty()) {
        next_expired = timers_.begin()->second->expiration();
    }

    if (next_expired.valid()) {
        resetTimerfd(timerfd_, next_expired);
    }
}

bool TimerQueue::insert(muduo::recipes::Timer* timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());

    // 最早到期时间是否改变
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto it = timers_.begin();
    // 如果timers为空或者when小于timers中的最早到期时间
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
        assert(result.second);
        (void)result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result =
                activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
        (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

}
}
