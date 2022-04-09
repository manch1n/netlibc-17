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

    void setConnectedCB(const TCPConnectedCB &cb);
    void setMessageCB(const TCPMessageCB &cb);
    void setClosingCB(const TCPClosingCB &cb);
    void setErrorCB(const TCPErrorCB &cb);

    ~TCPConnection() // no reference to this ptr
    {
        _loop->delChannel(&_chan);
        ::close(_connfd);
    }

private:
    void handleRead();
    EventLoop *_loop;
    int _connfd;
    const uint64_t _identifier;
    Channel _chan;
    const TCPConnConf &_conf;
    RecvHander _recvHandler;
};
