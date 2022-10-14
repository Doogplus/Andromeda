//
// Created by cleon on 22-10-14.
//

#include "muduo/recipes/EventLoop.h"
#include <stdio.h>

using namespace muduo;
using namespace muduo::recipes;

void threadFunc() {
    printf("threadFunc(): pid=%d, tid=%d\n", getpid(), CurrentThread::tid());
    EventLoop loop;
    loop.loop();
}

int main() {
    printf("main(): pid=%d, tid=%d\n", getpid(), CurrentThread::tid());
    EventLoop loop;
    Thread t(threadFunc);
    t.start();

    loop.loop();
    t.join();

    return 0;
}