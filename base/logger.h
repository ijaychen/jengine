#ifndef BASE_LOGGER_H
#define BASE_LOGGER_H

#include "global.h"
#include <cerrno>
#include <cassert>
#include <ctime>
#include <string>
#include <cstring>
#include <iostream>

namespace base
{
    enum LogLevel {
        LogLevelDebug,
        LogLevelWarn,
        LogLevelError,
    };

    // 日志记录器(non thread safe)
    class FileLogger
    {
    public:
        DISABLE_COPY(FileLogger)

        FileLogger();
        virtual ~FileLogger();

        static FileLogger& instance() {
            static FileLogger ins;
            return ins;
        }

        bool Setup(const std::string& logdir, LogLevel loglevel = LogLevelDebug);

        void Debug(const char* format, ...);
        void Debug2(const char* info, const char* format, ...);
        void Debug3(const char* file, int line, const char* format, ...);

        void Warn(const char* format, ...);
        void Warn2(const char* info, const char* format, ...);
        void Warn3(const char* file, int line, const char* format, ...);

        void Error(const char* format, ...);
        void Error2(const char* info, const char* format, ...);
        void Error3(const char* file, int line, const char* format, ...);

    private:
        void CheckIfCreateNew();
        bool OpenLogFile();

        LogLevel level_;
        std::string dir_;
        FILE* fd_;
        tm start_;
        tm now_;
    };

    class FileLoggerLocker
    {
    public:
        FileLoggerLocker();
        ~FileLoggerLocker();
    };
}

// 线程不安全(non thread safe)
#define LOG_DEBUG(format, ...) base::FileLogger::instance().Debug(format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) base::FileLogger::instance().Warn(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) base::FileLogger::instance().Error(format, ##__VA_ARGS__)

#define LOG_DEBUG2(format, ...) base::FileLogger::instance().Debug2(__PRETTY_FUNCTION__, format, ##__VA_ARGS__)
#define LOG_WARN2(format, ...) base::FileLogger::instance().Warn2(__PRETTY_FUNCTION__, format, ##__VA_ARGS__)
#define LOG_ERROR2(format, ...) base::FileLogger::instance().Error2(__PRETTY_FUNCTION__, format, ##__VA_ARGS__)

#define LOG_DEBUG3(format, ...) base::FileLogger::instance().Debug3(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN3(format, ...) base::FileLogger::instance().Warn3(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR3(format, ...) base::FileLogger::instance().Error3(__FILE__, __LINE__, format, ##__VA_ARGS__)

// 线程安全(thread safe)
#define TS_LOG_DEBUG(format, ...) do { base::FileLoggerLocker locker; base::FileLogger::instance().Debug(format, ##__VA_ARGS__); } while(0)
#define TS_LOG_WARN(format, ...) do { base::FileLoggerLocker locker; base::FileLogger::instance().Warn(format, ##__VA_ARGS__); } while(0)
#define TS_LOG_ERROR(format, ...) do { base::FileLoggerLocker locker; base::FileLogger::instance().Error(format, ##__VA_ARGS__); } while(0)

#define TS_LOG_DEBUG2(format, ...) do { base::FileLoggerLocker locker; base::FileLogger::instance().Debug2(__PRETTY_FUNCTION__, format, ##__VA_ARGS__); } while(0)
#define TS_LOG_WARN2(format, ...) do { base::FileLoggerLocker locker;   base::FileLogger::instance().Warn2(__PRETTY_FUNCTION__, format, ##__VA_ARGS__); } while(0)
#define TS_LOG_ERROR2(format, ...) do { base::FileLoggerLocker locker;  base::FileLogger::instance().Error2(__PRETTY_FUNCTION__, format, ##__VA_ARGS__); } while(0)

#define TS_LOG_DEBUG3(format, ...) do { base::FileLoggerLocker locker; base::FileLogger::instance().Debug3(__FILE__, __LINE__, format, ##__VA_ARGS__); } while(0)
#define TS_LOG_WARN3(format, ...) do { base::FileLoggerLocker locker; base::FileLogger::instance().Warn3(__FILE__, __LINE__, format, ##__VA_ARGS__); } while(0)
#define TS_LOG_ERROR3(format, ...) do { base::FileLoggerLocker locker; base::FileLogger::instance().Error3(__FILE__, __LINE__, format, ##__VA_ARGS__); } while(0)

#define errno_assert(x) \
    do {\
        if (!(x)) { \
            const char* errmsg = strerror(errno);\
            std::cerr<<__FILE__<<":"<<__LINE__<<" "<<errmsg<<std::endl;\
        } \
        assert(x); \
    } while (false);

#define dev_assert(x)\
    assert(x);

#define debug_assert(x)\
    assert(x);

#define release_assert(x)\
    assert(x);

#endif // LOGGER_H
