#pragma once

#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>

#include "EventLoop.h"
#include "Channel.h"
#include "Logger.h"

class TCPConnection;

using TCPConnectionPtr = std::shared_ptr<TCPConnection>;

using TCPConnectedCB = std::function<void(const TCPConnectionPtr &conn)>;
using TCPMessageCB = std::function<void(const TCPConnectionPtr &conn, std::vector<char> &&buf)>;
using TCPClosingCB = std::function<void(const TCPConnectionPtr &conn)>;
using TCPErrorCB = std::function<void(const TCPConnectionPtr &conn)>;

class TCPConnection : boost::noncopyable,
                      public std::enable_shared_from_this<TCPConnection>
{
public:
    TCPConnection(EventLoop *loop, int connfd, uint64_t identifier, const TCPMessageCB &mcb, const TCPErrorCB &ecbconst, const TCPConnectedCB &ccb, const TCPClosingCB &clocb);

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
    TCPMessageCB _msgCB;
    TCPErrorCB _errorCB;
    TCPConnectedCB _connCB;
    TCPClosingCB _closeCB;
};
