#ifndef BASE_ENGINE_H
#define BASE_ENGINE_H

#include "global.h"
#include "utils/random.h"
#include "event.h"
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>

namespace base
{
	class Framework
	{
	public:
        Framework();
        ~Framework();

       

        // 随机数生成
        utils::Random& random() {
            return random_;
        }
        // 资源目录
        const std::string& resource_dir() const {
            return resource_dir_;
        }
        // 私有目录
        const std::string& priv_dir() const {
            return priv_dir_;
        }
        // 获取当前时间(高效)
        int64_t GetTickCache() const;        

        // 启动
        virtual bool Setup(const std::string& resource_dir, const std::string& priv_dir) = 0;
        // 运行
		virtual int Run(boost::function<void(int64_t)>* loop = nullptr) = 0;
        // 停止
        virtual void Stop() = 0;

		void CheckExit();
    private:
        void Cleanup();       
        utils::Random random_;
        std::string resource_dir_;
        std::string priv_dir_;
    };
}

#endif // BASE_ENGINE_H
