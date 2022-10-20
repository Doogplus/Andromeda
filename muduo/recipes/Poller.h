//
// Created by cleon on 22-10-15.
//

#ifndef ANDROMEDA_POLLER_H
#define ANDROMEDA_POLLER_H

#include <vector>
#include <boost/noncopyable.hpp>

#include "muduo/base/Timestamp.h"
#include "muduo/recipes/EventLoop.h"

namespace muduo {
namespace recipes {

class Channel;

/**
 * Base class for IO Multiplexing
 * This class doesn't own the Channel objects.
 */
class Poller : boost::noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    explicit Poller(EventLoop* loop);
    virtual ~Poller();

    // Polls the I/O  events/
    // Must be called in the loop thread
    virtual Timestamp poll(int timeoutMs, ChannelList* activateChannels) = 0;

    // Changes the interested I/O events
    // Must be called in the loop thread.
    virtual void updateChannel(Channel* channel) = 0;

    virtual void removeChannel(Channel* channel) = 0;

    static Poller* newDefaultPoller(EventLoop* loop);

    void assertInLoopThread() {
        ownerLoop_->assertInLoopThread();
    }

private:
    EventLoop* ownerLoop_; // Poller所属EventLoop
};

}
}

#endif//ANDROMEDA_POLLER_H
