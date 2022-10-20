//
// Created by cleon on 22-10-14.
//

#include "muduo/recipes/Channel.h"
#include "muduo/recipes/EventLoop.h"

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/timerfd.h>

using namespace muduo;
using namespace muduo::recipes;

EventLoop* g_loop;
int timerfd;

void timeout(Timestamp receiveTime) {
    printf("Timeout\n");
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    (void)n;
    g_loop->quit();
}

int main() {
    EventLoop loop;
    g_loop = &loop;

    timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback(boost::bind(timeout, _1));
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 1;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    loop.loop();

    ::close(timerfd);
    return 0;
}