#include "rowreadertext.h"
#include <cstdlib>
#include <iostream>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            using namespace std;

            RowReaderText::~RowReaderText()
            {
                if (fields_) {
                    delete[] fields_;
                    fields_ = nullptr;
                }
            }

            void RowReaderText::ParseRowPacket(PacketIn& data)
            {
                pktin_.Swap(data);
                pktin_.SkipTo(4);
                //cout << "parse text row packet.." << pktin_.Dump() << endl;
                // 将每个字段的位置读取出来
                uint32_t len = 0;
                uint32_t buffer_pos = 0;
                fields_ = new Field[metadata_.num_columns];
                string buffer;
                buffer.reserve(32);
                for (uint32_t column = 0; column < metadata_.num_columns; ++column) {
                    buffer_pos = pktin_.pos();
                    uint8_t flag = pktin_.CurrentByte();
                    if (flag == 0xFB) {
                        fields_[column].is_null = true;
                        pktin_.Skip(1);
                    } else {
                        pktin_.ReadVariableInteger(&len);
                        const ColumnDefinition& cm = metadata_.GetColumnDefinition(column);
                        switch (cm.type) {
                            case MYSQL_TYPE_TINY:
                            case MYSQL_TYPE_SHORT:
                            case MYSQL_TYPE_LONG:
                            case MYSQL_TYPE_YEAR:
                                pktin_.ReadFixedString(buffer, len);
                                fields_[column].value.int32 = strtol(buffer.c_str(), nullptr, 10);
                                break;
                            case MYSQL_TYPE_LONGLONG:
                                pktin_.ReadFixedString(buffer, len);
                                fields_[column].value.int64 = strtoll(buffer.c_str(), nullptr, 10);
                                break;
                            case MYSQL_TYPE_FLOAT:
                                pktin_.ReadFixedString(buffer, len);
                                fields_[column].value.v_float = strtof(buffer.c_str(), nullptr);
                                break;
                            case MYSQL_TYPE_DOUBLE:
                                pktin_.ReadFixedString(buffer, len);
                                fields_[column].value.v_double = strtod(buffer.c_str(), nullptr);
                                break;
                            case MYSQL_TYPE_VARCHAR:
                            case MYSQL_TYPE_BIT:
                            case MYSQL_TYPE_NEWDECIMAL:
                            case MYSQL_TYPE_TINY_BLOB:
                            case MYSQL_TYPE_MEDIUM_BLOB:
                            case MYSQL_TYPE_LONG_BLOB:
                            case MYSQL_TYPE_BLOB:
                            case MYSQL_TYPE_VAR_STRING:
                            case MYSQL_TYPE_STRING:
                                fields_[column].value.buffer_pos = buffer_pos;
                                pktin_.Skip(len);
                                break;
                            default:
                                throw Exception("not support mysql type");
                                break;
                        }
                    }
                }
            }

            bool RowReaderText::GetBoolean(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.boolean;
            }

            int8_t RowReaderText::GetInt8(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.int8;
            }

            uint8_t RowReaderText::GetUInt8(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.uint8;
            }

            int16_t RowReaderText::GetInt16(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.int16;
            }

            uint16_t RowReaderText::GetUInt16(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.uint16;
            }

            int32_t RowReaderText::GetInt32(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.int32;
            }

            uint32_t RowReaderText::GetUInt32(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.uint32;
            }

            int64_t RowReaderText::GetInt64(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.int64;
            }

            uint64_t RowReaderText::GetUInt64(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.uint64;
            }

            std::string RowReaderText::GetString(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                pktin_.SkipTo(fields_[column].value.buffer_pos);
                string ret;
                pktin_.ReadLengthEncodedString(ret);
#ifdef HAS_CXX11
                return move(ret);
#else
                return ret;
#endif
            }

            float RowReaderText::GetFloat(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.v_float;
            }

            double RowReaderText::GetDouble(uint32_t column)
            {
                if (fields_[column].is_null) {
                    throw Exception("null field");
                }
                return fields_[column].value.v_double;
            }
        }
    }
}
