#ifndef BASE_CLUSTER_MODULECLUSTER_H
#define BASE_CLUSTER_MODULECLUSTER_H

#include "../modulebase.h"

namespace base
{
    namespace cluster
    {
        class ModuleCluster : public ModuleBase
        {
        public:
            virtual ~ModuleCluster();
            
            static ModuleCluster* Create();

        private:
            ModuleCluster();
            virtual void OnModuleSetup();
            virtual void OnModuleCleanup();
            
            void NodeMonitorSetupCallback(bool result);
            void NodeMonitorCleanupCallback();

        };
    }
}

#endif // MODULECLUSTER_H
