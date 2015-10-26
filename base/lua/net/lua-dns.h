#ifndef BASE_LUA_NET_LUA_DNS_H
#define BASE_LUA_NET_LUA_DNS_H

class lua_State;

namespace base
{
    namespace lua
    {
        namespace net
        {
            void luaopen_dns(lua_State* L);
            void luaclose_dns(lua_State* L);
        }
    }
}

#endif
