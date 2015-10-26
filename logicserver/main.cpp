#include <iostream>
#include <string>
#include "base/utils/string.h"
#include "base/utils/configure.h"
#include "gateserver.h"
//#include "gatewaymanager.h"

using namespace std;
using namespace gateserver;

class GateEngine
{
public:
	virtual void LogicRun(int64_t tick)
	{
		LOG_WARN("====>>>>LogicRun,tick:%d", tick);
		sleep(1);
	}
};

void ShowTime(int64_t tick)
{
	cout << "showTime:" << tick << endl;
}

int main ( int argc, char **argv )
{   
	GateEngine gateEngine;
	//GatewayManager gatewayManager;
	//gatewayManager.Setup("127.0.0.1", 5526, 300);
	std::string ip = base::utils::GetConfigByName<std::string>("./gateserver.conf", "gateserver.localservice.ip");
	cout << ip << endl;
	uint32_t port = base::utils::GetConfigByName<uint32_t>("./gateserver.conf", "gateserver.localservice.port");
	cout << port << endl;
	cout << base::utils::hash_string("test") << endl;
	if ( !g_gateserver.Setup ("./", "./") ) {
		return 1;
	}
	typedef boost::function<void (int64_t)> Callback;  
	Callback loop = boost::bind(&GateEngine::LogicRun, gateEngine, _1);
	//Callback loop = boost::bind(ShowTime, _1);
	 //boost::function<void(GateEngine*, int)> loop; 
	 //loop = &GateEngine::LogicRun;
	//执行框架	
	return g_gateserver.Run(&loop);
  
}
