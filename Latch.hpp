#pragma once
#include <mutex>
#include <condition_variable>

class CountDownLatch
{
public:
    explicit CountDownLatch(int count) : _count(count) {}
    void wait()
    {
        std::unique_lock lock(_mutex);
        _cond.wait(lock, [&]()
                   { return _count == 0; });
    }

    void CountDown()
    {
        std::unique_lock lock(_mutex);
        if (--_count; _count == 0)
        {
            _cond.notify_all();
        }
    }

private:
    mutable std::mutex _mutex;
    std::condition_variable _cond;
    int _count;
};