#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop, int fd)
    : _fd(fd),
      _owner(loop), _event(EPOLLIN | EPOLLERR), _revent(0)
{
  //_owner->addChannel(this);
}

void Channel::handleEvent()
{
  if (_revent & EPOLLIN)
  {
    if (_readCB) [[likely]]
    {
      _readCB();
    }
  }
  if (_revent & EPOLLOUT)
  {
    if (_writeCB) [[likely]]
    {
      _writeCB();
    }
  }
  if (_revent & EPOLLERR)
  {
    if (_execpCB) [[likely]]
    {
      _execpCB();
    }
  }
  _revent = 0;
}

Channel::~Channel()
{
  //_owner->delChannel(this);
}
