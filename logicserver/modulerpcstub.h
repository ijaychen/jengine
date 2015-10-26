#ifndef MODULERPCSTUB_H
#define MODULERPCSTUB_H

#include <base/modulebase.h>
namespace fs
{
    namespace rpc
    {
        class WorldStub;
        class MailStub;
        class FriendStub;
    }

    class ModuleRpcStub : public base::ModuleBase
    {
    public:
        static ModuleRpcStub* Create();
        virtual ~ModuleRpcStub();
        void OnWorldSetup();
        void OnWorldChatSetup();
    private:
        ModuleRpcStub();
        virtual void OnModuleSetup();
        virtual void OnModuleCleanup();
    private:
        rpc::WorldStub* world_;
        rpc::MailStub* mail_;
        rpc::FriendStub* friend_;

    };
}
#endif // MODULERPCSTUB_H
