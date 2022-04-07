#pragma once
#include <functional>
#include <string_view>
#include <type_traits>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <cstdio>
#include <cassert>

#include "Timestamp.h"
#include "Singleton.hpp"
//笔记
// 1.在命名空间中不可以定义变量,只可以声明.正确的做法是在头文件extern声明,在源文件定义.或者使用inline,inline会使其在使用其头文件的多个翻译单元中有相同的定义,但是这些定义在程序中都有着相同的地址.
// 2.可以通过enableif来控制函数是否可以编译使用
// 3. 通过宏的'##'语法来生成函数
namespace Logger
{

    namespace Impl
    {
        using LogHandler = std::function<void(std::string_view msg)>;

        // level date msg
        enum class LogLevel
        {
            LOG_INFO = 0,
            LOG_DEBUG,
            LOG_WARN,
            LOG_ERROR,
        };
        inline const char *LOG_PREFIX[4] = {"INFO  ", "DEBUG ", "WARN  ", "ERROR "};
        constexpr unsigned LOG_MAX_LEN = 200;
        constexpr unsigned LOG_PREFIX_LEN = 6;
        constexpr unsigned LOG_MSG_LEN = LOG_MAX_LEN - LOG_PREFIX_LEN - Timestamp::DAY_TIME_LEN - 2; //'\n' and ' '
        inline LogLevel initLogLevel()
        {
            if (::getenv("LOG_DEBUG"))
            {
                return LogLevel::LOG_DEBUG;
            }
            else
            {
                return LogLevel::LOG_INFO;
            }
        }
        inline LogLevel g_logLevel = initLogLevel();
        inline int var_sprintf(char *dst, size_t maxlen, const char *fmt)
        {
            return snprintf(dst, maxlen, "%s", fmt);
        }

        template <typename... ARGS>
        typename std::enable_if<sizeof...(ARGS) != 0, int>::type
        var_sprintf(char *dst, size_t maxlen, const char *fmt, const ARGS &...args)
        {
            return snprintf(dst, maxlen, fmt, args...);
        }

        template <typename... ARGS>
        std::string formatLogMsg(LogLevel level, const char *fmt, const ARGS &...args)
        {
            std::string buf(LOG_MAX_LEN + 1, '\0');
            int offset = 0;
            const char *lp = LOG_PREFIX[static_cast<unsigned>(level)];
            std::copy(lp, lp + LOG_PREFIX_LEN, buf.data());
            Timestamp now = Timestamp::now();
            offset += LOG_PREFIX_LEN;
            now.HourMinSec(buf.data() + offset);
            offset += Timestamp::DAY_TIME_LEN;
            buf[offset] = ' ';
            offset += 1;
            int ret = var_sprintf(buf.data() + offset, LOG_MSG_LEN, fmt, args...);
            assert(ret != -1);
            offset += ret;
            buf[offset] = '\n';
            offset += 1;
            buf[offset] = '\0';
            buf.resize(offset);
            return buf;
        }
        class StdIOLogger
        {
        public:
            static void handler(std::string_view msg);
        };
        class FileLogger
        {
        public:
            static constexpr int FileBufSize = 8192;
            static constexpr int FileBufThreshold = 3;
            static constexpr int FileBufMaxSeconds = 3;
            FileLogger();
            void handler(std::string_view msg);

            ~FileLogger()
            {
                _running = false;
                _worker.join();
                _buffers.push_back(std::move(_curBuf));
                pushFile(_buffers);
                ::fclose(_fp);
            }

        private:
            struct FileBuf
            {
            public:
                FileBuf() : _size(0)
                {
                    _data.reserve(FileBufSize);
                    _data.resize(FileBufSize);
                }
                bool push_msg(std::string_view msg)
                {
                    if (static_cast<int>(msg.size()) > FileBufSize - _size)
                    {
                        return false;
                    }
                    std::copy(msg.begin(), msg.end(), _data.begin() + _size);
                    _size += msg.size();
                    return true;
                }
                int _size;
                std::vector<char> _data; // use vector instead of array
            };

            void workerIOThread();
            void pushFile(const std::vector<FileBuf> &bufs);

        private:
            mutable std::mutex _mutex;
            std::condition_variable _cond;
            FileBuf _curBuf;
            std::vector<FileBuf> _buffers; // size >=3 or 3 seconds push to files
            std::thread _worker;
            bool _running;
            FILE *_fp;
        };
        inline LogHandler g_logHandler = StdIOLogger::handler;
    }
    using Impl::LOG_MSG_LEN;

#define LOG_IMPL(level, LEVEL)                                                     \
    template <typename... ARGS>                                                    \
    void log_##level(const char *fmt, const ARGS &...args)                         \
    {                                                                              \
        if (Impl::g_logLevel > Impl::LogLevel::LOG_##LEVEL)                        \
        {                                                                          \
            return;                                                                \
        }                                                                          \
        std::string msg = formatLogMsg(Impl::LogLevel::LOG_##LEVEL, fmt, args...); \
        Impl::g_logHandler(msg);                                                   \
    }

    LOG_IMPL(info, INFO)
    LOG_IMPL(debug, DEBUG)
    LOG_IMPL(warn, WARN)
    LOG_IMPL(error, ERROR)

    inline Singleton<Impl::FileLogger> g_fileLogger;
}
using Logger::log_debug;
using Logger::log_error;
using Logger::log_info;
using Logger::log_warn;

void enableFileLog();
