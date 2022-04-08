#include "SocketWrapper.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <cstring>

#include <cassert>

int createNonblockSockOrDie()
{
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // default tcp
    assert(fd != -1);
    return fd;
}

// block connect but nonblock communication
int connectRemoteOrDie(const IPv4Address &remote)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(fd != -1);
    assert(::connect(fd, remote.getUnderlyAddr(), AddrLen) != -1);
    assert(::fcntl(fd, F_SETFL, O_NONBLOCK) != -1);
    return fd;
}

void listenOrDie(int fd)
{
    assert(::listen(fd, 10) != -1);
}

// default reuse addr
int bindListenOrDie(const IPv4Address &addr)
{
    int fd = createNonblockSockOrDie();
    int on = 1;
    assert(::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != -1);
    assert(::bind(fd, addr.getUnderlyAddr(), AddrLen) != -1);
    listenOrDie(fd);
    return fd;
}

int acceptClient(int fd)
{
    int clientfd = accept4(fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    return clientfd;
}

IPv4Address getAddressByFd(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ::bzero(&addr, len);
    int ret = ::getpeername(fd, (struct sockaddr *)&addr, &len);
    assert(ret != -1);
    return IPv4Address(addr);
}
