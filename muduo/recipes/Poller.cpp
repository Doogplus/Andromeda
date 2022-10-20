//
// Created by cleon on 22-10-15.
//

#include "muduo/recipes/Poller.h"

using namespace muduo;
using namespace muduo::recipes;

Poller::Poller(EventLoop* loop)
        : ownerLoop_(loop) {
}

Poller::~Poller() {
}