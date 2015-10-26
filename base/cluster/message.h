#ifndef BASE_CLUSTER_MESSAGE_H
#define BASE_CLUSTER_MESSAGE_H

#include "../gateway/packet.h"
#include "mailboxid.h"

namespace base
{
    namespace cluster
    {

        class Mailbox;
        // Message头部格式如下 head_size=16byte
        // 4 -- size            uint32
        // 2 -- code            uint16
        // 4 -- mbid_from       MailboxID
        // 4 -- mbid_to         MailboxID
        // 2 -- session         uint16

        typedef gateway::packet_data_t message_data_t;

        // 收到的消息
        class MessageIn : public gateway::PacketInBase
        {
        public:
            DISABLE_COPY(MessageIn)
            MessageIn(message_data_t& data)
                : gateway::PacketInBase(data) {}
            virtual ~MessageIn() {}

            static const uint16_t HEAD_SIZE = 16;

            uint16_t code() const {
                return code_;
            }

            const MailboxID& from() const {
                return mbid_from_;
            }

            const MailboxID& to() const {
                return mbid_to_;
            }

            uint16_t session() const {
                return session_;
            }

            void ReadHead() {
                SkipTo(4);
                code_ = ReadUShort();
                mbid_from_.Parse(ReadUInt());
                mbid_to_.Parse(ReadUInt());
                session_ = ReadUShort();
            }

        private:
            uint16_t code_;
            uint16_t session_;
            MailboxID mbid_from_;
            MailboxID mbid_to_;
        };

        // 发出的消息
        class MessageOut : public base::gateway::PacketOutBase
        {
        public:
            DISABLE_COPY(MessageOut)
            MessageOut(uint16_t code, uint32_t approx_size, base::memory::MemoryPool& mp)
                : gateway::PacketOutBase(approx_size < HEAD_SIZE ? HEAD_SIZE : approx_size, mp),
                  code_(code), session_(0) {
                SkipTo(HEAD_SIZE);
            }
            virtual ~MessageOut() {}

            static const uint16_t HEAD_SIZE = 16;

            const MailboxID& from() const {
                return mbid_from_;
            }

            const MailboxID& to() const {
                return mbid_to_;
            }

            uint16_t code() const {
                return code_;
            }

            uint16_t session() const {
                return session_;
            }

            void SetFrom(const MailboxID& from) {
                mbid_from_ = from;
            }
            void SetTo(const MailboxID& to) {
                mbid_to_ = to;
            }
            void SetSession(uint16_t session) {
                session_ = session;
            }

            void WriteHead() {
                SkipTo(0);
                WriteUInt(size());
                WriteUShort(code_);
                WriteUInt(mbid_from_.ValueID());
                WriteUInt(mbid_to_.ValueID());
                WriteUShort(session_);
            }

            uint16_t code_;
            MailboxID mbid_from_;
            MailboxID mbid_to_;
            uint16_t session_;
        };
    }
}

#endif // BASE_CLUSTER_MESSAGE_H
