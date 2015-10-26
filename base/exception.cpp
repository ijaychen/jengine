#include "exception.h"

namespace base
{
    Exception::Exception(const char* what)
        : what_(what)
    {
    }

    Exception::Exception(const std::string& what)
        : what_(what)
    {
    }

    Exception::~Exception() throw()
    {
    }

    const char* Exception::what() const throw()
    {
        return what_.c_str();
    }
}
