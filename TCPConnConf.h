#pragma once
#include "TCPConnection.h"

inline const TCPConnectedCB DefaultConnectedCB = [](const TCPConnectionPtr &conn)
{
    log_warn("not set connected CB id: %ld", conn->getIndentifier());
};
inline const TCPMessageCB DefaultMessageCB = [](const TCPConnectionPtr &conn, std::vector<char> &&header, std::vector<char> &&content)
{
    log_warn("not set msg cb id: %ld headersize: %d contentsize: %d", conn->getIndentifier(), header.size(), content.size());
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
    explicit TCPConnConf(int32_t hmax, const IFCompleteHeader &ifheader) : msgCB(DefaultMessageCB), errCB(DefaultErrorCB), connCB(DefaultConnectedCB), cloCB(DefaultClosingCB), hMaxLen(hmax), headerJudge(ifheader) {}
    TCPMessageCB msgCB;
    TCPErrorCB errCB;
    TCPConnectedCB connCB;
    TCPClosingCB cloCB;
    int32_t hMaxLen;
    IFCompleteHeader headerJudge;
};