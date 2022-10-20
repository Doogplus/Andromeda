//
// Created by cleon on 22-10-15.
//

#include "muduo/recipes/poller/EPollPoller.h"
#include "muduo/base/Logging.h"
#include "muduo/recipes/Channel.h"

#include <boost/static_assert.hpp>

#include <assert.h>
#include <poll.h>
#include <sys/epoll.h>

using namespace muduo;
using namespace muduo::recipes;

BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EpollPoller::EpollPoller(muduo::recipes::EventLoop* loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPollerPoller::EPollPoller";
    }
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, muduo::recipes::Poller::ChannelList* activateChannels) {
    int num_events = ::epoll_wait(epollfd_,
                                  &*events_.begin(),
                                  static_cast<int>(events_.size()),
                                  timeoutMs);
    Timestamp now(Timestamp::now());
    if (num_events > 0) {
        LOG_TRACE << num_events << " events happened";
        fillActivateChannels(num_events, activateChannels);
        if (implicit_cast<size_t>(num_events) == events_.size()) {
            events_.resize(events_.size()*2);
        }
    } else if (num_events == 0) {
        LOG_TRACE << " nothing happened";
    } else {
        LOG_SYSERR << "EPollPoller::poll()";
    }

    return now;
}

void EpollPoller::fillActivateChannels(int numEvents, muduo::recipes::Poller::ChannelList* activateChannels) const {
    assert(implicit_cast<size_t>(numEvents) <= events_.size());
    for (size_t i = 0; i < static_cast<size_t>(numEvents); ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(static_cast<int>(events_[i].events));
        activateChannels->push_back(channel);
    }
}

void EpollPoller::updateChannel(muduo::recipes::Channel* channel) {
    Poller::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    const int index = channel->index();
    if (index == kNew || index == kDeleted) {
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(muduo::recipes::Channel* channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;

    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kDeleted || index == kAdded);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = static_cast<uint32_t>(channel->events());
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_TRACE << "epoll_ctl op = " << operation << " fd = " << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op =" << operation << " fd = " << fd;
        }
    }
}