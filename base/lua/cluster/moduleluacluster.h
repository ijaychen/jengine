#ifndef BASE_LUA_CLUSTER_MODULELUACLUSTER_H
#define BASE_LUA_CLUSTER_MODULELUACLUSTER_H

#include "../../modulebase.h"
#include "../../objectmaintainer.h"

namespace base
{
    namespace lua
    {
        namespace cluster
        {
            class ModuleLuaCluster : public ModuleBase
            {
            public:
                virtual ~ModuleLuaCluster();
                
                static ModuleLuaCluster* Create();

            private:
                ModuleLuaCluster();
                virtual void OnModuleSetup();
                virtual void OnModuleCleanup();
                
                ObjectMaintainer maintainer_;
            };
        }
    }
}

#endif // MODULELUACLUSTER_H
