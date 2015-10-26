#ifndef BASE_LUA_HELPER_H
#define BASE_LUA_HELPER_H

#include "../global.h"
#include <lua.hpp>
#include <string>

class lua_State;

namespace base
{
    namespace lua
    {
        void lua_debug_dump_stack(lua_State* L);

        void lua_append_package_path(lua_State* L, const std::string& path);

        void lua_append_package_cpath(lua_State* L, const std::string& cpath);

        // 自动清空lua stack
        class AutoCleanupStack
        {
        public:
            AutoCleanupStack(lua_State* _L)
                : L(_L) {
            }
            ~AutoCleanupStack() {
                lua_pop(L, lua_gettop(L));
            }
        private:
            lua_State* L;
            DISABLE_COPY(AutoCleanupStack)
        };
    }
}

#endif
