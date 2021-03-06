#include "gateserver.h"
#include "base/event/dispatcher.h"
#include "base/event/eventtimeout.h"
#include "base/autoreleasepool.h"
#include "base/logger.h"
#include <csignal>
#include <cassert>
#include <iostream>
#include <clocale>

gateserver::Gateserver g_gateserver;

namespace gateserver
{
    using namespace std;
	using namespace base;
   

    static void signal_handler(int sig)
    {
        if (sig == SIGINT) {
            g_gateserver.Stop();
        }
    }

    Gateserver::Gateserver()
    {
        signal(SIGINT, signal_handler);
    }

    Gateserver::~Gateserver()
    {
    }
	void Gateserver::Initialize(const string& resource_dir, const string& priv_dir)
	{
		LOG_WARN("****************************************\n");
		LOG_WARN("     application start running ...\n");
		LOG_WARN("****************************************\n");
		LOG_WARN("res_dir:  %s\n", resource_dir.c_str());
		LOG_WARN("priv_dir: %s\n", priv_dir.c_str());
		LOG_WARN("\n");

		resource_dir_ = resource_dir;
		priv_dir_ = priv_dir;

		// 设置环境
		setlocale(LC_ALL, "en_US.utf8");
		// 设定时区
		tzset();
		PoolManager::CreateInstance();
		CheckExit();
		random_.Setup();
	}

    bool Gateserver::Setup(const string& resource_dir, const string& priv_dir)
    {        
		Initialize(resource_dir, priv_dir);
		gatewaymanager.Setup("127.0.0.1", 5526, 300);
		return true;
    }


	int Gateserver::Run(boost::function<void(int64_t)>* loop)
    {
        event::Dispatcher::instance().Dispatch(loop);
        Cleanup();
        return 0;
    }

    void Gateserver::Cleanup()
    {      
        PoolManager::DeleteInstance();
    }

    void Gateserver::Stop()
    {
		LOG_WARN("     application stop running ...\n");
		gatewaymanager.Stop();

		//atlast
		event::Dispatcher::instance().NormalExit();		
    }
}
