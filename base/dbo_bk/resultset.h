#ifndef BASE_DBO_RESULTSET_H
#define BASE_DBO_RESULTSET_H

#include <string>
#include <vector>
#include <list>
#include "metadata.h"
#include "internal/rowreader.h"

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class PacketIn;
            class PacketIn;
        }

        class ResultSet
        {
        public:
            ResultSet();
            virtual ~ResultSet();

            // 是否有错误
            bool HasError() {
                return error_code_ > 0;
            }
            // 错误码
            uint16_t error_code() const {
                return error_code_;
            }
            // 错误状态码
            const char* error_state() const {
                return error_state_.c_str();
            }
            // 错误消息
            const char* error_message() const {
                return error_message_.c_str();
            }
            // 受影响的行数
            uint64_t affected_rows() const {
                return affected_rows_;
            }
            // 数据行数
            uint32_t rows_count() const {
                return rowreaders_.size();
            }
            // 上次插入的自增 ID
            uint64_t last_insert_id() const {
                return last_insert_id_;
            }
            const ResultSetMetadata& metadata() const {
                return metadata_;
            }

            // 读取下一条记录
            bool Next() {
                if (++rowreaders_index_ < (int32_t)rowreaders_.size()) {
                    return true;
                } else {
                    return false;
                }
            }
            
            bool GetBoolean(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetBoolean(column - 1);
            }

            int8_t GetInt8(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetInt8(column - 1);
            }

            uint8_t GetUInt8(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetUInt8(column - 1);
            }

            int16_t GetInt16(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetInt16(column - 1);
            }

            uint16_t GetUInt16(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetUInt16(column - 1);
            }

            int32_t GetInt32(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetInt32(column - 1);
            }

            uint32_t GetUInt32(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetUInt32(column - 1);
            }

            int64_t GetInt64(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetInt64(column - 1);
            }

            uint64_t GetUInt64(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetUInt64(column - 1);
            }

            std::string GetString(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetString(column - 1);
            }

            float GetFloat(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetFloat(column - 1);
            }

            double GetDouble(uint32_t column) {
                return rowreaders_[rowreaders_index_]->GetDouble(column - 1);
            }

            bool IsNull(uint32_t column) {
                return rowreaders_[rowreaders_index_]->IsNull(column - 1);
            }

            bool Parse(internal::PacketIn& pktin, internal::ProtocolType protocol);

        private:
            uint16_t error_code_;
            std::string error_state_;
            std::string error_message_;
            uint64_t affected_rows_;
            uint64_t last_insert_id_;
            uint16_t status_flag_;
            uint16_t warnings_;
            std::string info_;
            ResultSetMetadata metadata_;
            enum ParsePhase {
                PARSE_NEW,
                PARSE_COLUMN_DEFINITION,
                PARSE_RESULTSET_ROW,
                PARSE_FINISH,
            };
            ParsePhase phase_;
            std::vector<internal::RowReader*> rowreaders_;
            int32_t rowreaders_index_;
            friend class Statement;
            friend class PreparedStatement;
        };
    }
}

#endif // RESULTSET_H
