#include "packet_base.h"
#include "../memory/memorypool.h"
#include "../utils/string.h"

namespace base
{
    namespace gateway
    {
        using namespace std;

        Packet::~Packet()
        {
        }

        std::string Packet::Dump() const
        {
            std::string txt;
            uint32_t total = size();
            packet_data_t::const_iterator it = data().begin();
            while (it != data().end() && total > 0) {
                int acc = 0;
                for (uint32_t i = 0; i < total && i < (*it).count(); ++i) {
                    base::utils::string_append_format(txt, "%u,", (uint32_t)(*((*it).data() + i)));
                    ++acc;
                }
                total -= acc;
                ++it;
            }
            return txt;
        }

        void PacketOutBase::AquireMemory(uint32_t need_bytes)
        {
            mempool_.AquireByByteNeed(data_, need_bytes + 10);
            UpdateCapacity();
        }
    }
}
