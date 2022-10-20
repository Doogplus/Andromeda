//
// Created by cleon on 22-10-15.
//

#include "muduo/recipes/poller/PollPoller.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Types.h"
#include "muduo/recipes/Channel.h"

#include <assert.h>
#include <poll.h>

using namespace muduo;
using namespace muduo::recipes;

PollPoller::PollPoller(muduo::recipes::EventLoop* loop)
    : Poller(loop) {
}

PollPoller::~PollPoller() {
}

Timestamp PollPoller::poll(int timeoutMs, muduo::recipes::Poller::ChannelList* activateChannels) {
    int num_events = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());

    if (num_events > 0) {
        LOG_TRACE << num_events << " events happened";
        fillActivateChannels(num_events, activateChannels);
    } else if (num_events == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        LOG_SYSERR << "PollPoller::poll()";
    }
    return now;
}

void PollPoller::fillActivateChannels(int num_events, muduo::recipes::Poller::ChannelList* activateChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end(); ++pfd) {
        if (pfd->revents > 0) {
            --num_events;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activateChannels->push_back(channel);
        }
    }
}
void PollPoller::updateChannel(muduo::recipes::Channel* channel) {
    Poller::assertInLoopThread();
    LOG_TRACE << "fd=" << channel->fd() << " events = " << channel->events();

    if (channel->index() < 0) {  // 新的通道
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size())-1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    } else {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert( 0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[static_cast<size_t>(idx)];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;

        if (channel->isNoneEvent()) {
            // ignore this pollfd
            pfd.fd = - channel->fd()-1;
        }
    }
}


void PollPoller::removeChannel(muduo::recipes::Channel* channel) {
    Poller::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();

    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[static_cast<size_t>(idx)]; (void)pfd;
    assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1); (void)n;
    if (implicit_cast<size_t>(idx) == pollfds_.size()-1) {
        pollfds_.pop_back();
    } else {
        int channel_fd_at_end = pollfds_.back().fd;
        std::iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
        if (channel_fd_at_end < 0) {
            channel_fd_at_end = -channel_fd_at_end-1;
        }
        channels_[channel_fd_at_end]->set_index(idx);
        pollfds_.pop_back();
    }
}