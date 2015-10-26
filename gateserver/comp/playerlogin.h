#ifndef PLAYER_LOGIN_H
#define PLAYER_LOGIN_H

#include "componentbase.h"

namespace gateserver
{
	namespace comp
	{
		class PlayerLogin : public ComponentBase
		{
		public:
			PlayerLogin(PlayerSession & ps) : ComponentBase(ps, "PlayerLogin"){}
			~PlayerLogin(){}
		private:
			virtual void BeginSetup();
			virtual void BeginCleanup();
		};
	}
}

#endif