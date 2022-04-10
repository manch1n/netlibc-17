#pragma once
#include <sys/signal.h>

#include "TCPConnection.h"

inline const TCPConnectedCB DefaultConnectedCB = [](const TCPConnectionPtr &conn)
{
    log_warn("not set connected CB id: %ld", conn->getIndentifier());
};
inline const TCPMessageCB DefaultMessageCB = [](const TCPConnectionPtr &conn, std::vector<char> &&message)
{
    log_warn("not set msg cb id: %ld messageSize", conn->getIndentifier(), message.size());
};
inline const TCPClosingCB DefaultClosingCB = [](const TCPConnectionPtr &conn)
{
    log_warn("not set closing cb id: %ld", conn->getIndentifier());
};
inline const TCPErrorCB DefaultErrorCB = [](const TCPConnectionPtr &conn)
{
    log_warn("not set error cb id: %ld", conn->getIndentifier());
};

struct TCPConnConf
{
public:
    explicit TCPConnConf(const IFCompleteHeader &ifheader, int32_t hmax = 256) : msgCB(DefaultMessageCB), errCB(DefaultErrorCB), connCB(DefaultConnectedCB), cloCB(DefaultClosingCB), hMaxLen(hmax), headerJudge(ifheader) {}
    TCPMessageCB msgCB;
    TCPErrorCB errCB;
    TCPConnectedCB connCB;
    TCPClosingCB cloCB;
    int32_t hMaxLen;
    IFCompleteHeader headerJudge;
};

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

inline IgnoreSigPipe g_sigpipe;