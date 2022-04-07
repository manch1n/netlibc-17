#pragma once

#include <sys/epoll.h>

#include <functional>

#include <boost/noncopyable.hpp>

class EventLoop;
class Channel : boost::noncopyable
{
public:
    using EventCallBack = std::function<void()>;

    Channel(EventLoop *loop, int fd);

    void handleEvent();

    void disableWriting();
    void setrevent(int32_t re)
    {
        _revent = re;
    }
    int32_t event() const { return _event; }
    int fd() const { return _fd; }
    void setReadCB(const EventCallBack &rcb)
    {
        _readCB = rcb;
    }
    void setWriteCB(const EventCallBack &wcb)
    {
        _writeCB = wcb;
    }
    void setExecpCB(const EventCallBack &ecb)
    {
        _execpCB = ecb;
    }

    ~Channel();

private:
    int _fd;
    EventLoop *_owner;
    int32_t _event;
    int32_t _revent;

    EventCallBack _readCB;
    EventCallBack _writeCB;
    EventCallBack _execpCB;
};