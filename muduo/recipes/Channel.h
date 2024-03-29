//
// Created by cleon on 22-10-15.
//

#ifndef ANDROMEDA_CHANNEL_H
#define ANDROMEDA_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "muduo/base/Timestamp.h"

namespace muduo {
namespace recipes {

class EventLoop;

/**
 * @brief A selectable I/O channel
 *
 * This class doesn't own the file descriptor.
 * The file descriptor could be a socket,
 *  an eventfd, a timerfd, or a signalfd
 */
class Channel : boost::noncopyable {
public:
    using EventCallback = boost::function<void()>;
    using ReadEventCallback = boost::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(const ReadEventCallback& cb) {
        readCallback_ = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        writeCallback_ = cb;
    }
    void setCloseCallback(const EventCallback& cb) {
        closeCallback_ = cb;
    }
    void setErrorCallback(const EventCallback& cb) {
        errorCallback_ = cb;
    }

    /**
     * @brief Tie this channel to the owner object managed by shared_ptr,
     * prevent the owner object being destroyed in handleEvent.
     */
    void tie(const boost::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; } // used by pollers

    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ |= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    bool isWriting() const { return events_ & kWriteEvent; }

    // for Poller
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    string reventsToString() const;
    void doNotLogHup() { logHup_ = false; }

    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;  // 所属的EventLoop
    const int fd_; // 文件描述符，但部负责关闭该文件描述符
    int events_; // 关注的事件
    int revents_; // poll/epoll返回的事件
    int index_; // Used by Poller,表示poll的事件数组中的序号
    bool logHup_;  // for POLLHUP

    boost::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_; // 是否处于事件处理中
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

}
}

#endif//ANDROMEDA_CHANNEL_H
