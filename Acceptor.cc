#include "Acceptor.h"

#include "SocketWrapper.h"

Acceptor::Acceptor(EventLoop *loop, const IPv4Address &addr, const NewConnCB &cb)
    : _owner(loop),
      _listenFd(bindListenOrDie(addr)),
      _listenChan(loop, _listenFd),
      _newConnCB(cb)
{
    _listenChan.setReadCB(std::bind(&Acceptor::readCB, this));
    loop->addChannel(&_listenChan);
}

void Acceptor::startAccept()
{
    _listenChan.enableReading();
}

void Acceptor::setAcceptCB(const NewConnCB &cb)
{
    _newConnCB = cb;
}

Acceptor::~Acceptor()
{
    _owner->delChannel(&_listenChan);
    ::close(_listenFd);
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
