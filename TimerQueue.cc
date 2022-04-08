#include "TimerQueue.h"

#include <sys/timerfd.h>

#include <cstring>
#include <cassert>

#include "EventLoop.h"

TimerQueue::TimerQueue(EventLoop *loop) : _tfd(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)), _chan(loop, _tfd), _loop(loop)
{
    _chan.setReadCB(std::bind(&TimerQueue::timerReadHandler, this));
    _loop->addChannel(&_chan);
    _chan.enableReading();
}

void TimerQueue::addTimer(const Timer::TimerCB &tcb, double when, double interval)
{
    Timer timer(tcb, when, interval);
    if (_timers.empty() || timer < _timers.top())
    {
        _timers.push(timer);
        resetTimerfd();
        return;
    }
    _timers.push(timer);
}

void TimerQueue::timerReadHandler()
{
    uint64_t times;
    size_t n = ::read(_tfd, &times, sizeof(uint64_t));
    assert(n == sizeof(uint64_t));
    auto funcs = getExpired(Timestamp::now());
    if (_timers.empty() == false)
    {
        resetTimerfd();
    }
    for (const auto &func : funcs)
    {
        func();
    }
}

TimerQueue::~TimerQueue()
{
    _loop->delChannel(&_chan);
    ::close(_tfd);
}

void TimerQueue::resetTimerfd()
{
    assert(_timers.empty() == false);
    struct itimerspec newTime;
    ::bzero(&newTime, sizeof(newTime));
    const auto &timer = _timers.top();
    constexpr uint64_t nano = 1000000000;
    uint64_t seconds, nanoSeconds;
    Timestamp now = Timestamp::now();
    uint64_t nanoDuration = timer._expiredTime.getNanoseconds() - now.getNanoseconds();
    seconds = nanoDuration / nano;
    nanoSeconds = nanoDuration - seconds * nano;
    newTime.it_value.tv_sec = seconds;
    newTime.it_value.tv_nsec = nanoSeconds;
    int ret = ::timerfd_settime(_tfd, 0, &newTime, nullptr);
    assert(ret != -1);
}

std::vector<Timer::TimerCB> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Timer::TimerCB> ret;
    while (_timers.empty() == false && _timers.top()._expiredTime < now)
    {
        const auto &curTimer = _timers.top();
        if (curTimer._running == false)
        {
            _timers.pop();
            continue;
        }
        ret.push_back(curTimer._tcb);
        if (curTimer._repeate == true)
        {
            _timers.emplace(curTimer._tcb, curTimer._interval, curTimer._interval);
        }
        _timers.pop();
    }
    return ret;
}
