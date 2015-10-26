#include "time.h"
#include "../event/dispatcher.h"

namespace base
{
    namespace utils
    {
        bool before_timepoint_in_today(int64_t timestamp, uint32_t today_hour)
        {
            int64_t today = (event::Dispatcher::instance().GetTimestampCache() - timezone) / SECONDS_OF_ONE_DAY;
            int64_t ts = today * SECONDS_OF_ONE_DAY + today_hour * SECONDS_OF_ONE_HOUR + timezone;
            return timestamp < ts;
        }

        int64_t get_today_timestamp_at_hour(int64_t timestamp, uint32_t hour)
        {
            int64_t today = (timestamp - timezone) / SECONDS_OF_ONE_DAY;
            return today * SECONDS_OF_ONE_DAY + hour * SECONDS_OF_ONE_HOUR + timezone;
        }

    }
}
