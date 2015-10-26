#ifndef BASE_DBO_INTERNAL_ROWREADERTEXT_H
#define BASE_DBO_INTERNAL_ROWREADERTEXT_H

#include "rowreader.h"
#include "packet.h"
#include "../metadata.h"

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class RowReaderText : public RowReader
            {
            public:
                RowReaderText(ResultSetMetadata& metadata)
                    : metadata_(metadata), fields_(nullptr) {}
                virtual ~RowReaderText();

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
                    return fields_[column].is_null;
                }

            private:
                PacketIn pktin_;
                ResultSetMetadata& metadata_;
                struct Field {
                    bool is_null;
                    FieldValue value;
                    Field() : is_null(false) {}
                };
                Field* fields_;
            };
        }
    }
}

#endif // ROWREADERTEXT_H
