#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop, int fd)
    : _fd(fd),
      _owner(loop), _event(0), _revent(0)
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

void Channel::enableReading()
{
  _event = _event | EPOLLIN;
  _owner->updateChannel(this);
}

void Channel::enableWriting()
{
  _event = _event | EPOLLOUT;
  _owner->updateChannel(this);
}

void Channel::disableWriting()
{
  _event = _event & (~EPOLLOUT);
  _owner->updateChannel(this);
}

void Channel::disableReading()
{
  _event = _event & (~EPOLLIN);
  _owner->updateChannel(this);
}

Channel::~Channel()
{
  //_owner->delChannel(this);
}
