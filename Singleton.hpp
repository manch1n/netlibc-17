#pragma once
#include <mutex>

template <typename T>
class Singleton
{
public:
    static T &instance()
    {
        std::call_once(_oflag, [&]()
                       { _value = new T{}; });
        return *_value;
    }

    template <typename... ARGS>
    static void init(ARGS &&...args)
    {
        std::call_once(_oflag, [&]()
                       { _value = new T(std::forward<ARGS>(args)...); });
    }

    ~Singleton()
    {
        delete _value;
    }

private:
    static T *_value;
    static std::once_flag _oflag;
};

template <typename T>
T *Singleton<T>::_value = nullptr;
template <typename T>
std::once_flag Singleton<T>::_oflag{};