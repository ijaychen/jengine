#include "rowreaderbinary.h"
#include "../../global.h"
#include <iostream>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            using namespace std;

            RowReaderBinary::~RowReaderBinary()
            {
                if (bitmap_ != nullptr) {
                    delete[] bitmap_;
                    bitmap_ = nullptr;
                }
                if (fields_ != nullptr) {
                    delete[] fields_;
                }
            }

            void RowReaderBinary::ParseRowPacket(PacketIn& data)
            {
                pktin_.Swap(data);
                pktin_.SkipTo(5);
                // parse NULL-bitmap
                uint32_t bitmap_len = (metadata_.num_columns + 7 + 2) / 8;
                bitmap_ = new char[bitmap_len];
                pktin_.ReadFixedString(bitmap_, bitmap_len);
                //cout << "----------- parse binary row packet.. num_columns:" << metadata_.num_columns << ", rawdata:" << pktin_.Dump() << endl;
                //cout << "bitmap_len:" << bitmap_len << ", " << (uint32_t)bitmap_[0] << endl;

                fields_ = new FieldValue[metadata_.num_columns];
                for (uint32_t column = 0; column < metadata_.num_columns; ++column) {
                    // 判断是否为空
                    const ColumnDefinition& cm = metadata_.GetColumnDefinition(column);
                    if (!IsNull(column)) {
                        switch (cm.type) {
                            case MYSQL_TYPE_TINY:
                                pktin_.ReadFixedInteger<1>(&fields_[column].int8);
                                break;
                            case MYSQL_TYPE_SHORT:
                                pktin_.ReadFixedInteger<2>(&fields_[column].int16);
                                break;
                            case MYSQL_TYPE_LONG:
                                pktin_.ReadFixedInteger<4>(&fields_[column].int32);
                                break;
                            case MYSQL_TYPE_LONGLONG:
                                pktin_.ReadFixedInteger<8>(&fields_[column].int64);
                                break;
                            case MYSQL_TYPE_FLOAT:
                                pktin_.ReadFixedInteger<4>(&fields_[column].v_float);
                                break;
                            case MYSQL_TYPE_DOUBLE:
                                pktin_.ReadFixedInteger<8>(&fields_[column].v_double);
                                break;
                            case MYSQL_TYPE_INT24:
                                pktin_.ReadFixedInteger<4>(&fields_[column].int32);
                                break;
                            case MYSQL_TYPE_YEAR:
                                pktin_.ReadFixedInteger<2>(&fields_[column].int16);
                                break;
                            case MYSQL_TYPE_DECIMAL:
                            case MYSQL_TYPE_VARCHAR:
                            case MYSQL_TYPE_BIT:
                            case MYSQL_TYPE_NEWDECIMAL:
                            case MYSQL_TYPE_TINY_BLOB:
                            case MYSQL_TYPE_MEDIUM_BLOB:
                            case MYSQL_TYPE_LONG_BLOB:
                            case MYSQL_TYPE_BLOB:
                            case MYSQL_TYPE_VAR_STRING:
                            case MYSQL_TYPE_STRING:
                                fields_[column].buffer_pos = pktin_.pos();
                                uint32_t len;
                                pktin_.ReadVariableInteger(&len);
                                pktin_.Skip(len);
                                break;
                            default:
                                throw Exception("not support mysql type");
                                break;
                        }
                    }
                }
            }

            /// 判定类型是否匹配，或是否可以隐匿转换

            bool RowReaderBinary::GetBoolean(uint32_t column)
            {
                return fields_[column].boolean;
            }

            int8_t RowReaderBinary::GetInt8(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_TINY) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].int8;
            }

            uint8_t RowReaderBinary::GetUInt8(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_TINY) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].uint8;
            }

            int16_t RowReaderBinary::GetInt16(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_SHORT && type != MYSQL_TYPE_YEAR) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].int16;
            }

            uint16_t RowReaderBinary::GetUInt16(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_SHORT) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].uint16;
            }

            int32_t RowReaderBinary::GetInt32(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_LONG && type != MYSQL_TYPE_INT24) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].int32;
            }

            uint32_t RowReaderBinary::GetUInt32(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_LONG && type != MYSQL_TYPE_INT24) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].uint32;
            }

            int64_t RowReaderBinary::GetInt64(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_LONGLONG) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].int64;
            }

            uint64_t RowReaderBinary::GetUInt64(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_LONGLONG) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].uint64;
            }

            string RowReaderBinary::GetString(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if ((type != MYSQL_TYPE_VAR_STRING
                        && type != MYSQL_TYPE_BLOB
                        && type != MYSQL_TYPE_TINY_BLOB
                        && type != MYSQL_TYPE_MEDIUM_BLOB
                        && type != MYSQL_TYPE_LONG_BLOB
                        && type != MYSQL_TYPE_BIT
                        && type != MYSQL_TYPE_STRING
                        && type != MYSQL_TYPE_VARCHAR) || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                string ret;
                pktin_.SkipTo(fields_[column].buffer_pos);
                pktin_.ReadLengthEncodedString(ret);
#ifdef HAS_CXX11
                return move(ret);
#else
                return ret;
#endif
            }

            float RowReaderBinary::GetFloat(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if (type != MYSQL_TYPE_FLOAT || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].v_float;
            }
            
            double RowReaderBinary::GetDouble(uint32_t column)
            {
                MysqlFieldType type = metadata_.GetColumnDefinition(column).type;
                if (type != MYSQL_TYPE_DOUBLE || IsNull(column)) {
                    throw Exception("field type mismatch or is null");
                }
                return fields_[column].v_double;
            }
        }
    }
}

