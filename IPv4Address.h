#pragma once
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

constexpr int AddrLen = sizeof(struct sockaddr_in);

class IPv4Address
{
public:
    IPv4Address(const std::string &dotAddr, uint16_t port);
    IPv4Address(uint64_t port);
    IPv4Address(const struct sockaddr_in &addr);
    std::string IPPortstr() const;
    const struct sockaddr *getUndelyAddr() const;

private:
    struct sockaddr_in _addr;
};
