#include "Message.h"
#include <cassert>

MessageHeader::MessageHeader(int32_t hMaxLen)
    : _valid(true),
      _buf(hMaxLen),
      _nread(0)
{
}

MessageHeader::MessageHeader(std::vector<char> &&extra, int32_t hMaxLen)
    : _valid(true),
      _buf(std::move(extra)),
      _nread(_buf.size())
{
    _buf.resize(hMaxLen);
}

int32_t MessageHeader::read(int fd)
{
    assert(_valid);
    assert(_nread < static_cast<int32_t>(_buf.size()));
    int32_t nread = ::read(fd, _buf.data() + _nread, static_cast<int32_t>(_buf.size()) - _nread);
    if (nread <= 0)
    {
        return nread;
    }
    _nread += nread;
    return nread;
}

std::optional<HeaderRet> MessageHeader::tryGetHeader(const IFCompleteHeader &judge)
{
    auto [headerLen, contentLen] = judge(_buf.data(), _nread);
    if (headerLen > 0)
    {
        assert(contentLen >= 0);
        assert(headerLen <= _nread);
        std::vector<char> extraRead(_nread - headerLen);
        if (headerLen < _nread)
        {
            std::copy(_buf.begin() + headerLen, _buf.begin() + _nread, extraRead.begin());
        }
        _buf.resize(headerLen);
        _valid = false;
        return std::make_optional<HeaderRet>(std::move(_buf), contentLen, std::move(extraRead));
    }
    else
        return {};
}

void MessageHeader::swap(MessageHeader &rhs)
{
    std::swap(_valid, rhs._valid);
    std::swap(_buf, rhs._buf);
    std::swap(_nread, rhs._nread);
}

MessageBuf::MessageBuf(int32_t msgSize, std::vector<char> &&content)
    : _valid(true),
      _buf(std::move(content)),
      _nread(_buf.size())
{
    _buf.resize(msgSize);
}

int32_t MessageBuf::read(int fd)
{
    int32_t nread = ::read(fd, _buf.data() + _nread, static_cast<int32_t>(_buf.size()) - _nread);
    if (nread <= 0)
    {
        return nread;
    }
    _nread += nread;
    return nread;
}

std::optional<std::vector<char>> MessageBuf::tryGetMessage()
{
    if (_nread == _buf.size())
    {
        return {std::move(_buf)};
    }
    else
        return {};
}

void MessageBuf::swap(MessageBuf &rhs)
{
    std::swap(_valid, rhs._valid);
    std::swap(_buf, rhs._buf);
    std::swap(_nread, rhs._nread);
}

RecvHander::RecvHander(int hMaxLen, const IFCompleteHeader &judge)
    : _hMaxLen(hMaxLen),
      _state(RecvState::recvHeader),
      _judge(judge),
      _curHeader(hMaxLen),
      _curContent()
{
}

int32_t RecvHander::read(int fd, const TCPConnectionPtr &ptr, const TCPMessageCB &msgCB)
{
    if (_state == RecvState::recvHeader)
    {
        int32_t nread = _curHeader.read(fd);
        if (nread <= 0)
            return nread;
        recvHeader(ptr, msgCB);
        return nread;
    }
    else if (_state == RecvState::recvMsg)
    {
        int32_t nread = _curContent.read(fd);
        if (nread <= 0)
            return nread;
        recvMsg(ptr, msgCB);
        return nread;
    }
    assert(0);
    return -1;
}

void RecvHander::recvHeader(const TCPConnectionPtr &ptr, const TCPMessageCB &msgCB)
{
    while (1)
    {
        auto header = _curHeader.tryGetHeader(_judge);
        if (header)
        {
            auto &[rawHeader, contentLen, extraRead] = header.value();
            if (contentLen > static_cast<int32_t>(extraRead.size())) [[likely]]
            {
                _state = RecvState::recvMsg;
                MessageBuf newBuf(contentLen, std::move(extraRead));
                _curContent.swap(newBuf);
                _rawHeaderTmp = std::move(rawHeader);
                break;
            }
            else if (contentLen == static_cast<int32_t>(extraRead.size()))
            {
                msgCB(ptr, std::move(rawHeader), std::move(extraRead));
                _state = RecvState::recvHeader;
                MessageHeader newHeader(_hMaxLen);
                _curHeader.swap(newHeader);
                break;
            }
            else if (contentLen < static_cast<int32_t>(extraRead.size()))
            {
                std::vector<char> nextExtra(extraRead.begin() + contentLen, extraRead.end());
                extraRead.resize(contentLen);
                msgCB(ptr, std::move(rawHeader), std::move(extraRead));
                MessageHeader newHeader(std::move(nextExtra), _hMaxLen);
                _curHeader.swap(newHeader);
            }
        }
        else
            break;
    }
}

void RecvHander::recvMsg(const TCPConnectionPtr &ptr, const TCPMessageCB &msgCB)
{
    auto msg = _curContent.tryGetMessage();
    if (msg)
    {
        msgCB(ptr, std::move(_rawHeaderTmp), std::move(msg.value()));
        _state = RecvState::recvHeader;
        MessageHeader newHeader(_hMaxLen);
        _curHeader.swap(newHeader);
    }
}
