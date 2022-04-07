#include "Poller.h"

#include <sys/epoll.h>
#include <cstring>

#include <cassert>

Poller::Poller(EventLoop *loop) : _nintrest(0), _owner(loop)
{
    _pfd = ::epoll_create1(EPOLL_CLOEXEC);
    assert(_pfd != -1);
}

Timestamp Poller::poll(ChannelList *activated)
{
    std::vector<struct epoll_event> ret(_nintrest);
    int nactivated = ::epoll_wait(_pfd, ret.data(), _nintrest, -1);
    assert(nactivated > 0);
    for (int i = 0; i < nactivated; ++i)
    {
        Channel *chan = reinterpret_cast<Channel *>(ret[i].data.ptr);
        chan->setrevent(ret[i].events);
        activated->push_back(chan);
    }
    return Timestamp::now();
}

void Poller::addIntrest(Channel *chan)
{
    struct epoll_event event;
    event.data.ptr = chan;
    event.events = chan->event();
    int ret = ::epoll_ctl(_pfd, EPOLL_CTL_ADD, chan->fd(), &event);
    assert(ret != -1);
    _nintrest++;
}

void Poller::delIntrest(Channel *chan)
{
    int ret = ::epoll_ctl(_pfd, EPOLL_CTL_DEL, chan->fd(), nullptr);
    assert(ret != -1);
    _nintrest--;
}

void Poller::updateIntrest(Channel *chan)
{
    struct epoll_event event;
    event.data.ptr = chan;
    event.events = chan->event();
    int ret = ::epoll_ctl(_pfd, EPOLL_CTL_MOD, chan->fd(), &event);
    assert(ret != -1);
}

Poller::~Poller()
{
    ::close(_pfd);
}
