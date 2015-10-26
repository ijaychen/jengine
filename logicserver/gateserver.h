#ifndef BASE_SERVER_H
#define BASE_SERVER_H
#include <base/global.h>
#include <base/framework.h>
#include "gatewaymanager.h"

namespace gateserver
{
	using namespace base;
	class Gateserver : public Framework
	{
	public:
        Gateserver();
        ~Gateserver();

        // 启动
        bool Setup(const std::string& resource_dir, const std::string& priv_dir);
        // 运行
		int Run(boost::function<void(int64_t)>* loop = nullptr);
        // 停止
        void Stop();

		void Initialize(const std::string& resource_dir, const std::string& priv_dir);
    private:
        void Cleanup();       
        utils::Random random_;
        std::string resource_dir_;
        std::string priv_dir_;

	private:
		GatewayManager gatewaymanager;
    };
}

extern gateserver::Gateserver g_gateserver;

#endif // BASE_SERVER_H
