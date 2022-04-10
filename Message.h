#pragma once

#include <vector>
#include <optional>
#include <cstdint>
#include <functional>

#include <unistd.h>

#include <boost/noncopyable.hpp>

#include "Definition.h"

class RecvHander : boost::noncopyable
{
public:
    enum class RecvState : int32_t
    {
        RecvErr = -1,
        RecvEOF = 0,
        RecvNormal = 1,
    };
    RecvHander(int hMaxLen, const IFCompleteHeader &judge);
    RecvState read(int fd, const TCPConnectionPtr &ptr, const TCPMessageCB &msgCB);

private:
    enum class CurrentRecv
    {
        RecvHeader,
        RecvMsg,
    };

    void reset();
    const int _hMaxLen;
    const IFCompleteHeader &_ifCompleteHeader;
    std::vector<char> _message;
    CurrentRecv _state;
    int32_t _nread;
};