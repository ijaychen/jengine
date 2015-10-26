#ifndef BASE_GATEWAY_PACKETFILTERLIST_H
#define BASE_GATEWAY_PACKETFILTERLIST_H

#include "packetfilter.h"

namespace base
{
    namespace gateway
    {
        class PacketFilterList
        {
        public:
            void Add(PacketFilter* filter) {
                filters_.push_front(filter);
            }
            void Remove(PacketFilter* filter) {
                filters_.erase(filter);
            }
            bool HandleReceivePacket(PacketIn& pktin) {
                PacketFilter* filter = filters_.front();
                while (filter) {
                    if (!filter->HandleReceivePacket(pktin)) {
                        return false;
                    }
                    filter = filter->list_next();
                }
                return true;
            }

        private:
            base::utils::IntrusiveList<PacketFilter> filters_;
        };
    }
}

#endif
