#pragma once
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
class ThreadPool
{
public:
    using ThreadFunc = std::function<void()>;
    ThreadPool(int threadNum);
    void pushTask(const ThreadFunc &task);
    ~ThreadPool();
    void stop();

private:
    void runner();
    ThreadFunc getTask();

private:
    bool _running;
    std::condition_variable _cond;
    std::mutex _mutex;
    std::queue<ThreadFunc> _queue;
    std::vector<std::thread> _workers;
};