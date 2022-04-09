#pragma once

#include <vector>
#include <optional>
#include <cstdint>
#include <functional>

#include <unistd.h>

#include <boost/noncopyable.hpp>

#include "Definition.h"

class MessageHeader : boost::noncopyable
{
public:
    MessageHeader(int32_t hMaxLen);
    MessageHeader(std::vector<char> &&extra, int32_t hMaxLen);
    int32_t read(int fd);
    [[nodiscard]] std::optional<HeaderRet> tryGetHeader(const IFCompleteHeader &judge);
    void swap(MessageHeader &rhs);

private:
    bool _valid;
    std::vector<char> _buf;
    int32_t _nread;
};

class MessageBuf : boost::noncopyable
{
public:
    MessageBuf() = default;
    MessageBuf(int32_t msgSize, std::vector<char> &&content);
    int32_t read(int fd);
    [[nodiscard]] std::optional<std::vector<char>> tryGetMessage();
    void swap(MessageBuf &rhs);

private:
    bool _valid;
    std::vector<char> _buf;
    size_t _nread;
};

class RecvHander : boost::noncopyable
{
public:
    enum class RecvState
    {
        recvHeader,
        recvMsg,
    };
    RecvHander(int hMaxLen, const IFCompleteHeader &judge);
    int32_t read(int fd, const TCPConnectionPtr &ptr, const TCPMessageCB &msgCB);

private:
    void recvHeader(const TCPConnectionPtr &ptr, const TCPMessageCB &msgCB);
    void recvMsg(const TCPConnectionPtr &ptr, const TCPMessageCB &msg);
    const int _hMaxLen;
    RecvState _state;
    const IFCompleteHeader &_judge;
    MessageHeader _curHeader;
    MessageBuf _curContent;
    std::vector<char> _rawHeaderTmp;
};