#ifndef BASE_DBO_INTERNAL_CONFIGURE_H
#define BASE_DBO_INTERNAL_CONFIGURE_H

#include "../../global.h"
#include <string>
#include <vector>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            // Mysql服务器信息
            struct ServerInfo {
                std::string dbname;             // 数据库名
                std::string username;           // 用户名
                std::string password;           // 密码
                std::string host;               // 主机地址
                int port;                       // 端口
            };

            // 连接池信息
            struct ConnectionPoolInfo {
                uint32_t poolid;                // 池编号
                uint32_t poolsize;              // 连接池大小
                ServerInfo serverinfo;          // 服务器信息
            };

            // 所有Mysql连接池配置信息
            class Configure
            {
            public:
                Configure();

                // 是否为合法的连接池ID
                bool is_valid_poolid(uint32_t poolid) const {
                    return poolid < pools_.size();
                }

                const std::vector<ConnectionPoolInfo>& pools() const {
                    return pools_;
                }

                bool ParseXml();

            private:
                std::vector<ConnectionPoolInfo> pools_;
            };
        }
    }
}

#endif // CONFIGURE_H
