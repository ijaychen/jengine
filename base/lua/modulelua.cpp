#include "modulelua.h"
#include "../framework.h"
#include "../file/utils.h"
#include "../logger.h"
#include "../action/executor.h"
#include "helper.h"
#include "internal/lua-framework.h"
#include "internal/lua-timer.h"
#include "internal/lua-event.h"
#include <lua.hpp>
#include <iostream>
#include <boost/bind/bind.hpp>

base::lua::ModuleLua* g_module_lua = nullptr;

namespace base
{
    namespace lua
    {
        using namespace std;

        ModuleLua::ModuleLua()
            : ModuleBase("lua"), L(nullptr), exe_(nullptr)
        {
            assert(g_module_lua == nullptr);
            g_module_lua = this;
        }

        ModuleLua::~ModuleLua()
        {
            SAFE_RELEASE(exe_);
            if (L) {
                lua_close(L);
                L = nullptr;
            }
            g_module_lua = nullptr;
        }

        ModuleLua* ModuleLua::Create()
        {
            ModuleLua* obj = new ModuleLua;
            obj->AutoRelease();
            return obj;
        }

        void ModuleLua::OnModuleSetup()
        {
            exe_ = new action::Executor();

            L = luaL_newstate();

            // 打开库
            luaL_openlibs(L);
            internal::luaopen_event_c(L);
            internal::luaopen_timer(L);
            internal::luaopen_framework(L);
            
            EventLinker* linker = framework.evt_before_shutdown.Attach(boost::bind(&internal::lua_framework_on_before_shutdown, L));
            maintainer_.Add(linker);
            
            lua_append_package_path(L, framework.resource_dir() + "/lualib");
            lua_append_package_cpath(L, framework.resource_dir() + "/luaclib");
            lua_append_package_path(L, framework.priv_dir() + "/lualib");
            lua_append_package_cpath(L, framework.priv_dir() + "/luaclib");

            string bootstrap0 = framework.resource_dir() + "/lualib/bootstrap.lua";
            if (base::file::file_can_read(bootstrap0.c_str())) {
                int err = luaL_loadfile(L, bootstrap0.c_str()) || lua_pcall(L, 0, 0, 0);
                if (err) {
                    LOG_WARN("[%s] exec fail: %s\n", bootstrap0.c_str(), lua_tostring(L, -1));
                }
            }

            string bootstrap1 = framework.priv_dir() + "/lualib/bootstrap.lua";
            if (base::file::file_can_read(bootstrap1.c_str())) {
                int err = luaL_loadfile(L, bootstrap1.c_str()) || lua_pcall(L, 0, 0, 0);
                if (err) {
                    LOG_WARN("[%s] exec fail: %s\n", bootstrap1.c_str(), lua_tostring(L, -1));
                }
            }

            SetModuleState(MODULE_STATE_RUNNING);
        }

        void ModuleLua::OnModuleCleanup()
        {
            maintainer_.ClearAll();
            exe_->StopAllAction();
            if (L) {
                lua_close(L);
                L = nullptr;
            }
            SAFE_RELEASE(exe_);
            SetModuleState(MODULE_STATE_DELETE);
        }
        
        void ModuleLua::AppendPackagePath(const string& path)
        {
            lua_append_package_path(L, path);
        }
        
        bool ModuleLua::ExecuteScript(const char* script)
        {
            AutoCleanupStack ac(L);
            if (base::file::file_can_read(script)) {
                int err = luaL_loadfile(L, script) || lua_pcall(L, 0, 0, 0);
                if (err) {
                    LOG_WARN("[%s] exec fail: %s\n", script, lua_tostring(L, -1));
                    return false;
                } else {
                    return true;
                }
            }
            LOG_WARN("can not execute not exist script: %s\n", script);
            return false;
        }
    }
}
