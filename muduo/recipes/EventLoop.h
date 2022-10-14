//
// Created by cleon on 22-10-14.
//

#ifndef ANDROMEDA_EVENTLOOP_H
#define ANDROMEDA_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Thread.h"

namespace muduo {
namespace recipes {
//
//
class EventLoop : boost::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();

    bool looping_;
    const pid_t threadId_;
};

}  // namespace recipes
}  // namespace muduo

#endif//ANDROMEDA_EVENTLOOP_H
