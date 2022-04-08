#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadNum) : _running(true)
{
    assert(threadNum >= 1);
    for (int i = 0; i < threadNum; ++i)
    {
        _workers.emplace_back(&ThreadPool::runner, this);
    }
}

ThreadPool::ThreadFunc ThreadPool::getTask()
{
    std::unique_lock lock(_mutex);
    _cond.wait(lock, [&]()
               { return !_queue.empty() || !_running; });
    ThreadFunc task = []() {};
    if (!_queue.empty())
    {
        task = _queue.front();
        _queue.pop();
    }
    return task;
}

void ThreadPool::runner()
{
    while (_running)
    {
        ThreadFunc task = getTask();
        task();
    }
}

void ThreadPool::pushTask(const ThreadFunc &task)
{
    assert(_running == true);
    std::lock_guard lock(_mutex);
    _queue.push(task);
    _cond.notify_one();
}

void ThreadPool::stop()
{
    _running = false;
    _cond.notify_all();
    for (auto &worker : _workers)
    {
        worker.join();
    }
}

ThreadPool::~ThreadPool()
{
    if (_running)
    {
        stop();
    }
}