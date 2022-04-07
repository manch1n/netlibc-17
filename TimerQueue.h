#pragma once

#include <queue>

#include "Timer.h"
#include "Channel.h"

class TimerQueue
{
public:
    TimerQueue(EventLoop *loop);
    void addTimer(const Timer::TimerCB &tcb, double when, double interval);

    void timerReadHandler();
    ~TimerQueue();

private:
    void resetTimerfd();
    std::vector<Timer::TimerCB> getExpired(Timestamp stamp);
    int _tfd;
    std::priority_queue<Timer, std::vector<Timer>, std::greater<Timer>> _timers;

    Channel _chan;
    EventLoop *_loop;
};