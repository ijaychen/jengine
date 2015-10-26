#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace base
{
    namespace utils
    {
        using namespace std;

        void string_append_format(std::string& str, const char* format, ...)
        {
            static char tmp[2048];
            va_list va;
            va_start(va, format);
            vsprintf(tmp, format, va);
            va_end(va);
            str.append(tmp);
        }

        string string_dump_binary(const string& src)
        {
            string ret;
            for (uint32_t i = 0; i < src.length(); ++i) {
                ret.append(base::utils::to_string((uint32_t)(uint8_t)src[i]));
                ret.append(",");
            }
            return ret;
        }

        string to_string(int32_t v)
        {
            static char buff[128];
            memset(buff, 0, 128);
            sprintf(buff, "%d", v);
            return string(buff);
        }

        string to_string(uint32_t v)
        {
            static char buff[128];
            memset(buff, 0, 128);
            sprintf(buff, "%u", v);
            return string(buff);
        }

        size_t utf8strlen(const string& data)
        {
            size_t result = 0;
            mbstate_t s;
            memset(&s, 0, sizeof(s));
            //setlocale(LC_ALL, "en_US.utf8");
            const char* begin = data.data();
            const char* end = data.data() + data.length();
            size_t r = mbrlen(begin, end - begin,  &s);
            while (r > 0) {
                begin += r;
                ++result;
                r = mbrlen(begin, end - begin,  &s);
            }
            return result;
        }
    }
}
