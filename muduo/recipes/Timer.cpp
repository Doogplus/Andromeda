//
// Created by cleon on 22-10-17.
//

#include "muduo/recipes/Timer.h"

using namespace muduo;
using namespace muduo::recipes;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(muduo::Timestamp now) {
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}