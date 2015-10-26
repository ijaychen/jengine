#include "modulecluster.h"
#include "nodemonitor.h"
#include <boost/bind.hpp>

namespace base
{
    namespace cluster
    {
        ModuleCluster::ModuleCluster()
            : ModuleBase("cluster")
        {
        }

        ModuleCluster::~ModuleCluster()
        {
        }

        ModuleCluster* ModuleCluster::Create()
        {
            ModuleCluster* obj = new ModuleCluster();
            obj->AutoRelease();
            return obj;
        }

        void ModuleCluster::OnModuleSetup()
        {
            SetModuleState(MODULE_STATE_IN_SETUP);
            NodeMonitor::instance().BeginSetup(boost::bind(&ModuleCluster::NodeMonitorSetupCallback, this, _1));
        }

        void ModuleCluster::OnModuleCleanup()
        {
            SetModuleState(MODULE_STATE_IN_CLEANUP);
            NodeMonitor::instance().BeginCleanup(boost::bind(&ModuleCluster::NodeMonitorCleanupCallback, this));
        }

        void ModuleCluster::NodeMonitorSetupCallback(bool result)
        {
            if (result) {
                SetModuleState(MODULE_STATE_RUNNING);
            } else {
                SetModuleState(MODULE_STATE_DELETE);
            }
        }

        void ModuleCluster::NodeMonitorCleanupCallback()
        {
            SetModuleState(MODULE_STATE_DELETE);
        }
    }
}

