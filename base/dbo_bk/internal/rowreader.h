#ifndef BASE_DBO_INTERNAL_ROWREADER_H
#define BASE_DBO_INTERNAL_ROWREADER_H

#include "../../global.h"
#include <string>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class PacketIn;

            union FieldValue {
                bool boolean;
                int8_t int8;
                uint8_t uint8;
                int16_t int16;
                uint16_t uint16;
                uint32_t uint32;
                int32_t int32;
                int64_t int64;
                uint64_t uint64;
                float v_float;
                double v_double;
                uint32_t buffer_pos;
            };

            class RowReader
            {
            public:
                virtual ~RowReader();

                // 解析行协议包
                virtual void ParseRowPacket(PacketIn& pktin) = 0;

                // 获取指定列数据
                virtual bool GetBoolean(uint32_t column) = 0;
                virtual int8_t GetInt8(uint32_t column) = 0;
                virtual uint8_t GetUInt8(uint32_t column) = 0;
                virtual int16_t GetInt16(uint32_t column) = 0;
                virtual uint16_t GetUInt16(uint32_t column) = 0;
                virtual int32_t GetInt32(uint32_t column) = 0;
                virtual uint32_t GetUInt32(uint32_t column) = 0;
                virtual int64_t GetInt64(uint32_t column) = 0;
                virtual uint64_t GetUInt64(uint32_t column) = 0;
                virtual std::string GetString(uint32_t column) = 0;
                virtual float GetFloat(uint32_t column) = 0;
                virtual double GetDouble(uint32_t column) = 0;

                // 指定列是否为NULL
                virtual bool IsNull(uint32_t column) const = 0;
            };

        }
    }
}
#endif // ROWREADER_H
