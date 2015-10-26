#ifndef BASE_DBO_INTERNAL_ROWREADERBINARY_H
#define BASE_DBO_INTERNAL_ROWREADERBINARY_H

#include "rowreader.h"
#include "packet.h"
#include "../metadata.h"

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class RowReaderBinary : public RowReader
            {
            public:
                RowReaderBinary(ResultSetMetadata& metadata)
                    : metadata_(metadata), bitmap_(nullptr), fields_(nullptr) {}
                virtual ~RowReaderBinary();

                virtual void ParseRowPacket(PacketIn& pktin);

                virtual bool GetBoolean(uint32_t column);
                virtual int8_t GetInt8(uint32_t column);
                virtual uint8_t GetUInt8(uint32_t column);
                virtual int16_t GetInt16(uint32_t column);
                virtual uint16_t GetUInt16(uint32_t column);
                virtual int32_t GetInt32(uint32_t column);
                virtual uint32_t GetUInt32(uint32_t column);
                virtual int64_t GetInt64(uint32_t column);
                virtual uint64_t GetUInt64(uint32_t column);
                virtual std::string GetString(uint32_t column);
                virtual float GetFloat(uint32_t column);
                virtual double GetDouble(uint32_t column);

                virtual inline bool IsNull(uint32_t column) const {
                    uint32_t byte_pos = (column + 2) / 8;
                    uint32_t bit_pos = (column + 2) % 8;
                    return bitmap_[byte_pos] & (1 << bit_pos);
                }

            private:
                PacketIn pktin_;
                ResultSetMetadata& metadata_;
                char* bitmap_;
                FieldValue* fields_;
            };
        }
    }
}

#endif // ROWREADERBINARY_H
