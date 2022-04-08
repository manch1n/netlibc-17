#pragma once
#include "IPv4Address.h"

int connectRemoteOrDie(const IPv4Address &remote);

int bindListenOrDie(const IPv4Address &addr);

int acceptClient(int fd);

IPv4Address getAddressByFd(int fd);