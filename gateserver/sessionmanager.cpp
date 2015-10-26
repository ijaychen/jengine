#include "sessionmanager.h"
#include "response.h"
#include <base/logger.h>

namespace gateserver
{
	using namespace base;
	using namespace base::gateway;

	SessionManager* g_sessionManager = SessionManager::GetInstance();
	SessionManager * SessionManager::GetInstance()
	{
		static SessionManager _instance;
		return &_instance;
	}

	SessionManager::SessionManager()
	{  
		
	}
	//创建一个session
	GateSession * SessionManager::CreateSession(UserClient* client)
	{
		GateSession* session = new GateSession ( client );
		if(nullptr == session)
		{
			LOG_ERROR ( "SessionManager:session init failed!\n" );
			return nullptr;
		}
		if ( session->Setup() ) {
			players_.push_front ( session );
			return session;
		} else {
			session->Release();
			LOG_ERROR ( "SessionManager:player init failed!\n" );
			return nullptr;
		}
	}
	void SessionManager::DeleteSession(GateSession* player)
	{
		players_.erase ( player );
		player->Release();
	}
	//关闭所有session
	void SessionManager::DisconnectAllSession()
	{
		GateSession* it = players_.front();
		while ( it ) {
			it->GetResponse().SendLogout ( 2 );
			it->Exit();
			it = it->list_next();
		}
	}
}