//
// Created by cleon on 22-10-17.
//

#ifndef ANDROMEDA_TIMER_H
#define ANDROMEDA_TIMER_H

#include <boost/noncopyable.hpp>
#include <utility>

#include "muduo/base/Atomic.h"
#include "muduo/base/Timestamp.h"
#include "muduo/recipes/Callbacks.h"

namespace muduo {
namespace recipes {

class Timer : boost::noncopyable {
public:
    Timer(muduo::recipes::TimerCallback cb, muduo::Timestamp when, double interval)
            : callback_(std::move(cb))
              , expiration_(when)
              , interval_(interval)
              , repeat_(interval > 0.0)
              , sequence_(s_numCreated_.incrementAndGet()) {
    }

    void run() const { callback_(); }

    muduo::Timestamp expiration() const { return expiration_; }

    bool repeat() const { return repeat_; }

    int64_t sequence() const { return sequence_; }

    void restart(muduo::Timestamp now);

    static int64_t numCreated() { return s_numCreated_.get(); }

private:
    const muduo::recipes::TimerCallback callback_; // 定时器回调函数
    muduo::Timestamp expiration_;  // 下一次超时时刻
    const double interval_;  // 超时时间间隔，如果是一次性，该值为0
    const bool repeat_; // 是否重复
    const int64_t sequence_; // 定时器序号

    static muduo::AtomicInt64 s_numCreated_; // 定时器计数，当前已经创建的定时器数量
};

}
}
#endif //ANDROMEDA_TIMER_H
