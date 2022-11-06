//
// Created by cleon on 22-10-17.
//

#ifndef ANDROMEDA_CALLBACKS_H
#define ANDROMEDA_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "muduo/base/Timestamp.h"

namespace muduo {
namespace recipes {

// All client visible callbacks go here.

class Buffer;

class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef boost::function<void()> TimerCallback;

typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
//typedef boost::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
//typedef boost::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
//
//// the data has been read to (buf, len)
typedef boost::function<void (const TcpConnectionPtr&,
                              Buffer*,
                              Timestamp)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp receiveTime);

//typedef boost::function<void (const TcpConnectionPtr&,
//                              const char* data,
//                              ssize_t len)> MessageCallback;

}
}

#endif //ANDROMEDA_CALLBACKS_H
