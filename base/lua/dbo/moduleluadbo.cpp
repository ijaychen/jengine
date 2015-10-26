#include "moduleluadbo.h"
#include "lua-dbo.h"
#include "../modulelua.h"
#include <lua.hpp>
#include <iostream>

namespace base
{
    namespace lua
    {
        namespace dbo
        {
            using namespace std;

            ModuleLuaDBO::ModuleLuaDBO()
                : ModuleBase("lua_dbo")
            {
                AddDependentModule("lua");
                AddDependentModule("dbo");
            }

            ModuleLuaDBO::~ModuleLuaDBO()
            {
            }

            ModuleLuaDBO* ModuleLuaDBO::Create()
            {
                ModuleLuaDBO* obj = new ModuleLuaDBO;
                obj->AutoRelease();
                return obj;
            }

            void ModuleLuaDBO::OnModuleSetup()
            {
                lua_State* L = g_module_lua->GetL();
                dbo::luaopen_dbo(L);
                SetModuleState(MODULE_STATE_RUNNING);
            }

            void ModuleLuaDBO::OnModuleCleanup()
            {
                lua_State* L = g_module_lua->GetL();
                dbo::luaclose_dbo(L);
                SetModuleState(MODULE_STATE_DELETE);
            }
        }
    }
}
