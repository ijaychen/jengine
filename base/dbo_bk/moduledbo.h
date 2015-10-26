#ifndef BASE_DBO_MODULEDBO_H
#define BASE_DBO_MODULEDBO_H

#include "../modulebase.h"

namespace base
{
    namespace memory
    {
        class MemoryPool;
    }

    namespace dbo
    {
        class ModuleDBO : public ModuleBase
        {
        public:
            static ModuleDBO* Create();
            virtual ~ModuleDBO();

        private:
            ModuleDBO();
            // modulebase event handler
            virtual void OnModuleSetup();
            virtual void OnModuleCleanup();
            
            void RealConnectionPoolManagerSetupCallback(bool result);
            void RealConnectionPoolManagerCleanupCallback();
        };
    }
}

#endif // MODULEDBO_H
