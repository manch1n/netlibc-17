#include "Timer.h"

Timer::Timer(const TimerCB &tcb, double when, double interval)
    : _tcb(tcb),
      _expiredTime(Timestamp::now() + when),
      _interval(interval),
      _repeate(interval > 0.0),
      _running(true)
{
}

void Timer::stop()
{
    _running = false;
}
