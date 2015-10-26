#include "moduledbo.h"
#include "../memory/memorypool.h"
#include "../logger.h"
#include "internal/realconnectionpool.h"
#include <boost/bind.hpp>

namespace base
{
    namespace dbo
    {
        using namespace std;

        ModuleDBO::ModuleDBO()
            : ModuleBase("dbo")
        {
        }

        ModuleDBO::~ModuleDBO()
        {
        }

        ModuleDBO* ModuleDBO::Create()
        {
            ModuleDBO* ptr = new ModuleDBO();
            ptr->AutoRelease();
            return ptr;
        }

        void ModuleDBO::OnModuleSetup()
        {
            SetModuleState(MODULE_STATE_IN_SETUP);
            internal::RealConnectionPoolManager::instance().BeginSetup(boost::bind(&ModuleDBO::RealConnectionPoolManagerSetupCallback, this, _1));
        }

        void ModuleDBO::OnModuleCleanup()
        {
            SetModuleState(MODULE_STATE_IN_CLEANUP);
            internal::RealConnectionPoolManager::instance().BeginCleanup(boost::bind(&ModuleDBO::RealConnectionPoolManagerCleanupCallback, this));
        }

        void ModuleDBO::RealConnectionPoolManagerSetupCallback(bool result)
        {
            if (result) {
                SetModuleState(MODULE_STATE_RUNNING);
            } else {
                LOG_ERROR("<module>.dbo setup fail..\n");
                SetModuleState(MODULE_STATE_DELETE);
            }
        }

        void ModuleDBO::RealConnectionPoolManagerCleanupCallback()
        {
            SetModuleState(MODULE_STATE_DELETE);
        }
    }
}
