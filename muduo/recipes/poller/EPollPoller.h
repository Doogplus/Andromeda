//
// Created by cleon on 22-10-15.
//

#ifndef ANDROMEDA_EPOLLPOLLER_H
#define ANDROMEDA_EPOLLPOLLER_H

#include "muduo/recipes/Poller.h"
#include <map>
#include <vector>

struct epoll_event;

namespace muduo {
namespace recipes {

class EpollPoller : public Poller {
public:
    explicit EpollPoller(EventLoop* loop);
    virtual ~EpollPoller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activateChannels);
    virtual void updateChannel(Channel* channel);
    virtual void removeChannel(Channel* channel);

private:
    static const int kInitEventListSize = 16;

    void fillActivateChannels(int numEvents, ChannelList* activateChannels) const;
    void update(int operation, Channel* channel);

    using EventList = std::vector<struct epoll_event>;
    using ChannelMap = std::map<int, Channel*>;

    int epollfd_;
    EventList events_;
    ChannelMap channels_;
};

}
}

#endif//ANDROMEDA_EPOLLPOLLER_H
