#include "TCPServer.h"

TCPServer::TCPServer(EventLoop *loop, const IPv4Address &addr, const TCPConnConf &conf)
    : _loop(loop),
      _curIdentifier(0),
      _acceptor(std::make_unique<Acceptor>(loop, addr)),
      _saveConn(),
      _conf(conf)
{
    auto userCloCB = _conf.cloCB;
    _conf.cloCB = [this, userCloCB](const TCPConnectionPtr &ptr)
    {
        userCloCB(ptr);
        this->closeConn(ptr);
    };
    _acceptor->setAcceptCB(std::bind(&TCPServer::newConn, this, std::placeholders::_1));
    _acceptor->startAccept();
}

void TCPServer::newConn(int fd)
{
    uint64_t id = _curIdentifier.fetch_add(1);
    log_debug("%s", getAddressByFd(fd).IPPortstr().c_str());
    TCPConnectionPtr ptr = std::make_shared<TCPConnection>(_loop, fd, id, _conf);
    _saveConn[id] = ptr;
    _conf.connCB(ptr);
}

void TCPServer::closeConn(const TCPConnectionPtr &ptr)
{
    _saveConn.erase(ptr->getIndentifier());
}
