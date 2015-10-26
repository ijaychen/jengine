#include "lua-event.h"
#include "../helper.h"
#include "../../logger.h"
#include <cstdarg>
#include <lua.hpp>
#include <iostream>

namespace base
{
    namespace lua
    {
        namespace internal
        {
            using namespace std;

            static const char* MT_EVENT = "MT_EVENT_C";
            static const char* MT_EVENT_WEAK = "MT_EVENT_WEAK";

            struct UD_Event {};

            static int event_new(lua_State* L)
            {
                int argc = lua_gettop(L);
                lua_newuserdata(L, sizeof(UD_Event));
                luaL_setmetatable(L, MT_EVENT);
                lua_newtable(L);
                for (int i = 1; i <= argc; ++i) {
                    if (lua_isstring(L, i)) {
                        lua_pushvalue(L, i);
                        lua_newtable(L);
                        luaL_setmetatable(L, MT_EVENT_WEAK);
                        lua_rawset(L, -3);
                    } else {
                        return luaL_argerror(L, i, "string expected");
                    }
                }
                lua_setuservalue(L, -2);
                return 1;
            }

            void lua_event_c_new(lua_State* L, ...)
            {
                const char* name;
                int argc = 0;
                lua_pushcfunction(L, &event_new);
                va_list ap;
                va_start(ap, L);
                while (true) {
                    name = va_arg(ap, const char*);
                    if (!name) {
                        break;
                    }
                    ++argc;
                    lua_pushstring(L, name);
                }
                va_end(ap);
                if (lua_pcall(L, argc, 1, 0)) {
                    LOG_ERROR2("new event error: %s\n", lua_tostring(L, -1));
                    lua_pop(L, 1);
                    lua_pushnil(L);
                }
                lua_setfield(L, -2, "event");
            }

            static int event_meta_attach(lua_State* L)
            {
                if (!lua_isuserdata(L, 1)) {
                    return luaL_argerror(L, 1, "userdata expected");
                }
                if (!lua_isstring(L, 2)) {
                    return luaL_argerror(L, 2, "string expected");
                }
                if (!lua_isfunction(L, 3)) {
                    return luaL_argerror(L, 3, "function expected");
                }

                lua_getuservalue(L, 1);
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
                if (!lua_istable(L, -1)) {
                    return luaL_argerror(L, 2, "not exist event");
                }
                int idx = lua_rawlen(L, -1);
                lua_pushvalue(L, 3);
                lua_rawseti(L, -2, idx + 1);    // append the callback function
                return 0;
            }

            static int event_meta_detach(lua_State* L)
            {
                if (!lua_isuserdata(L, 1)) {
                    return luaL_argerror(L, 1, "userdata expected");
                }
                if (!lua_isstring(L, 2)) {
                    return luaL_argerror(L, 2, "string expected");
                }
                if (!lua_isfunction(L, 3)) {
                    return luaL_argerror(L, 3, "function expected");
                }
                lua_getuservalue(L, 1);
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
                if (!lua_istable(L, -1)) {
                    return luaL_argerror(L, 2, "not exist event");
                }
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_compare(L, -1, 3, LUA_OPEQ)) {
                        lua_pop(L, 1);
                        lua_pushvalue(L, -1);
                        lua_pushnil(L);
                        lua_rawset(L, -4);
                    } else {
                        lua_pop(L, 1);
                    }
                }
                return 0;
            }

            static int event_meta_trigger(lua_State* L)
            {
                if (!lua_isuserdata(L, 1)) {
                    return luaL_argerror(L, 1, "userdata expected");
                }
                if (!lua_isstring(L, 2)) {
                    return luaL_argerror(L, 2, "string expected");
                }
                int argc = lua_gettop(L);
                lua_getuservalue(L, 1);
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
                if (!lua_istable(L, -1)) {
                    return luaL_argerror(L, 2, "not exist event");
                }
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    // copy args
                    for (int i = 3; i <= argc; ++i) {
                        lua_pushvalue(L, i);
                    }
                    if (lua_pcall(L, argc - 2, 1, 0)) {
                        LOG_ERROR2("event trigger exec error: %s\n", lua_tostring(L, -1));
                        lua_pop(L, 1);
                        continue;
                    }
                    if (lua_toboolean(L, -1)) {
                        break;
                    }
                    lua_pop(L, 1);
                }
                return 0;
            }

            void lua_event_c_trigger(lua_State* L, int argc)
            {
                lua_pushcfunction(L, &event_meta_trigger);
                lua_insert(L, - (argc + 1));
                if (lua_pcall(L, argc, 0, 0)) {
                    LOG_ERROR2("event.c trigger exec error: %s\n", lua_tostring(L, -1));
                }
            }

            static int _luaopen_event_c(lua_State* L)
            {
                luaL_newmetatable(L, MT_EVENT_WEAK);
                lua_pushstring(L, "v");
                lua_setfield(L, -2, "__mode");
                lua_pop(L, 1);

                luaL_newmetatable(L, MT_EVENT);
                lua_pushvalue(L, -1);
                luaL_Reg event_meta_funcs[] = {
                    {"attach", event_meta_attach},
                    {"detach", event_meta_detach},
                    {"trigger", event_meta_trigger},
                    {nullptr, nullptr}
                };
                luaL_setfuncs(L, event_meta_funcs, 0);
                lua_setfield(L, -2, "__index");
                lua_pop(L, 1);

                luaL_Reg event_c_funcs[] = {
                    {"new", event_new},
                    {nullptr, nullptr}
                };
                luaL_newlib(L, event_c_funcs);
                return 1;
            }

            void luaopen_event_c(lua_State* L)
            {
                luaL_requiref(L, "event.c", &_luaopen_event_c, 0);
                lua_pop(L, 1);
            }
        }
    }
}
