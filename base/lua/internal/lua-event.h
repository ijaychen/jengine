#ifndef BASE_LUA_INTERNAL_LUA_EVENT_H
#define BASE_LUA_INTERNAL_LUA_EVENT_H

class lua_State;

namespace base
{
    namespace lua
    {
        namespace internal
        {
            void luaopen_event_c(lua_State* L);
            // ...以nullptr结尾的事件名列表
            void lua_event_c_new(lua_State* L, ...);
            void lua_event_c_trigger(lua_State* L, int argc);
        }
    }
}

#endif
