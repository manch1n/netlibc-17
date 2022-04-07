#pragma once
#include <vector>

#include <boost/noncopyable.hpp>

#include "Timestamp.h"
#include "Channel.h"
class EventLoop;

class Poller : boost::noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;
    Poller(EventLoop *loop);

    Timestamp poll(ChannelList *activated);

    void addIntrest(Channel *chan);
    void delIntrest(Channel *chan);
    void updateIntrest(Channel *chan);
    ~Poller();

private:
    int _pfd;
    int _nintrest;
    EventLoop *_owner;
};
