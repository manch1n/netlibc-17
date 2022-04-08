#pragma once

#include <atomic>
#include <map>

#include "Acceptor.h"
#include "TCPConnection.h"

inline const TCPConnectedCB DefaultConnectedCB = [](const TCPConnectionPtr &conn)
{
    log_warn("not set connected CB id: %ld", conn->getIndentifier());
};
inline const TCPMessageCB DefaultMessageCB = [](const TCPConnectionPtr &conn, std::vector<char> &&buf)
{
    log_warn("not set msg cb id: %ld msgsize: %d", conn->getIndentifier(), buf.size());
};
inline const TCPClosingCB DefaultClosingCB = [](const TCPConnectionPtr &conn)
{
    log_warn("not set closing cb id: %ld", conn->getIndentifier());
};
inline const TCPErrorCB DefaultErrorCB = [](const TCPConnectionPtr &conn)
{
    log_warn("not set error cb id: %ld", conn->getIndentifier());
};

class TCPServer : boost::noncopyable
{
public:
    TCPServer(EventLoop *loop, const IPv4Address &addr, const TCPMessageCB &msgcb = DefaultMessageCB, const TCPErrorCB &errcb = DefaultErrorCB, const TCPConnectedCB &conncb = DefaultConnectedCB, const TCPClosingCB &clocb = DefaultClosingCB);

private:
    void newConn(int fd);
    void closeConn(const TCPConnectionPtr &ptr);
    EventLoop *_loop;
    std::atomic<uint64_t> _curIdentifier;
    std::unique_ptr<Acceptor> _acceptor;
    std::map<uint64_t, TCPConnectionPtr> _saveConn;
    const TCPMessageCB _msgCB;
    const TCPErrorCB _errCB;
    const TCPConnectedCB _connCB;
    const TCPClosingCB _cloCB;
};