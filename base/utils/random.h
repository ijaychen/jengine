#ifndef BASE_UTILS_RANDOM_H
#define BASE_UTILS_RANDOM_H

#include "../global.h"
#include <string>

namespace base
{
    namespace utils
    {
        class Random
        {
        public:
            DISABLE_COPY(Random)
            Random();
            ~Random();
            
            bool Setup();
            
            std::string GenRandomString(int size);
            
            // 生成随机数  0 <= 值 < max
            int GenRandomNum(int max);
            // 生成随机数 min <= 值 < max
            int GenRandomNum(int min, int max);
        };
    }
}

#endif // RANDOM_H
