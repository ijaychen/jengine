#ifndef BASE_LUA_CLUSTER_LUA_CLUSTER_H
#define BASE_LUA_CLUSTER_LUA_CLUSTER_H

#include "../../global.h"

class lua_State;

namespace base
{
    namespace lua
    {
        namespace cluster
        {
            void luaopen_cluster(lua_State* L);
            void luaclose_cluster(lua_State* L);
            
            void lua_cluster_on_named_mailbox_up(lua_State* L, const char* name, uint32_t mbid);
            void lua_cluster_on_named_mailbox_down(lua_State* L, const char* name);
            void lua_cluster_on_node_up(lua_State* L, const char* name, uint32_t nodeid);
            void lua_cluster_on_node_down(lua_State* L, const char* name, uint32_t nodeid);
        }
    }
}

#endif
