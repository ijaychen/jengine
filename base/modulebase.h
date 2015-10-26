#ifndef BASE_MODULEBASE_H
#define BASE_MODULEBASE_H

#include "object.h"
#include <string>
#include <vector>

namespace base
{
    enum ModuleState {
        // 新创建的模块
        MODULE_STATE_NEW,
        // 正在启动中
        MODULE_STATE_IN_SETUP,
        // 模块正常运行中
        MODULE_STATE_RUNNING,
        // 正在停止中
        MODULE_STATE_IN_CLEANUP,
        // 模块已停止
        MODULE_STATE_DELETE,
    };

    class Framework;

    class ModuleBase : public Object
    {
    public:
        ModuleBase(const char* module_name);
        virtual ~ModuleBase();

        // 模块名
        const std::string& module_name() const {
            return module_name_;
        }
        // 模块状态
        ModuleState module_state() const {
            return module_state_;
        }
        // 设置模块状态
        void SetModuleState(ModuleState state) ;
        // 获得前置模块列表
        const std::vector<std::string>& GetDependentsModules() const {
            return dependents_;
        }
        // 模块启动
        virtual void OnModuleSetup() = 0;
        // 模块停止
        virtual void OnModuleCleanup() = 0;

    protected:
        void AddDependentModule(const char* mod) {
            dependents_.push_back(mod);
        }
    private:
        ModuleState module_state_;
        std::string module_name_;
        // 依赖的模块
        std::vector<std::string> dependents_;
        uint32_t bootstrap_order_;
        friend class ActionBootstrap;
        friend class ActionShutdown;
    };
}

#endif // MODULEBASE_H
