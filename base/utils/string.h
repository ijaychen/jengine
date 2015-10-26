#ifndef BASE_UTILS_STRING_H
#define BASE_UTILS_STRING_H

#include "../global.h"
#include <string>
#include <vector>

namespace base
{
    namespace utils
    {
        using namespace std;

        // 切割字符串
        inline std::vector<string> split_string(const string& str , char separator , bool trim_empty = true)
        {
            std::vector<string> list;
            for (size_t pos = 0 ; pos < str.length(); ++pos) {
                size_t tag = str.find( separator , pos);
                string temp;
                if (tag != string::npos) {
                    temp = str.substr(pos , tag - pos);
                    pos = tag;
                } else {
                    temp = str.substr(pos);
                }
                if (!trim_empty || !temp.empty()) {
                    list.push_back(temp);
                }
                if (tag == string::npos) break;
            }
            return list;
        }

        // 计算字符串哈希值
        inline uint32_t hash_string(const char* str)
        {
            uint32_t seed = 13131; // 31 131 1313 13131 131313 etc..
            uint32_t hash = 0;
            while (*str) {
                hash = hash * seed + (*str++);
            }
            return (hash & 0x7FFFFFFF);
        }

        // 附加格式化字符串 (非线程安全)
        void string_append_format(std::string& str, const char* format, ...);

        // 将二进制数据转为可读格式
        std::string string_dump_binary(const std::string& src);

        std::string to_string(int32_t v);
        std::string to_string(uint32_t v);

        size_t utf8strlen(const std::string& data);
    }
}

#endif
