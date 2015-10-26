#include "userclient.h"
#include "../net/utils.h"
#include "../logger.h"
#include "usersession.h"

namespace base
{
    namespace gateway
    {
        UserClient::~UserClient()
        {
        }
        
        void UserClient::OnReceive(std::size_t count)
        {
            // 解析数据包
            while (count >= PacketIn::HEAD_SIZE) {
                CopyReceive(pkthead_.data, sizeof(pkthead_));
                if (pkthead_.head.identifier == PacketIn::IDENTIFIER
                        && pkthead_.head.length >= PacketIn::HEAD_SIZE) {
							LOG_WARN("pkthead_.head.length:%d,count:%d",pkthead_.head.length,count);
                    if (count >= pkthead_.head.length) {
                        std::vector<memory::RefMemoryChunk> data;
                        PopReceive(data, pkthead_.head.length);
                        PacketIn pktin(data);
                        try {
                            pktin.ReadHead();
                            if (session_) {
                                session_->OnUserClientReceivePacket(pktin);
                            }
                            count -= pkthead_.head.length;
                        } catch (std::exception& ex) {
                            LOG_ERROR("handle receive packet catch exception: %s\n", ex.what());
                            Close();
                            break;
                        }
                    } else {
                        // 不足以够成一个完整的数据包
                        break;
                    }
                } else {
                    char buff[PacketIn::HEAD_SIZE];
                    CopyReceive(buff, PacketIn::HEAD_SIZE);
                    std::string text = net::dump_raw_data(buff, PacketIn::HEAD_SIZE);
                    LOG_ERROR("detected broken packet from %s:%d: count=%u, identifier=%u, length=%u, data=%s\n",
                              ipaddr().c_str(), port(), count, pkthead_.head.identifier, pkthead_.head.length, text.c_str());
                    Close();
                    break;
                }
            }
        }
    }
}
