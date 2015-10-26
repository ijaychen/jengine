#ifndef BASE_LUA_MODULELUA_H
#define BASE_LUA_MODULELUA_H

#include "../modulebase.h"
#include "../objectmaintainer.h"

class lua_State;

namespace base
{
    namespace action
    {
        class Executor;
    }

    namespace lua
    {
        class ModuleLua : public ModuleBase
        {
        public:
            virtual ~ModuleLua();
            static ModuleLua* Create();

            action::Executor* executor() {
                return exe_;
            }
            lua_State* GetL() {
                return L;
            }

            // 添加lua包路径
            void AppendPackagePath(const std::string& path);

            // 执行一个脚本，发生错误时返回false, 正常时返回true
            bool ExecuteScript(const char* script);
            bool ExecuteScript(const std::string& script) {
                return ExecuteScript(script.c_str());
            }

        private:
            ModuleLua();
            virtual void OnModuleSetup();
            virtual void OnModuleCleanup();

        private:
            lua_State* L;
            action::Executor* exe_;
            ObjectMaintainer maintainer_;
        };
    }
}

extern base::lua::ModuleLua* g_module_lua;

#endif // MODULELUA_H
