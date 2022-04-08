#include "TCPServer.h"

TCPServer::TCPServer(EventLoop *loop, const IPv4Address &addr, const TCPMessageCB &msgcb, const TCPErrorCB &errcb, const TCPConnectedCB &conncb, const TCPClosingCB &clocb)
    : _loop(loop),
      _curIdentifier(0),
      _acceptor(std::make_unique<Acceptor>(loop, addr)),
      _saveConn(),
      _msgCB(msgcb),
      _errCB(errcb),
      _connCB(conncb),
      _cloCB(clocb)
{
    _acceptor->setAcceptCB(std::bind(&TCPServer::newConn, this, std::placeholders::_1));
    _acceptor->startAccept();
}

void TCPServer::newConn(int fd)
{
    uint64_t id = _curIdentifier.fetch_add(1);
    log_debug("%s", getAddressByFd(fd).IPPortstr().c_str());
    TCPConnectionPtr ptr = std::make_shared<TCPConnection>(_loop, fd, id, _msgCB, _errCB, _connCB, std::bind(&TCPServer::closeConn, this, std::placeholders::_1));
    _saveConn[id] = ptr;
    _connCB(ptr);
}

void TCPServer::closeConn(const TCPConnectionPtr &ptr)
{
    _cloCB(ptr);
    _saveConn.erase(ptr->getIndentifier());
}
