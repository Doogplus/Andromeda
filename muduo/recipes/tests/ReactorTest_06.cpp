//
// Created by cleon on 22-10-20.
//

#include "muduo/recipes/EventLoop.h"
#include "muduo/recipes/EventLoopThread.h"

#include <stdio.h>

using namespace muduo;
using namespace muduo::recipes;

void runInThread() {
  printf("runInThread(): pid = %d, tid = %d\n",
         getpid(), CurrentThread::tid());
}

int main() {
  printf("main(): pid = %d, tid = %d\n",
         getpid(), CurrentThread::tid());

  EventLoopThread loop_thread;
  EventLoop* loop = loop_thread.startLoop();
  // 异步调用runInThread，即将runInThread添加到loop对象所在IO线程，让该IO线程执行
  loop->runInLoop(runInThread);
  sleep(1);
  // runAfter内部也调用了runInLoop，所以这里也是异步调用
  loop->runAfter(2, runInThread);

  sleep(3);
  loop->quit();
  printf("exit main().\n");
}