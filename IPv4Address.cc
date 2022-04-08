#include "IPv4Address.h"

#include <cstring>
#include <cassert>
IPv4Address::IPv4Address(const std::string &dotAddr, uint16_t port)
{
    ::bzero(&_addr, sizeof(_addr));
    assert(::inet_pton(AF_INET, dotAddr.c_str(), &_addr.sin_addr) != -1);
    _addr.sin_family = AF_INET;
    _addr.sin_port = ::htons(port);
}

IPv4Address::IPv4Address(uint64_t port)
{
    ::bzero(&_addr, sizeof(_addr));
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_family = AF_INET;
    _addr.sin_port = ::htons(port);
}

IPv4Address::IPv4Address(const struct sockaddr_in &addr)
{
    ::memcpy(&_addr, &addr, sizeof(addr));
}

std::string IPv4Address::IPPortstr() const
{
    char buf[INET_ADDRSTRLEN + 10] = {0};
    assert(::inet_ntop(AF_INET, &_addr.sin_addr, buf, INET_ADDRSTRLEN) != nullptr);
    char *ipend = std::find(std::begin(buf), std::end(buf), '\0');
    assert(ipend != std::end(buf));
    ipend[0] = ':';
    ipend++;
    ::snprintf(ipend, std::distance(ipend, std::end(buf)), "%d", ::ntohs(_addr.sin_port));
    return buf;
}

const struct sockaddr *IPv4Address::getUnderlyAddr() const
{
    return reinterpret_cast<const struct sockaddr *>(&_addr);
}
