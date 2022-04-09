#include "TCPConnection.h"
#include "TCPConnConf.h"

TCPConnection::TCPConnection(EventLoop *loop, int connfd, uint64_t identifier, const TCPConnConf &conf)
    : _loop(loop),
      _connfd(connfd),
      _identifier(identifier),
      _chan(loop, connfd),
      _conf(conf),
      _recvHandler(_conf.hMaxLen, _conf.headerJudge)
{
    _chan.setReadCB(std::bind(&TCPConnection::handleRead, this));
    _loop->addChannel(&_chan);
    _chan.enableReading();
}

uint64_t TCPConnection::getIndentifier() const
{
    return _identifier;
}

void TCPConnection::handleRead()
{
    int32_t nread = _recvHandler.read(_connfd, shared_from_this(), _conf.msgCB);
    if (nread > 0)
    {
    }
    else if (nread == 0)
    {
        _conf.cloCB(shared_from_this()); // let server to release ptr
    }
    else if (nread == -1)
    {
        assert(errno != EWOULDBLOCK && errno != EAGAIN);
        _conf.errCB(shared_from_this());
    }
}
