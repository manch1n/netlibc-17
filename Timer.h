#pragma once

#include <functional>

#include "Timestamp.h"

class Timer
{
public:
    using TimerCB = std::function<void()>;
    Timer(const TimerCB &tcb, double when, double interval);
    void stop();

    bool operator<(const Timer &right) const
    {
        return this->_expiredTime < right._expiredTime;
    }

    bool operator>(const Timer &right) const
    {
        return this->_expiredTime > right._expiredTime;
    }

public:
    TimerCB _tcb;
    Timestamp _expiredTime;
    double _interval;
    bool _repeate;
    bool _running;
};