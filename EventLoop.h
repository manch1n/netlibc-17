#pragma once

#include <memory>

#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "Logger.h"

class EventLoop : boost::noncopyable
{
public:
    using RunInLoopCB = std::function<void()>;

    EventLoop();
    void loop();
    void stop() { _running = false; }

    void runOnce(double init, const Timer::TimerCB &tcb);
    void runEvery(double init, double interval, const Timer::TimerCB &tcb);

    void runInLoop(const RunInLoopCB &cb);

    void assertRunInLoopThread()
    {
        assert(runInLoopThread());
    }

    void addChannel(Channel *chan)
    {
        _poller->addIntrest(chan);
    }

    void delChannel(Channel *chan)
    {
        _poller->delIntrest(chan);
    }

    void updateChannel(Channel *chan)
    {
        _poller->updateIntrest(chan);
    }

    ~EventLoop()
    {
        wakeUp(); // manualy wakeup
        runInLoopHandler();
        delChannel(&_runInLoopChan);
        ::close(_inLoopEfd);
    }

private:
    bool runInLoopThread()
    {
        return _belongTo == ::gettid();
    }

    void runInLoopHandler();

    void wakeUp();

    bool _running;
    std::unique_ptr<Poller> _poller;
    std::unique_ptr<TimerQueue> _timerQueue;
    pid_t _belongTo;
    std::vector<RunInLoopCB> _inLoopQueue;
    int _inLoopEfd;
    std::mutex _runInLoopMutex;
    Channel _runInLoopChan;
};