#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <base/utils/intrusive_list.h>
#include <base/objectmaintainer.h>
#include <boost/unordered_map.hpp>
#include "playersession.h"

namespace gateserver
{
	class SessionManager
	{
	public:
		static SessionManager * GetInstance();
		//����һ��session
		PlayerSession * CreateSession(base::gateway::UserClient* client);
		void DeleteSession(PlayerSession* player);
		//�ر�����session
		void DisconnectAllSession();
	private:
		SessionManager();
	private:
		base::utils::IntrusiveList<PlayerSession> players_;
	};
	extern SessionManager* g_sessionManager;
}
#endif