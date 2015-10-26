#include "helper.h"
#include <iostream>

namespace base
{
    namespace lua
    {
        using namespace std;

        void lua_append_package_path(lua_State* L, const std::string& path)
        {
            lua_getglobal(L, "package");
            lua_getfield(L, -1, "path");
            luaL_Buffer b;
            luaL_buffinit(L, &b);
            luaL_addstring(&b, path.c_str());
            luaL_addstring(&b, "/?.lua;");
            luaL_addvalue(&b);
            luaL_pushresult(&b);
            lua_setfield(L, -2, "path");
            lua_pop(L, 1);
        }

        void lua_append_package_cpath(lua_State* L, const string& cpath)
        {
            lua_getglobal(L, "package");
            lua_getfield(L, -1, "cpath");
            luaL_Buffer b;
            luaL_buffinit(L, &b);
            luaL_addstring(&b, cpath.c_str());
            luaL_addstring(&b, "/?.so;");
            luaL_addvalue(&b);
            luaL_pushresult(&b);
            lua_setfield(L, -2, "cpath");
            lua_pop(L, 1);
        }

        void lua_debug_dump_stack(lua_State* L)
        {
            int i;
            int r = -1;
            int top = lua_gettop(L);
            cout << "===== lua_debug_dump_stack =====" << endl;
            for (i = top; i > 0; i--) {
                int type = lua_type(L, i);
                switch (type) {
                    case LUA_TNUMBER:
                        printf("%d(%d) = number %g\n", i, r, lua_tonumber(L, i));
                        break;
                    case LUA_TSTRING:
                        printf("%d(%d) = string %s\n", i, r, lua_tostring(L, i));
                        break;
                    default:
                        printf("%d(%d) = %s\n", i, r, lua_typename(L, type));
                        break;
                }
                r--;
            }
            cout << "=================================" << endl;
        }
    }
}
