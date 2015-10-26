#ifndef BASE_CLUSTER_MAILBOXID_H
#define BASE_CLUSTER_MAILBOXID_H

#include "../global.h"
#include <iosfwd>

namespace base
{
    namespace gateway
    {
        class PacketInBase;
        class PacketOutBase;
    }
    
    namespace cluster
    {
        // 分布式邮箱ID
        class MailboxID
        {
        public:
            MailboxID() : nodeid_(0), pid_(0) {}
            MailboxID(uint8_t nodeid, int32_t pid) : nodeid_(nodeid), pid_(pid) {}

            inline uint8_t nodeid() const {
                return nodeid_;
            }

            inline int32_t pid() const {
                return pid_;
            }

            operator bool() const {
                return nodeid_ != 0 && pid_ != 0;
            }

            bool operator == (const MailboxID& rhs) const {
                return nodeid() == rhs.nodeid() && pid() == rhs.pid();
            }

            bool operator != (const MailboxID& rhs) const {
                return !(*this == rhs);
            }

            uint32_t ValueID() const {
                return nodeid_ << 24 | (~0 >> 8 & pid_);
            }

            void Parse(uint32_t value) {
                nodeid_ = value >> 24;
                pid_ = (value << 8) >> 8;
            }

            void Clear() {
                nodeid_ = 0;
                pid_ = 0;
            }

            static const MailboxID& NullID() {
                static MailboxID nullid(0, 0);
                return nullid;
            }


        private:
            int32_t nodeid_;
            int32_t pid_;
        };

        std::ostream& operator << (std::ostream& out, const MailboxID& mbid);
        gateway::PacketOutBase& operator << (gateway::PacketOutBase& pktout, const MailboxID& mbid);
        gateway::PacketInBase& operator >> (gateway::PacketInBase& pktin, MailboxID& mbid);
    }
}

#endif // MAILBOXID_H
