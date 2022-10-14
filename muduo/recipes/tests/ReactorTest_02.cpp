//
// Created by cleon on 22-10-14.
//

#include "muduo/recipes/EventLoop.h"
#include <stdio.h>

using namespace muduo;
using namespace muduo::recipes;

EventLoop* g_loop;

void threadFunc() {
    g_loop->loop();
}

int main() {
    printf("main(): pid=%d, tid=%d\n", getpid(), CurrentThread::tid());
    EventLoop loop;
    g_loop = &loop;
    Thread t(threadFunc);
    t.start();
    t.join();

    return 0;
}