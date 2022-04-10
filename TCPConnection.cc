#include "TCPConnection.h"

#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "TCPConnConf.h"

TCPConnection::TCPConnection(EventLoop *loop, int connfd, uint64_t identifier, const TCPConnConf &conf)
    : _loop(loop),
      _connfd(connfd),
      _identifier(identifier),
      _chan(loop, connfd),
      _conf(conf),
      _recvHandler(_conf.hMaxLen, _conf.headerJudge),
      _errFlag(false),
      _writeEofFlag(false),
      _readEofFlag(false)
{
    _chan.setReadCB(std::bind(&TCPConnection::handleRead, this));
    _chan.setWriteCB(std::bind(&TCPConnection::handleWrite, this));
    _loop->addChannel(&_chan);
    _chan.enableReading();
}

uint64_t TCPConnection::getIndentifier() const
{
    return _identifier;
}

void TCPConnection::send(std::vector<char> &&data)
{
    if (data.size() == 0 || _errFlag || _writeEofFlag)
        return;
    if (_loop->ifRunInLoopThread())
    {
        sendInLoop(data);
    }
    else
    {
        _loop->runInLoop(std::bind(&TCPConnection::sendInLoop, this, std::ref(data)));
    }
}

void TCPConnection::shutDown()
{
    _loop->runInLoop(std::bind(&TCPConnection::shutDownInLoop, this));
}

void TCPConnection::setNoDelay()
{
    int on = 1;
    ::setsockopt(_connfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}

void TCPConnection::keepAlive(int idle, int interval, int tryCount)
{
    int on = 1;
    ::setsockopt(_connfd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    ::setsockopt(_connfd, SOL_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
    ::setsockopt(_connfd, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    ::setsockopt(_connfd, SOL_TCP, TCP_KEEPCNT, &tryCount, sizeof(tryCount));
}

bool TCPConnection::readEOF() const
{
    return _readEofFlag;
}

void TCPConnection::handleRead()
{
    if (_errFlag)
        return;
    RecvHander::RecvState state = _recvHandler.read(_connfd, shared_from_this(), _conf.msgCB);
    if (state == RecvHander::RecvState::RecvNormal)
    {
    }
    else if (state == RecvHander::RecvState::RecvEOF)
    {
        _readEofFlag = true;
        _chan.disableReading();
        _conf.msgCB(shared_from_this(), std::vector<char>{});
        if (readCompleted() && writeCompleted())
        {
            _conf.cloCB(shared_from_this()); // let server to release ptr
        }
    }
    else if (state == RecvHander::RecvState::RecvErr)
    {
        assert(errno != EAGAIN);
        _errFlag = true;
        _conf.errCB(shared_from_this());
    }
}

void TCPConnection::handleWrite()
{
    if (_errFlag)
    {
        _chan.disableWriting();
        return;
    }
    std::vector<struct iovec> iovecs(_outputBuffers.size());
    for (int i = 0; i < _outputBuffers.size(); ++i)
    {
        iovecs[i].iov_base = _outputBuffers[i].data();
        iovecs[i].iov_len = _outputBuffers[i].size();
    }
    int32_t nwrite = ::writev(_connfd, iovecs.data(), iovecs.size());
    if (nwrite > 0)
    {
        while (_outputBuffers.empty() == false)
        {
            std::vector<char> &front = _outputBuffers.front();
            if (nwrite >= front.size())
            {
                nwrite -= front.size();
                _outputBuffers.pop_front();
            }
            else
            {
                std::vector<char> tmp(front.begin() + nwrite, front.end());
                nwrite -= front.size();
                front = std::move(tmp);
                break;
            }
        }
        if (nwrite == 0)
        {
            _chan.disableWriting();
        }
        if (writeCompleted())
        {
            ::shutdown(_connfd, SHUT_WR);
        }
        if (readCompleted() && writeCompleted())
        {
            _conf.cloCB(shared_from_this());
        }
    }
    else if (nwrite < 0)
    {
        assert(errno != EAGAIN);
        _errFlag = true;
        _chan.disableWriting();
        _conf.errCB(shared_from_this());
    }
}

void TCPConnection::sendInLoop(std::vector<char> &data)
{
    if (_errFlag)
        return;
    if (_outputBuffers.empty())
    {
        int32_t nwrite = ::write(_connfd, data.data(), data.size());
        if (nwrite > 0)
        {
            if (nwrite < data.size())
            {
                std::vector<char> nowrite(data.begin() + nwrite, data.end());
                _outputBuffers.push_back(std::move(nowrite));
            }
            else
                return;
        }
        else if (nwrite == 0)
        {
            _outputBuffers.push_back(std::move(data));
        }
        else if (nwrite == -1)
        {
            if (errno == EAGAIN)
            {
                _outputBuffers.push_back(std::move(data));
            }
            else
            {
                _errFlag = true;
                _chan.disableWriting();
                _conf.errCB(shared_from_this());
            }
        }
    }
    else
    {
        _outputBuffers.push_back(std::move(data));
    }
    _chan.enableWriting();
}

void TCPConnection::shutDownInLoop()
{
    if (_errFlag || _writeEofFlag)
        return;
    _writeEofFlag = true;
    if (writeCompleted())
    {
        ::shutdown(_connfd, SHUT_WR);
    }
}

bool TCPConnection::writeCompleted() const
{
    return _writeEofFlag && _outputBuffers.empty();
}

bool TCPConnection::readCompleted() const
{
    return _readEofFlag;
}
