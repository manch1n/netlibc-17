#pragma once

#include "EventLoop.h"
#include "IPv4Address.h"
#include "SocketWrapper.h"
#include "Logger.h"

using NewConnCB = std::function<void(int fd)>;

inline const NewConnCB DefaultNewConnCB = [](int fd)
{
    auto addr = getAddressByFd(fd);
    log_warn("default acceptCB:new connection from: %s .closed", addr.IPPortstr().c_str());
    ::close(fd);
};
class Acceptor : boost::noncopyable
{
public:
    Acceptor(EventLoop *loop, const IPv4Address &addr, const NewConnCB &cb = DefaultNewConnCB);
    void startAccept();
    void setAcceptCB(const NewConnCB &cb);
    ~Acceptor();

private:
    void readCB();
    EventLoop *_owner;
    int _listenFd;
    Channel _listenChan;
    NewConnCB _newConnCB;
};