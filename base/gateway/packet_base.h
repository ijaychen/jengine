#ifndef BASE_GATEWAY_PACKET_BASE_H
#define BASE_GATEWAY_PACKET_BASE_H

#include "../global.h"
#include "../memory/memorychunk.h"
#include "../exception.h"
#include <vector>
#include <list>
#ifdef HAS_CXX11
#include <array>
#endif
#include <string>

namespace base
{
    namespace gateway
    {
        typedef std::vector<memory::RefMemoryChunk> packet_data_t;

        // 数据封包
        class Packet
        {
        public:
            DISABLE_COPY(Packet)
            Packet() : size_(0), pos_(0), pos_y_(0), capacity_(0) {}
            virtual ~Packet();

            uint32_t size() const {
                return size_;
            }
            uint32_t capacity() const {
                return capacity_;
            }
            uint32_t pos() const {
                return pos_;
            }
            const std::vector<memory::RefMemoryChunk>& data() const {
                return data_;
            }

            // 跳转到指定光标
            void SkipTo(uint32_t pos) {
                assert(pos <= capacity_);
                pos_ = pos;
                UpdateSize();
                pos_y_ = 0;
                for (packet_data_t::iterator it = data_.begin(); it != data_.end(); ++it) {
                    if (pos == 0) {
                        (*it).SkipTo(pos);
                    } else {
                        if (pos < (*it).count()) {
                            (*it).SkipTo(pos);
                            pos = 0;
                        } else {
                            (*it).SkipTo((*it).count());
                            pos -= (*it).count();
                            ++pos_y_;
                        }
                    }
                }
            }

            std::string Dump() const;

        protected:
            packet_data_t data_;        // 数据
            uint32_t size_;             // 实际数据量
            uint32_t pos_;              // 光标
            uint32_t pos_y_;            // 光标y
            uint32_t capacity_;         // 容量

            // 更新实际数据量
            inline void UpdateSize() {
                if (size_ < pos_) {
                    size_ = pos_;
                }
            }
            // 更新容量
            inline void UpdateCapacity() {
                capacity_ = 0;
                for (packet_data_t::iterator it = data_.begin(); it != data_.end(); ++it) {
                    capacity_ += (*it).count();
                }
            }
        };

        // 只读的协议包
        class PacketInBase : public Packet
        {
        public:
            DISABLE_COPY(PacketInBase)
            PacketInBase(packet_data_t& data)
                : Packet() {
                data_.swap(data);
                UpdateCapacity();
                size_ = capacity();
                SkipTo(0);
            }
            virtual ~PacketInBase() {}

            bool ReadBoolean() {
                return ReadByte() == 1;
            }

            int8_t ReadSByte() {
                int8_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 1);
                return ret;
            }

            uint8_t ReadByte() {
                uint8_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 1);
                return ret;
            }

            int16_t ReadShort() {
                int16_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 2);
                return ret;
            }

            uint16_t ReadUShort() {
                uint16_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 2);
                return ret;
            }

            int32_t ReadInt() {
                int32_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 4);
                return ret;
            }

            uint32_t ReadUInt() {
                uint32_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 4);
                return ret;
            }

            float ReadFloat() {
                float ret;
                GetByte(reinterpret_cast<char*>(&ret), 4);
                return ret;
            }

            double ReadDouble() {
                double ret;
                GetByte(reinterpret_cast<char*>(&ret), 8);
                return ret;
            }

            int64_t ReadLong() {
                int64_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 8);
                return ret;
            }

            uint64_t ReadULong() {
                uint64_t ret;
                GetByte(reinterpret_cast<char*>(&ret), 8);
                return ret;
            }

            void ReadString(std::string& str) {
                uint16_t strlen = ReadUShort();
                str.resize(strlen);
                char* dst = const_cast<char*>(str.c_str());
                GetByte(dst, strlen);
            }

            PacketInBase& operator >> (bool& v) {
                uint8_t b;
                (*this) >> b;
                v = (b == 1);
                return *this;
            }

            PacketInBase& operator >> (uint8_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 1);
                return *this;
            }

            PacketInBase& operator >> (int8_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 1);
                return *this;
            }

            PacketInBase& operator >> (uint16_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 2);
                return *this;
            }

            PacketInBase& operator >> (int16_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 2);
                return *this;
            }

            PacketInBase& operator >> (uint32_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 4);
                return *this;
            }

            PacketInBase& operator >> (int32_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 4);
                return *this;
            }

            PacketInBase& operator >> (uint64_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 8);
                return *this;
            }

            PacketInBase& operator >> (int64_t& v) {
                GetByte(reinterpret_cast<char*>(&v), 8);
                return *this;
            }

            PacketInBase& operator >> (float& v) {
                GetByte(reinterpret_cast<char*>(&v), 4);
                return *this;
            }

            PacketInBase& operator >> (double& v) {
                GetByte(reinterpret_cast<char*>(&v), 8);
                return *this;
            }

            PacketInBase& operator >> (std::string& v) {
                uint16_t len;
                (*this) >> len;
                v.resize(len);
                GetByte(const_cast<char*>(v.c_str()), len);
                return *this;
            }

            std::string ReadString() {
                std::string str;
                ReadString(str);
#ifdef HAS_CXX11
                return std::move(str);
#else
                return str;
#endif
            }

            void ReadBits(std::vector<char>& dst) {
                uint16_t bitslen = ReadUShort();
                dst.resize(bitslen);
                GetByte(dst.data(), bitslen);
            }

            void ReadRaw(std::vector<char>& dst, uint16_t size) {
                dst.resize(size);
                GetByte(dst.data(), size);
            }

        private:
            void GetByte(char* dst, uint32_t count) {
                if (pos_ + count > size_) {
                    throw Exception("Packet::GetByte no more data");
                }
                pos_ += count;
                while (count > 0 && pos_y_ < data_.size()) {
                    uint32_t readed = data_[pos_y_].Read(dst, count);
                    count -= readed;
                    dst += readed;
                    if (data_[pos_y_].FreeCount() == 0) {
                        ++pos_y_;
                    }
                }
            }
        };

        // 只写的协议包
        class PacketOutBase : public Packet
        {
        public:
            DISABLE_COPY(PacketOutBase)
            PacketOutBase(uint32_t approx_size, memory::MemoryPool& mempool)
                : mempool_(mempool) {
                AquireMemory(approx_size);
            }

            void WriteBoolean(bool v) {
                if (v) {
                    WriteByte(1);
                } else {
                    WriteByte(0);
                }
            }

            void WriteSByte(int8_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 1);
            }

            void WriteByte(uint8_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 1);
            }

            void WriteShort(int16_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 2);
            }

            void WriteUShort(uint16_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 2);
            }

            void WriteInt(int32_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 4);
            }

            void WriteUInt(uint32_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 4);
            }

            void WriteFloat(float v) {
                PutByte(reinterpret_cast<const char*>(&v), 4);
            }

            void WriteDouble(double v) {
                PutByte(reinterpret_cast<const char*>(&v), 8);
            }

            void WriteLong(int64_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 8);
            }

            void WriteULong(uint64_t v) {
                PutByte(reinterpret_cast<const char*>(&v), 8);
            }

            void WriteString(const char* str) {
                uint16_t len = strlen(str);
                WriteUShort(strlen(str));
                PutByte(str, len);
            }

            void WriteString(const std::string& str) {
                WriteUShort(str.length());
                PutByte(str.c_str(), str.length());
            }

            void WriteBits(const char* v, uint16_t len) {
                WriteUShort(len);
                PutByte(v, len);
            }

            void WriteRaw(const char* data, uint16_t size) {
                PutByte(data, size);
            }

            PacketOutBase& operator << (uint8_t v) {
                WriteByte(v);
                return *this;
            }

            PacketOutBase& operator << (int8_t v) {
                WriteSByte(v);
                return *this;
            }

            PacketOutBase& operator << (uint16_t v) {
                WriteUShort(v);
                return *this;
            }

            PacketOutBase& operator << (int16_t v) {
                WriteShort(v);
                return *this;
            }

            PacketOutBase& operator << (uint32_t v) {
                WriteUInt(v);
                return *this;
            }

            PacketOutBase& operator << (int32_t v) {
                WriteInt(v);
                return *this;
            }

            PacketOutBase& operator << (uint64_t v) {
                WriteULong(v);
                return *this;
            }

            PacketOutBase& operator << (int64_t v) {
                WriteLong(v);
                return *this;
            }

            PacketOutBase& operator << (float v) {
                WriteFloat(v);
                return *this;
            }

            PacketOutBase& operator << (double v) {
                WriteDouble(v);
                return *this;
            }

            PacketOutBase& operator << (const char* v) {
                WriteString(v);
                return *this;
            }

            PacketOutBase& operator << (const std::string& v) {
                WriteString(v);
                return *this;
            }

            PacketOutBase& operator << (bool v) {
                WriteBoolean(v);
                return *this;
            }

            template<typename T>
            PacketOutBase& operator << (const T* v) {
                *this << *v;
                return *this;
            }

            packet_data_t FetchData() {
                packet_data_t ret;
                uint32_t s = size();
                packet_data_t::iterator it = data_.begin();
                while (it != data_.end() && s > 0) {
                    memory::RefMemoryChunk& ck = (*it);
                    if (s < ck.count()) {
                        ret.push_back(ck);
                        ret.back().ShrinkCount(ck.count() - s);
                        s = 0;
                    } else {
                        ret.push_back(ck);
                        s -= ck.count();
                    }
                    ++it;
                }
#ifdef HAS_CXX11
                return std::move(ret);
#else
                return ret;
#endif
            }

        private:
            void PutByte(const char* src, uint32_t count) {
                while (pos_ + count > capacity_) {
                    AquireMemory(count);
                }
                pos_ += count;
                UpdateSize();
                while (count > 0 && pos_y_ < data_.size()) {
                    uint32_t writed = data_[pos_y_].Write(src, count);
                    count -= writed;
                    src += writed;
                    if (data_[pos_y_].FreeCount() == 0) {
                        ++pos_y_;
                    }
                }
            }
            void AquireMemory(uint32_t need_bytes);
            memory::MemoryPool& mempool_;
        };

        template<typename T>
        PacketOutBase& operator << (PacketOutBase& pktout, const std::vector<T>& list)
        {
            pktout.WriteUShort(list.size());
            for (typename std::vector<T>::const_iterator it = list.begin(); it != list.end(); ++it) {
                pktout << *it;
            }
            return pktout;
        }

#ifdef HAS_CXX11
        template<typename T, std::size_t N>
        PacketOutBase& operator << (PacketOutBase& pktout, const std::array<T, N>& list)
        {
            pktout.WriteUShort(list.size());
            for (typename std::array<T, N>::const_iterator it = list.begin(); it != list.end(); ++it) {
                pktout << *it;
            }
            return pktout;
        }
#endif

        template<typename T>
        PacketOutBase& operator << (PacketOutBase& pktout, const std::list<T>& list)
        {
            pktout.WriteUShort(list.size());
            for (typename std::list<T>::const_iterator it = list.begin(); it != list.end(); ++it) {
                pktout << *it;
            }
            return pktout;
        }
    }
}

#endif // PACKET_H
