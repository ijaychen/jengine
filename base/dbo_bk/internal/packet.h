#ifndef BASE_DBO_INTERNAL_PACKET_H
#define BASE_DBO_INTERNAL_PACKET_H

#include "../../global.h"
#include "../../memory/memorypool.h"
#include "../../exception.h"

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            typedef std::vector<memory::RefMemoryChunk> packet_data_t;

            class Packet
            {
            public:
                DISABLE_COPY(Packet)
                Packet() : size_(0), pos_(0), pos_y_(0), capacity_(0), payload_(0), sequence_id_(0) {}
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
                uint32_t payload() const {
                    return payload_;
                }
                uint8_t sequence_id() const {
                    return sequence_id_;
                }

                uint32_t FreeCount() const {
                    return capacity_ - pos_;
                }

                void Skip(uint32_t step) {
                    SkipTo(pos_ + step);
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

                // mysql header
                uint32_t payload_;           // 有效负载
                uint8_t sequence_id_;        // 序号

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

            class PacketIn : public Packet
            {
            public:
                PacketIn() : Packet() {}
                PacketIn(packet_data_t& data) : Packet() {
                    data_.swap(data);
                    UpdateCapacity();
                    size_ = capacity_;
                }
                
                uint8_t sequence_id() const {
                    return sequence_id_;
                }

                void Swap(PacketIn& pktin) {
                    data_.swap(pktin.data_);
                    capacity_ = pktin.capacity_;
                    size_ = pktin.size_;
                    pktin.capacity_ = 0;
                    pktin.size_ = 0;
                    pktin.pos_ = 0;
                    pktin.pos_y_ = 0;
                }

                uint8_t CurrentByte() {
                    memory::RefMemoryChunk& ck = data_[pos_y_];
                    return ck.data()[ck.pos()];
                }

                template<typename T, uint32_t N>
                T ReadFixedInteger() {
                    T v = 0;
                    GetByte(reinterpret_cast<char*>(&v), N);
                    return v;
                }

                template<uint32_t N, typename T>
                void ReadFixedInteger(T* v) {
                    GetByte(reinterpret_cast<char*>(v), N);
                }

                template<typename T>
                void ReadVariableInteger(T* v) {
                    uint8_t first = 0;
                    GetByte(reinterpret_cast<char*>(&first), 1);
                    if (first < 251) {
                        *v = (T)first;
                    } else if (first == 0xfc) {
                        uint16_t follow;
                        ReadFixedInteger<2>(&follow);
                        *v = (T)follow;
                    } else if (first == 0xfd) {
                        uint32_t follow;
                        ReadFixedInteger<3>(&follow);
                        *v = (T)follow;
                    } else if (first == 0xfe) {
                        uint64_t follow;
                        ReadFixedInteger<8>(&follow);
                        *v = (T)follow;
                    } else {
                        assert(!"not support");
                    }
                }

                void ReadFixedString(std::string& v, uint32_t fixed_len) {
                    v.resize(fixed_len);
                    GetByte(const_cast<char*>(v.c_str()), fixed_len);
                }

                void ReadFixedString(char* v, uint32_t fixed_len) {
                    GetByte(v, fixed_len);
                }

                void ReadLengthEncodedString(std::string& txt) {
                    uint32_t len = 0;
                    ReadVariableInteger(&len);
                    txt.resize(len);
                    GetByte(const_cast<char*>(txt.c_str()), len);
                }

                void ReadRestOfPacketString(std::string& txt) {
                    uint32_t left = size_ - pos_;
                    txt.resize(left);
                    GetByte(const_cast<char*>(txt.c_str()), left);
                }

                void ReadNulTerminateString(std::string& txt) {
                    while (true) {
                        char v;
                        GetByte(&v, 1);
                        if (v == '\0') {
                            break;
                        } else {
                            txt.push_back(v);
                        }
                    }
                }

                void ReadHead() {
                    SkipTo(0);
                    ReadFixedInteger<3>(&payload_);
                    ReadFixedInteger<1>(&sequence_id_);
                    if (payload_ + 3 + 1 != size()) {
                        throw Exception("broken packet from mysql");
                    }
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

            class PacketOut : public Packet
            {
            public:
                PacketOut(memory::MemoryPool& mempool, uint32_t approx_size = 20, uint8_t sequence_id = 0)
                    : Packet(), mempool_(mempool) {
                    AquireMemory(approx_size < 20 ? 20 : approx_size);
                    sequence_id_ = sequence_id;
                    SkipTo(4);
                }

                void SetSequenceID(uint8_t sequence_id) {
                    sequence_id_ = sequence_id;
                }

                template<uint32_t N, typename T>
                void WriteFixedInteger(const T& v) {
                    PutByte((const char*)&v, N);
                }

                void WriteHead() {
                    SkipTo(0);
                    WriteFixedInteger<3>(size() - 4);
                    WriteFixedInteger<1>(sequence_id());
                }

                void WriteVariableInteger(uint64_t v) {
                    static char vi2 = 0xfc;
                    static char vi3 = 0xfd;
                    static char vi8 = 0xfe;
                    if (v < 251ull) {
                        PutByte(reinterpret_cast<char*>(&v), 1);
                    } else if (v >= 251lu && v < 65536ULL) {
                        PutByte(&vi2, 1);
                        PutByte(reinterpret_cast<char*>(&v), 2);
                    } else if (v >= 65536ULL && v < 16777216ULL) {
                        PutByte(&vi3, 1);
                        PutByte(reinterpret_cast<char*>(&v), 3);
                    } else {
                        PutByte(&vi8, 1);
                        PutByte(reinterpret_cast<char*>(&v), 8);
                    }
                }

                void WriteFixedString(const std::string& str) {
                    WriteFixedString(str.c_str(), str.length());
                }
                void WriteFixedString(const char* str, std::size_t len) {
                    PutByte(str, len);
                }

                void WriteLengthEncodedString(const std::string& str) {
                    WriteLengthEncodedString(str.c_str(), str.length());
                }
                void WriteLengthEncodedString(const char* str, std::size_t len) {
                    WriteVariableInteger(len);
                    PutByte(str, len);
                }

                void WriteNulTerminateString(const std::string& str) {
                    WriteNulTerminateString(str.c_str(), str.length());
                }
                void WriteNulTerminateString(const char* str, std::size_t len) {
                    PutByte(str, len);
                    static char terminate = '\0';
                    PutByte(&terminate, 1);
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

                void AppendData(const packet_data_t& data);

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
        }
    }
}
#endif // PACKET_H
