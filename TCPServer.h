#pragma once

#include <atomic>
#include <map>

#include "Acceptor.h"
#include "TCPConnection.h"
#include "TCPConnConf.h"

class TCPServer : boost::noncopyable
{
public:
    TCPServer(EventLoop *loop, const IPv4Address &addr, const TCPConnConf &conf);

private:
    void newConn(int fd);
    void closeConn(const TCPConnectionPtr &ptr);
    EventLoop *_loop;
    std::atomic<uint64_t> _curIdentifier;
    std::unique_ptr<Acceptor> _acceptor;
    std::map<uint64_t, TCPConnectionPtr> _saveConn;
    TCPConnConf _conf;
};