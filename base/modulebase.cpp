#include "modulebase.h"
#include "logger.h"

namespace base
{
    ModuleBase::ModuleBase(const char* module_name)
        : module_state_(MODULE_STATE_NEW), module_name_(module_name), bootstrap_order_(0)
    {
    }

    ModuleBase::~ModuleBase()
    {
    }

    void ModuleBase::SetModuleState(ModuleState state)
    {
        // precheck old state is valid with new state
        switch (module_state_) {
            case MODULE_STATE_NEW:
                debug_assert(state == MODULE_STATE_DELETE || state == MODULE_STATE_IN_SETUP || state == MODULE_STATE_RUNNING);
                break;
            case MODULE_STATE_IN_SETUP:
                debug_assert(state == MODULE_STATE_DELETE || state == MODULE_STATE_RUNNING);
                break;
            case MODULE_STATE_RUNNING:
                debug_assert(state == MODULE_STATE_DELETE || state == MODULE_STATE_IN_CLEANUP);
                break;
            case MODULE_STATE_IN_CLEANUP:
                debug_assert(state == MODULE_STATE_DELETE);
                break;
            case MODULE_STATE_DELETE:
                debug_assert(!"invalid state");
                break;
        }
        module_state_ = state;
    }
}
