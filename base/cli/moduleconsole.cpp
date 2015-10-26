#include "moduleconsole.h"
#include <iostream>

namespace base
{
    namespace cli
    {
        using namespace std;

        ModuleConsole::ModuleConsole()
            : ModuleBase("console")
        {
        }

        ModuleConsole::~ModuleConsole()
        {
        }

        ModuleConsole* ModuleConsole::Create()
        {
            ModuleConsole* obj = new ModuleConsole();
            obj->AutoRelease();
            return obj;
        }

        void ModuleConsole::OnModuleSetup()
        {
            cout << "<module> console setup.." << endl;
            SetModuleState(MODULE_STATE_RUNNING);
        }

        void ModuleConsole::OnModuleCleanup()
        {
            cout << "<module> console cleanup.." << endl;
            SetModuleState(MODULE_STATE_DELETE);
        }
    }
}
