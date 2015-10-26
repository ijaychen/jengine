#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <base/utils/intrusive_list.h>
#include <base/objectmaintainer.h>
#include <boost/unordered_map.hpp>
#include "gatesession.h"

namespace gateserver
{
	class SessionManager
	{
	public:
		static SessionManager * GetInstance();
		//创建一个session
		GateSession * CreateSession(base::gateway::UserClient* client);
		void DeleteSession(GateSession* player);
		//关闭所有session
		void DisconnectAllSession();
	private:
		SessionManager();
	private:
		base::utils::IntrusiveList<GateSession> players_;
	};
	extern SessionManager* g_sessionManager;
}
#endif