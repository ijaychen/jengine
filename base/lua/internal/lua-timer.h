#ifndef BASE_LUA_INTERNAL_LUA_TIMER_H
#define BASE_LUA_INTERNAL_LUA_TIMER_H

#include "../../global.h"

class lua_State;

namespace base
{
    namespace lua
    {
        namespace internal
        {
            void luaopen_timer(lua_State* L);
        }
    }
}

#endif // LUA_TIMER_H
