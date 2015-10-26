#include "mailboxid.h"
#include "message.h"
#include <ostream>

namespace base
{
    namespace cluster
    {
        using namespace std;

        std::ostream& operator<<(std::ostream& out, const MailboxID& mbid)
        {
            out << "{node:" << (uint32_t)mbid.nodeid() << ", pid:" << mbid.pid() << "}";
            return out;
        }

        gateway::PacketOutBase& operator<<(gateway::PacketOutBase& pktout, const MailboxID& mbid)
        {
            pktout.WriteUInt(mbid.ValueID());
            return pktout;
        }

        gateway::PacketInBase& operator>>(gateway::PacketInBase& pktin, MailboxID& mbid)
        {
            uint32_t value = pktin.ReadUInt();
            mbid.Parse(value);
            return pktin;
        }
    }
}
