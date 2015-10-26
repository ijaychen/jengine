#ifndef GATE_ENGINE_H
#define GATE_ENGINE_H

#include <base/global.h>
namespace gateserver
{
	class GateEngine
	{
	public:
		void LogicRun(int64_t tick);
	};
}

#endif // GATE_ENGINE_H
