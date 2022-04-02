#pragma once

#include <thread>
#include <mutex>
#include <memory>
#include <vector>

class Observable;

class Observer : public std::enable_shared_from_this<Observer>
{
public:
    void observe(Observable *);
    virtual ~Observer(){};

    virtual void update() = 0;

protected:
    Observable *_subject;
};

class Observable
{
public:
    void notify();
    void registerObserver(const std::weak_ptr<Observer> &observer);

private:
    mutable std::mutex _mutex;
    std::vector<std::weak_ptr<Observer>> _observers;
};

inline void Observer::observe(Observable *observ)
{
    observ->registerObserver(weak_from_this());
    _subject = observ;
}

inline void Observable::notify()
{
    std::lock_guard lock(_mutex);
    auto it = _observers.begin();
    while (it != _observers.end())
    {
        if (auto ptr = it->lock(); ptr)
        {
            ptr->update();
            it++;
        }
        else
        {
            it = _observers.erase(it);
        }
    }
}

inline void Observable::registerObserver(const std::weak_ptr<Observer> &observer)
{
    std::lock_guard lock(_mutex);
    _observers.push_back(observer);
}
