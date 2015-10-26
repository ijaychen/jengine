#include "moduleluacluster.h"
#include "lua-cluster.h"
#include "../modulelua.h"
#include "../../cluster/nodemonitor.h"
#include <lua.hpp>
#include <boost/bind.hpp>

namespace base
{
    namespace lua
    {
        namespace cluster
        {
            using namespace std;

            ModuleLuaCluster::ModuleLuaCluster()
                : ModuleBase("lua_cluster")
            {
                AddDependentModule("lua");
                AddDependentModule("cluster");
            }

            ModuleLuaCluster::~ModuleLuaCluster()
            {
            }

            ModuleLuaCluster* ModuleLuaCluster::Create()
            {
                ModuleLuaCluster* obj = new ModuleLuaCluster();
                obj->AutoRelease();
                return obj;
            }

            static void on_node_up(lua_State* L, const base::cluster::NodeInfo& node)
            {
                lua_cluster_on_node_up(L, node.name.c_str(), node.id);
            }

            static void on_node_down(lua_State* L, const base::cluster::NodeInfo& node)
            {
                lua_cluster_on_node_down(L, node.name.c_str(), node.id);
            }

            void ModuleLuaCluster::OnModuleSetup()
            {
                lua_State* L = g_module_lua->GetL();

                luaopen_cluster(L);
                EventLinker* linker1 = base::cluster::NodeMonitor::instance().evt_named_mailbox_up.Attach(boost::bind(&lua_cluster_on_named_mailbox_up, L, _1, _2));
                maintainer_.Add(linker1);
                EventLinker* linker2 = base::cluster::NodeMonitor::instance().evt_named_mailbox_down.Attach(boost::bind(&lua_cluster_on_named_mailbox_down, L, _1));
                maintainer_.Add(linker2);
                EventLinker* linker3 = base::cluster::NodeMonitor::instance().evt_node_up.Attach(boost::bind(&on_node_up, L, _1));
                maintainer_.Add(linker3);
                EventLinker* linker4 = base::cluster::NodeMonitor::instance().evt_node_down.Attach(boost::bind(&on_node_down, L, _1));
                maintainer_.Add(linker4);

                SetModuleState(MODULE_STATE_RUNNING);
            }

            void ModuleLuaCluster::OnModuleCleanup()
            {
                lua_State* L = g_module_lua->GetL();

                maintainer_.ClearAll();
                luaclose_cluster(L);

                SetModuleState(MODULE_STATE_DELETE);
            }
        }
    }
}
