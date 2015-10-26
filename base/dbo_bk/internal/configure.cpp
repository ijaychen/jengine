#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "configure.h"
#include "../../logger.h"
#include "../../file/utils.h"
#include "../../framework.h"
#include <string>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            using namespace std;
            using namespace boost::property_tree;

            Configure::Configure()
            {
            }

            bool Configure::ParseXml()
            {
                try {
                    string filename = framework.resource_dir() + "/dbo.conf";
                    if (file::file_can_read(filename.c_str())) {
                        ptree pt;
                        read_xml(filename, pt, xml_parser::no_comments);

                        ptree& dbo = pt.get_child("framework.dbo");
                        for (ptree::iterator it = dbo.begin(); it != dbo.end(); ++it) {
                            ptree& node = it->second.get_child("<xmlattr>");
                            ConnectionPoolInfo p;
                            p.poolid = node.get_child("id").get_value<uint32_t>();
                            p.poolsize = node.get_child("size").get_value<uint32_t>();
                            p.serverinfo.dbname = node.get_child("db").get_value<string>();
                            p.serverinfo.host = node.get_child("host").get_value<string>();
                            p.serverinfo.port = node.get_child("port").get_value<int32_t>();
                            p.serverinfo.username = node.get_child("user").get_value<string>();
                            p.serverinfo.password = node.get_child("pass").get_value<string>();
                            if (p.poolid >= pools_.size()) {
                                pools_.resize(p.poolid + 1);
                                pools_[p.poolid] = p;
                            }
                        }
                    }

                    filename = framework.priv_dir() + "/dbo.conf";
                    if (file::file_can_read(filename.c_str())) {
                        ptree pt;
                        read_xml(filename, pt, xml_parser::no_comments);

                        ptree& dbo = pt.get_child("framework.dbo");
                        for (ptree::iterator it = dbo.begin(); it != dbo.end(); ++it) {
                            ptree& node = it->second.get_child("<xmlattr>");
                            ConnectionPoolInfo p;
                            p.poolid = node.get_child("id").get_value<uint32_t>();
                            p.poolsize = node.get_child("size").get_value<uint32_t>();
                            p.serverinfo.dbname = node.get_child("db").get_value<string>();
                            p.serverinfo.host = node.get_child("host").get_value<string>();
                            p.serverinfo.port = node.get_child("port").get_value<int32_t>();
                            p.serverinfo.username = node.get_child("user").get_value<string>();
                            p.serverinfo.password = node.get_child("pass").get_value<string>();
                            if (p.poolid >= pools_.size()) {
                                pools_.resize(p.poolid + 1);
                                pools_[p.poolid] = p;
                            }
                        }
                    }
                    bool ok = true;
                    for (vector<ConnectionPoolInfo>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                        if ((*it).serverinfo.host.empty()) {
                            LOG_ERROR("parse dbo configure file fail: exist spare poolid!\n");
                            ok = false;
                            break;
                        }
                    }
                    return !pools_.empty() && ok;
                } catch (exception& ex) {
                    LOG_ERROR("parse dbo configure file fail: %s\n", ex.what());
                    return false;
                }
            }
        }
    }
}

