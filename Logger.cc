#include "Logger.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <chrono>

using namespace Logger;

void Logger::Impl::StdIOLogger::handler(std::string_view msg)
{
    ::fwrite(msg.data(), 1, msg.size(), stdout);
    ::fflush(stdout);
}

Logger::Impl::FileLogger::FileLogger()
    : _worker(&Logger::Impl::FileLogger::workerIOThread, this),
      _running(true)
{
    Timestamp now;
    std::string logName = now.YearMonthDay() + ".log";
    _fp = ::fopen(logName.c_str(), "w");
    assert(_fp != nullptr);
}

void Logger::Impl::FileLogger::handler(std::string_view msg)
{
    std::lock_guard lock(_mutex);
    if (_curBuf.push_msg(msg))
    {
        return;
    }
    else
    {
        _buffers.push_back(std::move(_curBuf));
        _curBuf = FileBuf();
        _curBuf.push_msg(msg);
    }
    if (_buffers.size() >= 3)
    {
        _cond.notify_one();
    }
}

void Logger::Impl::FileLogger::workerIOThread()
{
    while (_running)
    {
        decltype(_buffers) oldBufs;
        {
            std::unique_lock lock(_mutex);
            if (_buffers.size() < FileBufThreshold)
            {
                namespace ch = std::chrono;
                auto tp = ch::system_clock::now() + ch::seconds(FileBufMaxSeconds);
                _cond.wait_until(lock, tp);
            }
            oldBufs.swap(_buffers);
        }
        pushFile(oldBufs);
    }
}

void Logger::Impl::FileLogger::pushFile(const std::vector<FileBuf> &bufs)
{
    for (const auto &buf : bufs)
    {
        int size = buf._size;
        int nwrite = ::fwrite(buf._data.data(), 1, size, _fp);
        assert(nwrite == size);
    }
    ::fflush(_fp);
}

void enableFileLog()
{
    g_fileLogger.init();
    auto &logInstance = g_fileLogger.instance();
    Impl::g_logHandler = std::bind(&Impl::FileLogger::handler, &logInstance, std::placeholders::_1);
}
