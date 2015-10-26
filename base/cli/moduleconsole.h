#ifndef BASE_CLI_MODULECONSOLE_H
#define BASE_CLI_MODULECONSOLE_H

#include "../modulebase.h"

namespace base
{
    namespace cli
    {
        class ModuleConsole : public ModuleBase
        {
        public:
            static ModuleConsole* Create();
            virtual ~ModuleConsole();
            
        private:
            ModuleConsole();
            virtual void OnModuleSetup();
            virtual void OnModuleCleanup();
        };
    }
}

#endif // MODULECONSOLE_H
