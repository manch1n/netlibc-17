#include "Acceptor.h"

#include "SocketWrapper.h"
#include "Logger.h"

Acceptor::Acceptor(EventLoop *loop, const IPv4Address &addr)
    : _owner(loop),
      _listenFd(bindListenOrDie(addr)),
      _listenChan(loop, _listenFd),
      _newConnCB(std::bind(&Acceptor::defaultAcceptCB, this, std::placeholders::_1))
{
    _listenChan.setReadCB(std::bind(&Acceptor::readCB, this));
    loop->addChannel(&_listenChan);
}

void Acceptor::setAcceptCB(const NewConnCB &cb)
{
    _newConnCB = cb;
}

Acceptor::~Acceptor()
{
    _owner->delChannel(&_listenChan);
}

void Acceptor::defaultAcceptCB(int fd)
{
    auto addr = getAddessByFd(fd);
    log_info("default acceptCB:new connection from: %s .closed", addr.IPPortstr().c_str());
    ::close(fd);
}

void Acceptor::readCB()
{
    int connfd = -1;
    while ((connfd = acceptClient(_listenFd)) > 0)
    {
        _newConnCB(connfd);
    }
    assert(errno == EWOULDBLOCK || errno == EAGAIN);
}
