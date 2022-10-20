//
// Created by cleon on 22-10-15.
//

#ifndef ANDROMEDA_POLLPOLLER_H
#define ANDROMEDA_POLLPOLLER_H

#include "muduo/recipes/Poller.h"

#include <map>
#include <vector>

struct pollfd;

namespace muduo {
namespace recipes {

class PollPoller : public Poller {
public:
    PollPoller(EventLoop* loop);
    virtual ~PollPoller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activateChannels);
    virtual void updateChannel(Channel* channel);
    virtual void removeChannel(Channel* channel);

private:
    void fillActivateChannels(int numEvents, ChannelList* activateChannels) const;

    using PollFdList = std::vector<pollfd>;
    using ChannelMap = std::map<int, Channel*>;

    PollFdList pollfds_;
    ChannelMap channels_;
};

}
}

#endif//ANDROMEDA_POLLPOLLER_H
