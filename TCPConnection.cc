#include "TCPConnection.h"

TCPConnection::TCPConnection(EventLoop *loop, int connfd, uint64_t identifier, const TCPMessageCB &mcb, const TCPErrorCB &ecb, const TCPConnectedCB &ccb, const TCPClosingCB &clocb)
    : _loop(loop),
      _connfd(connfd),
      _identifier(identifier),
      _chan(loop, connfd),
      _msgCB(mcb), _errorCB(ecb), _connCB(ccb), _closeCB(clocb)
{
    _chan.setReadCB(std::bind(&TCPConnection::handleRead, this));
    _loop->addChannel(&_chan);
    _chan.enableReading();
}

uint64_t TCPConnection::getIndentifier() const
{
    return _identifier;
}

void TCPConnection::setConnectedCB(const TCPConnectedCB &cb)
{
    _connCB = cb;
}

void TCPConnection::setMessageCB(const TCPMessageCB &cb)
{
    _msgCB = cb;
}

void TCPConnection::setClosingCB(const TCPClosingCB &cb)
{
    _closeCB = cb;
}

void TCPConnection::setErrorCB(const TCPErrorCB &cb)
{
    _errorCB = cb;
}

void TCPConnection::handleRead()
{
    std::vector<char> buf(8192);
    int nread = ::read(_connfd, buf.data(), 8192);
    if (nread > 0)
    {
        buf.resize(nread);
        _msgCB(shared_from_this(), std::move(buf));
    }
    else if (nread == 0)
    {
        _closeCB(shared_from_this()); // let server to release ptr
    }
    else if (nread == -1)
    {
        assert(errno != EWOULDBLOCK && errno != EAGAIN);
        _errorCB(shared_from_this());
    }
}
