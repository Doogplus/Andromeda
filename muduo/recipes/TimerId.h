//
// Created by cleon on 22-10-17.
//

#ifndef ANDROMEDA_TIMERID_H
#define ANDROMEDA_TIMERID_H

#include "muduo/base/copyable.h"

namespace muduo {
namespace recipes {

class Timer;

class TimerId : public muduo::copyable {
public:
    TimerId()
        : timer_(nullptr)
        , sequence_(0) {
    }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer)
        , sequence_(seq) {
    }

    friend class TimerQueue;

private:
    Timer* timer_;
    int64_t sequence_;
};

}
}

#endif //ANDROMEDA_TIMERID_H
