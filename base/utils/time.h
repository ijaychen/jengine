#ifndef BASE_UTILS_TIME_H
#define BASE_UTILS_TIME_H

#include <time.h>
#include <stdint.h>
#include <string>

namespace base
{
    namespace utils
    {
        enum WeekDay {
            SUNDAY = 0,
            MONDAY = 1,
            TUESDAY = 2,
            WENDESDAY = 3,
            THURSDAY = 4,
            FRIDAY = 5 ,
            SATURDAY = 6,
        };

        static const int64_t SECONDS_OF_ONE_HOUR = 3600;
        static const int64_t TICKS_OF_ONE_HOUR = SECONDS_OF_ONE_HOUR * 1000;
        static const int64_t SECONDS_OF_ONE_DAY = SECONDS_OF_ONE_HOUR * 24;
        static const int64_t TICKS_OF_ONE_DAY = TICKS_OF_ONE_HOUR * 24;

        inline int64_t nowtick()
        {
            struct timespec now;
            clock_gettime(CLOCK_REALTIME, &now);
            return now.tv_sec * 1000 + now.tv_nsec / 1000000;
        }

        // 时间戳，单位为秒 (since epoch)
        inline int64_t timestamp()
        {
            return time(NULL);
        }

        inline int days_since_epoch()
        {
            return timestamp() / (24 * 60 * 60);
        }

        // 指定时间戳是否在今天指定点之前
        // today_hour 小时
        bool before_timepoint_in_today(int64_t timestamp, uint32_t today_hour);

        // 获取指定时间戳所在的天数的某个指定的小时数的timestamp
        // today_hour 小时
        int64_t get_today_timestamp_at_hour(int64_t timestamp, uint32_t hour);
        
        /*
         * 获取是否为当天
         * @timestamp 单位秒
         */
        inline int64_t is_today_from_timestamp(int64_t timestamp)
        {
            int64_t nowts = time(NULL);

            /*
            tm t1, t2;
            localtime_r(&nowts, &t1);
            localtime_r(&timestamp, &t2);

            bool is_today = (t1.tm_year == t2.tm_year && t1.tm_mon == t2.tm_mon && t1.tm_mday == t2.tm_mday);
            */
            return (nowts - timezone) / SECONDS_OF_ONE_DAY == (timestamp - timezone) / SECONDS_OF_ONE_DAY;
        }

        /*
         * 获取当天0点时间截
         * @timestamp 单位秒
         */
        inline int64_t today_zero_timestamp()
        {
            int64_t nowts = time(NULL);

            /*
            tm t;
            localtime_r(&nowts, &t);

            t.tm_hour = 0;
            t.tm_min = 0;
            t.tm_sec = 0;

            int64_t zerots = mktime(&t);
            */

            return (nowts - timezone / SECONDS_OF_ONE_DAY) * SECONDS_OF_ONE_DAY;

            //return zerots;
        }

        //根据时间戳，获取星期，时间戳单位为毫秒
        inline WeekDay GetWeekDay(int64_t tick)
        {
            time_t tick_t = tick / 1000;
            tm* p = gmtime(&tick_t);
            WeekDay result = (WeekDay)p->tm_wday;
            p = NULL;
            return result;
        }

        //获取当前时间的小时数
        //[0-23]
        inline int GetHour(int64_t tick)
        {
            time_t tick_t = tick / 1000;
            tm* p = gmtime(&tick_t);
            int hour = p->tm_hour;
            p = NULL;
            return hour;
        }

        //获取当前时间为星期几
        inline WeekDay now_weekday()
        {
            return GetWeekDay(nowtick());
        }

        //获取当前时间为当月第几天，时间戳单位为毫秒
        inline int GetMonday(int64_t tick)
        {
            time_t tick_t = tick / 1000;
            tm* p = gmtime(&tick_t);
            int result = p->tm_mday;
            p = NULL;
            return result;
        }
        //获取现在时间为当月第几天
        inline int now_monday()
        {
            return GetMonday(nowtick());
        }

        //一天的时间戳，毫秒
        inline int64_t daytick()
        {
            return 86400000;
        }

        //一小时的时间戳，毫秒
        inline int64_t hourtick()
        {
            return 3600000;
        }

        //时间戳增加days天
        inline int64_t AddDay(int64_t tick , float days)
        {
            return tick + (int64_t)(daytick() * days);
        }

        // 产生"YYYY-MM-DD hh:mm:ss"格式的字符串。
        inline std::string format_time_str(time_t tick = 0)
        {
            if (tick == 0)
                tick = time(NULL);

            static char buff[32];
            strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", localtime(&tick));

            return std::string(buff);
        }
    }
}

#endif
