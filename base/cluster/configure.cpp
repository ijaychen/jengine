#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "configure.h"
#include "../logger.h"
#include <algorithm>

namespace base
{
    namespace cluster
    {
        using namespace std;
        using namespace boost::property_tree;

        /// 配置
        Configure::Configure()
        {
        }

        bool Configure::ParseXml(const std::string& resource_dir, const std::string& priv_dir)
        {
            try {
                {
                    // 读取所有的集群节点信息
                    ptree xmldoc;
                    read_xml(resource_dir + "/cluster.conf", xmldoc, xml_parser::no_comments);
                    ptree& cluster = xmldoc.get_child("framework.cluster");
                    for (ptree::iterator it = cluster.begin(); it != cluster.end(); ++it) {
                        if (it->first != "node") {
                            continue;
                        }
                        NodeInfo node;
                        node.id = it->second.get_child("<xmlattr>.id").get_value<uint8_t>();
                        node.name = it->second.get_child("<xmlattr>.name").get_value<string>();
                        node.type = it->second.get_child("<xmlattr>.type").get_value<string>();
                        for (ptree::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                            if (it2->first == "listener") {
                                node.listener.ip = it2->second.get_child("<xmlattr>.ip").get_value<string>();
                                node.listener.port = it2->second.get_child("<xmlattr>.port").get_value<int32_t>();
                            } else if (it2->first == "link_by_name") {
                                string m = it2->second.get_value<string>();
                                bool r = it2->second.get_child("<xmlattr>.required").get_value<bool>();
                                node.links.push_back(NodeInfo::LinkInfo(NodeInfo::LinkInfo::BY_NAME, m, r));
                            } else if (it2->first == "link_by_type") {
                                string m = it2->second.get_value<string>();
                                bool r = it2->second.get_child("<xmlattr>.required").get_value<bool>();
                                node.links.push_back(NodeInfo::LinkInfo(NodeInfo::LinkInfo::BY_TYPE, m, r));
                            }
                        }
                        nodes_.push_back(node);
                    }
                }

                {
                    // 读取本地节点信息
                    ptree xmldoc;
                    read_xml(priv_dir + "/node.conf", xmldoc, xml_parser::no_comments);
                    string local_node_name = xmldoc.get_child("framework.cluster.localnode.<xmlattr>.name").get_value<string>();
                    ptree& cookies_node = xmldoc.get_child("framework.cluster.localnode");
                    for (ptree::iterator it = cookies_node.begin(); it != cookies_node.end(); ++ it) {
                        string cookie = it->second.get_value<string>();
                        local_node_.cookies.push_back(cookie);
                    }
                    for (vector<NodeInfo>::iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
                        if ((*it).name == local_node_name) {
                            local_node_.info = &(*it);
                            break;
                        }
                    }
                }

                if (!local_node_.info) {
                    LOG_ERROR("not found local node info in cluster.conf\n");
                    return false;
                } else {
                    return true;
                }
            } catch (exception& ex) {
                LOG_ERROR("parse cluster configure fail:%s\n", ex.what());
                return false;
            }
        }

        vector<LinkNodeInfo> Configure::FetchLinkNodeList()
        {
            vector<LinkNodeInfo> list;

            for (vector<NodeInfo>::iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
                if ((*it).name == local_node_.info->name) {
                    continue;
                }

                for (vector<NodeInfo::LinkInfo>::const_iterator it_l = local_node_.info->links.begin(); it_l != local_node_.info->links.end(); ++it_l) {
                    const NodeInfo::LinkInfo& link = *it_l;
                    if (link.type == NodeInfo::LinkInfo::BY_NAME) {
                        if ((*it).name == (*it_l).matcher) {
                            list.push_back(LinkNodeInfo(*it, link.require));
                            break;
                        }
                    } else if (link.type == NodeInfo::LinkInfo::BY_TYPE) {
                        if ((*it).type == (*it_l).matcher) {
                            list.push_back(LinkNodeInfo(*it, link.require));
                            break;
                        }
                    }
                }
            }

            return list;
        }
    }
}
