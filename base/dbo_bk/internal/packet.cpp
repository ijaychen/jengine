#include "packet.h"
#include "../../utils/string.h"

namespace base
{
    namespace dbo
    {
        namespace internal
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

            void PacketOut::AquireMemory(uint32_t need_bytes)
            {
                mempool_.AquireByByteNeed(data_, need_bytes + 10);
                UpdateCapacity();
            }

            void PacketOut::AppendData(const packet_data_t& data)
            {
                if (data.empty()) {
                    return;
                }
                
                memory::RefMemoryChunk& ck = data_[pos_y_];
                if (ck.pos() == 0) {
                    data_.pop_back();
                    --pos_y_;
                } else {
                    ck.ShrinkCount(ck.count() - ck.pos());
                }
                
                for (packet_data_t::const_iterator it = data.begin(); it != data.end(); ++it) {
                    data_.push_back(*it);
                    pos_ += (*it).count();
                    ++pos_y_;
                }
                UpdateCapacity();
                UpdateSize();
            }
        }
    }
}
