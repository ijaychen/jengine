#ifndef BASE_LUA_DBO_MODULELUADBO_H
#define BASE_LUA_DBO_MODULELUADBO_H

#include "../../modulebase.h"

namespace base
{
    namespace lua
    {
        namespace dbo
        {
            class ModuleLuaDBO : public ModuleBase
            {
            public:
                virtual ~ModuleLuaDBO();
                
                static ModuleLuaDBO* Create();
                
            private:
                ModuleLuaDBO();
                virtual void OnModuleSetup();
                virtual void OnModuleCleanup();
            };
        }
    }
}

#endif // MODULELUADBO_H
