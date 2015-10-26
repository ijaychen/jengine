#include "logger.h"
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include "file/utils.h"
#include "thread/mutex.h"

namespace base
{
    using namespace std;

    FileLogger::FileLogger()
        : level_(LogLevelDebug), dir_("./logs"), fd_(NULL)
    { 
    }

    FileLogger::~FileLogger()
    {
        if (fd_ != NULL) {
            fflush(fd_);
            fclose(fd_);
        }
    }

    bool FileLogger::Setup(const std::string& logdir, LogLevel level /* = LogLevelDebug */)
    {
        // {logdir}
        dir_ = logdir;
        level_ = level;
        time_t now = time(NULL);
        localtime_r(&now, &start_);

        return OpenLogFile();
    }

    bool FileLogger::OpenLogFile()
    {
        char filename[128];
        strcpy(filename, dir_.c_str());
        file::dir_ensure(filename, 0765);
        sprintf(filename + strlen(filename), "/log_%d.%.2d.%.2d",
                start_.tm_year + 1900, start_.tm_mon + 1, start_.tm_mday);

        if (fd_ != NULL) {
            fclose(fd_);
            fd_ = NULL;
        }

        fd_ = fopen(filename, "a");
        return fd_ != NULL;
    }

    void FileLogger::CheckIfCreateNew()
    {
        time_t now = time(NULL);        
        localtime_r(&now, &now_);
        if (now_.tm_year != start_.tm_year ||
                now_.tm_mon != start_.tm_mon ||
                now_.tm_mday != start_.tm_mday) {
            // 当前时间与当前日志文件所属时间不在同一天
            start_ = now_;
            OpenLogFile();
        }
    }

    static const char* debug = "DEBUG:";
    static const char* warn  = "WARN:";
    static const char* error = "ERROR:";

    static inline const char* loglevel2str(LogLevel level)
    {
        const char* levelstr;
        switch(level) {
        case LogLevelDebug:
            levelstr = debug;
            break;
        case LogLevelWarn:
            levelstr = warn;
            break;
        case LogLevelError:
            levelstr = error;
            break;
        }
        return levelstr;
    }

    static inline void print_header(FILE* fd, const tm& now, LogLevel level, const char* info)
    {
        const char* levelstr = loglevel2str(level);
        fprintf(fd, "%.2d:%.2d:%.2d [%6s]  %s => ",
                now.tm_hour, now.tm_min, now.tm_sec, levelstr, info);
        
        fprintf(stderr, "%.2d:%.2d:%.2d [%6s]  %s => ",
                now.tm_hour, now.tm_min, now.tm_sec, levelstr, info);
    }

    static inline void print_header(FILE* fd, const tm& now, LogLevel level)
    {
        const char* levelstr = loglevel2str(level);
        fprintf(fd, "%.2d:%.2d:%.2d [%6s]  ",
                now.tm_hour, now.tm_min, now.tm_sec, levelstr);
        
        fprintf(stderr, "%.2d:%.2d:%.2d [%6s]  ",
                now.tm_hour, now.tm_min, now.tm_sec, levelstr);
    }

    static inline void print_header(FILE* fd, const tm& now, LogLevel level, const char* file, int line)
    {
        const char* levelstr = loglevel2str(level);
        fprintf(fd, "%.2d:%.2d:%.2d [%6s]  %s:%d",
                now.tm_hour, now.tm_min, now.tm_sec, levelstr, file, line);
        
        fprintf(stderr, "%.2d:%.2d:%.2d [%6s]  %s:%d",
                now.tm_hour, now.tm_min, now.tm_sec, levelstr, file, line);
    }

#define PRINT2FILE()\
            va_list va;\
            va_start(va, format);\
            vfprintf(fd_, format, va);\
            if (format[strlen(format) - 1] != '\n') {\
                fputs("\n", fd_);\
            }\
            va_end(va);\
            fflush(fd_);\
            va_start(va, format);\
            vfprintf(stderr, format, va);\
            if (format[strlen(format) - 1] != '\n') {\
                fputs("\n", stderr);\
            }\
            va_end(va);\
            fflush(stderr)

    void FileLogger::Debug(const char* format, ...)
    {
        if (level_ <= LogLevelDebug) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelDebug);
            PRINT2FILE();
        }
    }

    void FileLogger::Debug2(const char* info, const char* format, ...)
    {
        if (level_ <= LogLevelDebug) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelDebug, info);
            PRINT2FILE();
        }
    }

    void FileLogger::Debug3(const char* file, int line, const char* format, ... )
    {
        if (level_ <= LogLevelDebug) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelDebug, file, line);
            PRINT2FILE();
        }
    }

    void FileLogger::Warn(const char* format, ...)
    {
        if (level_ <= LogLevelWarn) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelWarn);
            PRINT2FILE();
        }
    }

    void FileLogger::Warn2(const char* info, const char* format, ...)
    {
        if (level_ <= LogLevelWarn) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelWarn, info);
            PRINT2FILE();
        }
    }

    void FileLogger::Warn3(const char* file, int line, const char* format, ... )
    {
        if (level_ <= LogLevelWarn) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelWarn, file, line);
            PRINT2FILE();
        }
    }

    void FileLogger::Error(const char* format, ...)
    {
        if (level_ <= LogLevelError) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelError);
            PRINT2FILE();
        }
    }

    void FileLogger::Error2(const char* info, const char* format, ...)
    {
        if (level_ <= LogLevelError) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelError, info);
            PRINT2FILE();
        }
    }

    void FileLogger::Error3(const char* file, int line, const char* format, ... )
    {
        if (level_ <= LogLevelError) {
            CheckIfCreateNew();
            print_header(fd_, now_, LogLevelError, file, line);
            PRINT2FILE();
        }
    }
    
    static thread::Mutex fileloggermutex;
    
    FileLoggerLocker::FileLoggerLocker()
    {
        fileloggermutex.Lock();
    }

    FileLoggerLocker::~FileLoggerLocker()
    {
        fileloggermutex.Unlock();
    }
}
