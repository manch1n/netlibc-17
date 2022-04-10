#pragma once

#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>

#include "Definition.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Logger.h"
#include "Message.h"

class TCPConnConf;
class TCPConnection : boost::noncopyable,
                      public std::enable_shared_from_this<TCPConnection>
{
public:
    TCPConnection(EventLoop *loop, int connfd, uint64_t identifier, const TCPConnConf &conf);

    uint64_t getIndentifier() const;

    // threadsafe data then would unvailable
    void send(std::vector<char> &&data);
    void shutDown();
    void setNoDelay();
    void keepAlive(int idle, int interval, int tryCount);
    bool readEOF() const;

    ~TCPConnection() // no reference to this ptr
    {
        _loop->delChannel(&_chan);
        ::close(_connfd);
    }

private:
    void handleRead();
    void handleWrite();
    void sendInLoop(std::vector<char> &data);
    void shutDownInLoop();
    bool writeCompleted() const;
    bool readCompleted() const;
    EventLoop *_loop;
    int _connfd;
    const uint64_t _identifier;
    Channel _chan;
    const TCPConnConf &_conf;
    RecvHander _recvHandler;
    std::deque<std::vector<char>> _outputBuffers;
    bool _errFlag;
    bool _writeEofFlag;
    bool _readEofFlag;
};
