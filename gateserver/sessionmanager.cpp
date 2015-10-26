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
	//����һ��session
	PlayerSession * SessionManager::CreateSession(UserClient* client)
	{
		PlayerSession* session = new PlayerSession ( client );
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
	void SessionManager::DeleteSession(PlayerSession* player)
	{
		players_.erase ( player );
		player->Release();
	}
	//�ر�����session
	void SessionManager::DisconnectAllSession()
	{
		PlayerSession* it = players_.front();
		while ( it ) {
			it->GetResponse().SendLogout ( 2 );
			it->Exit();
			it = it->list_next();
		}
	}
}