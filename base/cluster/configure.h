#ifndef BASE_CLUSTER_CONFIGURE_H
#define BASE_CLUSTER_CONFIGURE_H

#include "../global.h"
#include <string>
#include <vector>

namespace base
{
    namespace cluster
    {
        // 节点信息
        struct NodeInfo {
            uint8_t id;
            std::string name;
            std::string type;
            struct Listener {
                std::string ip;
                int32_t port;
                Listener() : ip(""), port(-1) {}
            };
            Listener listener;
            struct LinkInfo {
                enum LinkType {
                    BY_NAME,
                    BY_TYPE,
                };
                LinkType type;
                std::string matcher;
                bool require;
                LinkInfo(LinkType t, const std::string& m, bool r) : type(t), matcher(m), require(r) {}
            };
            std::vector<LinkInfo> links;

            // 是否有侦听器
            bool HasListener() const {
                return !listener.ip.empty() && listener.port != -1;
            }
        };

        struct LinkNodeInfo {
            LinkNodeInfo(const NodeInfo& n, bool r) : node_(&n), require_(r) {}
            const NodeInfo& node() const {
                return *node_;
            }
            bool require() const {
                return require_;
            }
        private:
            const NodeInfo* node_;
            bool require_;
        };

        struct LocalNodeInfo {
            DISABLE_COPY(LocalNodeInfo)
            const NodeInfo* info;
            // 允许的cookie列表
            std::vector<std::string> cookies;
            // 指定cookie是否在允许列表中
            bool IsCookieAllow(const std::string& ck) const {
                for (std::vector<std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it) {
                    if (ck == *it) {
                        return true;
                    }
                }
                return false;
            }
            LocalNodeInfo() : info(nullptr) {}
        };

        // 集群配置读取器
        class Configure
        {
        public:
            Configure();

            // 当前节点信息
            const LocalNodeInfo& local_node() const {
                return local_node_;
            }
            // 所有节点
            const std::vector<NodeInfo>& nodes() const {
                return nodes_;
            }

            // 根据节点ID获取节点信息
            const NodeInfo* FindNodeInfoByID(uint8_t id) const {
                for (std::vector<NodeInfo>::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
                    if ((*it).id == id) {
                        return &(*it);
                    }
                }
                return nullptr;
            }
            // 获取联接节点列表
            std::vector<LinkNodeInfo> FetchLinkNodeList();
            // 解析配置文件
            bool ParseXml(const std::string& resource_dir, const std::string& priv_dir);

        private:
            LocalNodeInfo local_node_;
            std::vector<NodeInfo> nodes_;
        };
    }
}

#endif // CONFIGURE_H
