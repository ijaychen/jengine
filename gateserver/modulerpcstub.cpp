#include "modulerpcstub.h"
#include <base/global.h>
#include <boost/bind.hpp>
#include "rpc/worldstub.h"
 
namespace fs
{
    using namespace rpc;
    ModuleRpcStub::ModuleRpcStub() : ModuleBase ( "stub" ) , world_ ( nullptr )
    {
        AddDependentModule ( "cluster" );
    }

    ModuleRpcStub::~ModuleRpcStub()
    {
        SAFE_DELETE ( world_ );
    }

    ModuleRpcStub* ModuleRpcStub::Create()
    {
        ModuleRpcStub* obj = new ModuleRpcStub;
        obj->AutoRelease();
        return obj;
    }

    void ModuleRpcStub::OnModuleSetup()
    {
        world_ = new WorldStub();
        world_->BeginSetup ( boost::bind ( &ModuleRpcStub::OnWorldSetup , this ) );
    }

    void ModuleRpcStub::OnModuleCleanup()
    {
        SetModuleState ( base::MODULE_STATE_DELETE );
    }

    void ModuleRpcStub::OnWorldSetup()
    {
        if ( world_->ready() ) {
            SetModuleState ( base::MODULE_STATE_RUNNING );
        } else {
            SetModuleState ( base::MODULE_STATE_DELETE );
        }
    }


}

