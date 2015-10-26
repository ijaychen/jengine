#ifndef BASE_LUA_INTERNAL_LUA_FRAMEWORK_H
#define BASE_LUA_INTERNAL_LUA_FRAMEWORK_H

class lua_State;

namespace base
{
    namespace lua
    {
        namespace internal
        {
            void luaopen_framework(lua_State* L);
            
            void lua_framework_on_before_shutdown(lua_State* L);
        }
    }
}

#endif
