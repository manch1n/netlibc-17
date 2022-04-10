#include "Message.h"

#include <cassert>

#include "Logger.h"

RecvHander::RecvHander(int hMaxLen, const IFCompleteHeader &judge)
    : _hMaxLen(hMaxLen),
      _ifCompleteHeader(judge),
      _message(hMaxLen),
      _state(CurrentRecv::RecvHeader),
      _nread(0)
{
}

RecvHander::RecvState RecvHander::read(int fd, const TCPConnectionPtr &ptr, const TCPMessageCB &msgCB)
{
    if (_state == CurrentRecv::RecvHeader)
    {
        int32_t thisRead = ::read(fd, _message.data() + _nread, _hMaxLen - _nread);
        if (thisRead <= 0)
        {
            return static_cast<RecvState>(thisRead);
        }
        _nread += thisRead;
        while (1)
        {
            auto [headerLen, contentLen] = _ifCompleteHeader(_message.data(), _nread);
            if (headerLen != HeaderNotFound)
            {
                if (_nread > headerLen + contentLen) [[unlikely]]
                {
                    std::vector<char> extraRead(_message.begin() + headerLen + contentLen, _message.begin() + _nread);
                    _nread = extraRead.size();
                    extraRead.resize(_hMaxLen);
                    _message.resize(headerLen + contentLen);
                    msgCB(ptr, std::move(_message));
                    _message = std::move(extraRead);
                }
                else if (_nread == headerLen + contentLen) [[unlikely]]
                {
                    _message.resize(_nread);
                    msgCB(ptr, std::move(_message));
                    reset();
                    break;
                }
                else [[likely]]
                {
                    _message.resize(headerLen + contentLen);
                    _state = CurrentRecv::RecvMsg;
                    break;
                }
            }
            else
            {
                if (_nread == _hMaxLen)
                {
                    log_error("exceed header max len");
                    return RecvState::RecvErr;
                }
                break;
            }
        }
    }
    else
    {
        int32_t thisRead = ::read(fd, _message.data() + _nread, _message.size() - _nread);
        if (thisRead <= 0)
        {
            return static_cast<RecvState>(thisRead);
        }
        _nread += thisRead;
        if (_nread == _message.size())
        {
            msgCB(ptr, std::move(_message));
            reset();
        }
    }
    return RecvState::RecvNormal;
}

void RecvHander::reset()
{
    _message.resize(_hMaxLen);
    _nread = 0;
    _state = CurrentRecv::RecvHeader;
}
