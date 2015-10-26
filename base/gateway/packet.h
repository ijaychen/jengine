#ifndef BASE_GATEWAY_PACKET_H
#define BASE_GATEWAY_PACKET_H

#include "packet_base.h"

namespace base
{
    namespace gateway
    {
        // 接收到的协议包
        // 包头格式:
        // 2 -- identifier      uint16_t        包头标识 PacketIn::IDENTIFIER
        // 2 -- length          uint16_t        包长度
        // 2 -- code            uint16_t        协议号
        // 2 -- signature       int16_t         签名(用于校验)
        // 2 -- sessionid       uint16_t        调用编号
        // [[...data]]
        class PacketIn : public PacketInBase
        {
        public:
            PacketIn(std::vector< memory::RefMemoryChunk >& data)
                : PacketInBase(data), code_(0), sessionid_(0) {}
            virtual ~PacketIn();

            static const uint16_t IDENTIFIER = 28380;   // 110, 220, ...
            static const uint32_t HEAD_SIZE = 10;

            union PacketHead {
                char data[4];
                struct {
                    uint16_t identifier;
                    uint16_t length;
                } head;
            };

            uint16_t code() const {
                return code_;
            }
            uint16_t sessionid() const {
                return sessionid_;
            }

            void ReadHead() {
                SkipTo(4);
                int16_t signature;
                (*this) >> code_ >> signature >> sessionid_;
                if (signature != CalculateSignature()) {
                    //throw Exception("bad packet signature, the packet is broken");
                }
            }

        private:
            int16_t CalculateSignature() {
                return 0;
            }
            uint16_t code_;
            uint16_t sessionid_;
        };

        // 发送出去的协议包
        class PacketOut : public PacketOutBase
        {
        public:
            // code 协议号
            // approx_size 预估的协议包占用字节数
            // mempool 内存池
            PacketOut(uint16_t code, uint32_t approx_size, memory::MemoryPool& mempool)
                : PacketOutBase(approx_size, mempool), code_(code), sessionid_(0), head_writed_(false) {
                SkipTo(HEAD_SIZE);
            }
            virtual ~PacketOut();

            static const uint16_t IDENTIFIER = 28380;       // 220, 110, ...
            static const uint16_t HEAD_SIZE = 10;

            // 协议号
            uint16_t code() const {
                return code_;
            }
            uint16_t sessionid() const {
                return sessionid_;
            }

            void SetSessionID(uint16_t sid) {
                sessionid_ = sid;
            }

            // 写入包头
            // rewrite 是否强制再写一次协议头
            void WriteHead(bool rewrite = false) {
                if (!head_writed_ || rewrite) {
                    SkipTo(0);
                    WriteUShort(IDENTIFIER);
                    WriteUShort(size());
                    WriteUShort(code());
                    WriteShort(CalculateSignature());
                    WriteUShort(sessionid_);
                    head_writed_ = true;
                }
            }

        private:
            int16_t CalculateSignature() {
                return 0;
            }
            uint16_t code_;
            uint16_t sessionid_;
            bool head_writed_;
        };
    }
}
#endif
