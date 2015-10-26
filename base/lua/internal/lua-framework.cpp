#include "lua-framework.h"
#include "lua-event.h"
#include "../helper.h"
#include "../../framework.h"
#include "../../modulebase.h"

namespace base
{
    namespace lua
    {
        namespace internal
        {
            using namespace std;

            static int get_all_modules(lua_State* L)
            {
                vector<const ModuleBase*> modules = framework.GetAllModules();
                lua_newtable(L);
                int i = 0;
                for (vector<const ModuleBase*>::iterator it = modules.begin();
                        it != modules.end(); ++it) {
                    lua_pushstring(L, (*it)->module_name().c_str());
                    lua_rawseti(L, -2, ++i);
                }
                return 1;
            }

            static int _luaopen_framework(lua_State* L)
            {
                luaL_Reg framework_funcs[] = {
                    {"get_all_modules", get_all_modules},
                    {nullptr, nullptr}
                };
                luaL_newlib(L, framework_funcs);
                internal::lua_event_c_new(L, "before_shutdown", nullptr);
                return 1;
            }
            
            void lua_framework_on_before_shutdown(lua_State* L)
            {
                lua_getglobal(L, "package");
                lua_getfield(L, -1, "loaded");
                lua_getfield(L, -1, "framework");
                lua_getfield(L, -1, "event");
                lua_pushstring(L, "before_shutdown");
                internal::lua_event_c_trigger(L, 2);
            }

            void luaopen_framework(lua_State* L)
            {
                luaL_requiref(L, "framework", &_luaopen_framework, 0);
                lua_pop(L, 1);
            }
        }
    }
}
