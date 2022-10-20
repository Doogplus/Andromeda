//
// Created by cleon on 22-10-15.
//

#include "muduo/recipes/Poller.h"
#include "muduo/recipes/poller/PollPoller.h"
#include "muduo/recipes/poller/EPollPoller.h"

#include <stdlib.h>

using namespace muduo::recipes;

Poller* Poller::newDefaultPoller(muduo::recipes::EventLoop* loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return new PollPoller(loop);
    }  else {
        return new EpollPoller(loop);
    }
}