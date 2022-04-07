#include "EventLoop.h"

#include <sys/eventfd.h>
#include <cassert>
EventLoop::EventLoop()
    : _running(false), _poller(new Poller(this)), _timerQueue(new TimerQueue(this)), _belongTo(::gettid()), _inLoopEfd(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)), _runInLoopChan(this, _inLoopEfd)
{
    assert(_inLoopEfd != -1);
    _runInLoopChan.setReadCB(std::bind(&EventLoop::runInLoopHandler, this));
    addChannel(&_runInLoopChan);
}

void EventLoop::loop()
{
    _running = true;
    while (_running)
    {
        Poller::ChannelList activated;
        _poller->poll(&activated);
        for (const auto &chan : activated)
        {
            chan->handleEvent();
        }
    }
}

void EventLoop::runOnce(double init, const Timer::TimerCB &tcb)
{
    _timerQueue->addTimer(tcb, init, 0);
}

void EventLoop::runEvery(double init, double interval, const Timer::TimerCB &tcb)
{
    _timerQueue->addTimer(tcb, init, interval);
}

void EventLoop::runInLoop(const RunInLoopCB &cb)
{
    if (runInLoopThread())
    {
        cb();
    }
    else
    {
        {
            std::lock_guard lock(_runInLoopMutex);
            _inLoopQueue.push_back(cb);
        }
        if (_inLoopQueue.empty() == false)
        {
            wakeUp();
        }
    }
}

void EventLoop::runInLoopHandler()
{
    uint64_t count;
    int nread = ::read(_inLoopEfd, &count, sizeof(uint64_t));
    assert(nread == sizeof(uint64_t));
    decltype(_inLoopQueue) oldQueue;
    {
        std::lock_guard lock(_runInLoopMutex);
        std::swap(oldQueue, _inLoopQueue);
    }
    for (const auto &cb : oldQueue)
    {
        cb();
    }
}

void EventLoop::wakeUp()
{
    uint64_t on = 1;
    size_t nwrite = ::write(_inLoopEfd, &on, sizeof(on)); // wake up
    assert(nwrite == sizeof(uint64_t));
}
