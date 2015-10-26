#include "componentbase.h"
#include <iostream>

namespace gateserver
{
    namespace comp
    {
        using namespace std;

        ComponentBase::~ComponentBase()
        {
        }

        void ComponentBase::NotifySetupFinish()
        {
            state_ = OK;
            OnSetupFinish();
            ps().OnCompSetup(this);
        }

        void ComponentBase::NotifyCleanupFinish()
        {
            state_ = CLEANED;
            OnCleanupFinish();
            ps().OnCompCleanup(this);
        }
    }
}
