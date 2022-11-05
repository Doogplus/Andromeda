//
// Created by cleon on 22-10-21.
//

#ifndef ANDROMEDA_INETADDRESS_H
#define ANDROMEDA_INETADDRESS_H

#include "muduo/base/copyable.h"
#include "muduo/base/StringPiece.h"
#include <netinet/in.h>

namespace muduo {
namespace recipes {

class InetAddress : public muduo::copyable {
public:
    explicit InetAddress(uint16_t port);
    InetAddress(const StringPiece& ip, uint16_t port);
    InetAddress(const struct sockaddr_in& addr)
            : addr_(addr) { }

    string toIp() const;
    string toIpPort() const;

    const struct sockaddr_in& getSockAddrInet() const { return addr_; }
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

    uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }
    uint16_t portNetEndian() const { return addr_.sin_port; }

private:
    struct sockaddr_in addr_;
};

}
}

#endif //ANDROMEDA_INETADDRESS_H
