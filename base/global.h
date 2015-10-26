#ifndef BASE_GLOBAL_H
#define BASE_GLOBAL_H

#ifndef __GNUC__
#error "only support gcc"
#endif

//#ifndef NOT_USE_CXX11
//    #if (__GNUC__ >=4) && (__GNUC_MINOR__ >= 6)
//        #define HAS_CXX11
//    #endif
//#endif

// gcc4.7.n or later
//#if (__GNUC__ >=4) && (__GNUC_MINOR__ >= 7)
//    
//#else
//    #define final
//    #define override
//#endif

#ifdef HAS_CXX11
    #include <cstdint>
    #define DISABLE_COPY(Class) \
    Class(const Class&) = delete; \
    Class & operator=(const Class&) = delete;
#else
    #include <stdint.h>
    #define nullptr NULL
    #define DISABLE_COPY(Class) \
    Class(const Class&); \
    Class & operator=(const Class&);
#endif

#include <cstddef>
#include <boost/concept_check.hpp>

#define SAFE_DELETE(ptr) do { if (ptr != nullptr) { delete ptr; ptr = nullptr; } } while(0)

#endif

