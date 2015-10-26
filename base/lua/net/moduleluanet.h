#ifndef BASE_LUA_NET_MODULELUANET_H
#define BASE_LUA_NET_MODULELUANET_H

#include "../../modulebase.h"

namespace base
{
    namespace lua
    {
        namespace net
        {
            class ModuleLuaNet : public ModuleBase
            {
            public:
                virtual ~ModuleLuaNet();
                static ModuleLuaNet* Create();
                
            private:
                ModuleLuaNet();
                virtual void OnModuleSetup();
                virtual void OnModuleCleanup();

            private:
                
            };
        }
    }
}

#endif // MODULELUANET_H
