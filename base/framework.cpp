#include "framework.h"
#include "event/dispatcher.h"
#include "event/eventtimeout.h"
#include "autoreleasepool.h"
#include "logger.h"
#include <csignal>
#include <cassert>
#include <iostream>
#include <clocale>


namespace base
{
    using namespace std;
	static bool is_framework_exist = false;
	extern uint32_t g_object_count;

	static void when_app_exit()
	{
		cout << "== check memory leak ==" << endl;
		cout << "g_object_count: " << g_object_count << endl;
	}

    Framework::Framework()
    {
        assert(!is_framework_exist);
        is_framework_exist = true;       
    }

    Framework::~Framework()
    {
        is_framework_exist = false;
    }
	
	void Framework::CheckExit()
	{
		atexit(when_app_exit);
	}

    int64_t Framework::GetTickCache() const
    {
        return event::Dispatcher::instance().GetTickCache();
    }

  

    void Framework::Cleanup()
    {      
        PoolManager::DeleteInstance();
    }
}
