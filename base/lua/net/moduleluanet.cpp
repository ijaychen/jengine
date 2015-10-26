#include "moduleluanet.h"
#include "../modulelua.h"
#include "lua-dns.h"

namespace base
{
    namespace lua
    {
        namespace net
        {
            ModuleLuaNet::ModuleLuaNet()
                : ModuleBase("lua_net")
            {
                AddDependentModule("lua");
            }

            ModuleLuaNet::~ModuleLuaNet()
            {
            }

            ModuleLuaNet* ModuleLuaNet::Create()
            {
                ModuleLuaNet* obj = new ModuleLuaNet;
                obj->AutoRelease();;
                return obj;
            }

            void ModuleLuaNet::OnModuleSetup()
            {
                lua_State* L = g_module_lua->GetL();
                luaopen_dns(L);
                SetModuleState(MODULE_STATE_RUNNING);
            }

            void ModuleLuaNet::OnModuleCleanup()
            {
                lua_State* L = g_module_lua->GetL();
                luaclose_dns(L);
                SetModuleState(MODULE_STATE_DELETE);
            }
        }
    }
}
