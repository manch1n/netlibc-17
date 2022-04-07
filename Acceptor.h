#pragma once

#include "EventLoop.h"
#include "IPv4Address.h"

class Acceptor : boost::noncopyable
{
public:
    using NewConnCB = std::function<void(int fd)>;

    Acceptor(EventLoop *loop, const IPv4Address &addr);
    void setAcceptCB(const NewConnCB &cb);
    ~Acceptor();

private:
    void defaultAcceptCB(int fd);
    void readCB();
    EventLoop *_owner;
    int _listenFd;
    Channel _listenChan;
    NewConnCB _newConnCB;
};