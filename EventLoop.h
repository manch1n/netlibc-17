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

    // threadsafe
    void runOnce(double init, const Timer::TimerCB &tcb);
    // threadsafe
    void runEvery(double init, double interval, const Timer::TimerCB &tcb);
    // threadsafe
    void runInLoop(const RunInLoopCB &cb);

    void assertRunInLoopThread()
    {
        assert(ifRunInLoopThread());
    }

    //  threadsafe
    void addChannel(Channel *chan)
    {
        _poller->addIntrest(chan);
    }
    // threadsafe
    void delChannel(Channel *chan)
    {
        _poller->delIntrest(chan);
    }
    // threadsafe
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

    bool ifRunInLoopThread()
    {
        return _belongTo == ::gettid();
    }

private:
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