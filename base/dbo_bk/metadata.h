#ifndef BASE_DBO_METADATA_H
#define BASE_DBO_METADATA_H

#include "../global.h"
#include "internal/common.h"
#include <string>
#include <vector>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class PacketIn;
        }

        // 列定义元数据
        struct ColumnDefinition {
            std::string catalog;
            std::string schema;
            std::string table;
            std::string org_table;
            std::string name;
            std::string org_name;
            uint16_t fixed_length;
            uint16_t charset;
            uint32_t column_length;
            internal::MysqlFieldType type;
            uint16_t flag;              // MysqlColumnFlags bitset
            uint8_t decimals;
            uint16_t filter;

            bool IsUnsigned() const {
                return flag & internal::MYSQL_COLUMN_FLAG_UNSIGNED;
            }

            bool IsPrimaryKey() const {
                return flag & internal::MYSQL_COLUMN_FLAG_PRIMARY_KEY;
            }

            bool IsUniqueKey() const {
                return flag & internal::MYSQL_COLUMN_FLAG_UNIQUE_KEY;
            }

            bool IsAutoIncrement() const {
                return flag & internal::MYSQL_COLUMN_FLAG_AUTO_INCREMENT;
            }

            bool IsNotNull() const {
                return flag & internal::MYSQL_COLUMN_FLAG_NOT_NULL;
            }

            void Dump() const;
            void Parse(internal::PacketIn& pktin);
        };

        // 结果集元数据
        struct ResultSetMetadata {
            std::vector<ColumnDefinition> columns;
            uint32_t num_columns;

            const ColumnDefinition& GetColumnDefinition(uint32_t idx) const {
                return columns[idx];
            }
            const std::string& GetColumnName(uint32_t idx) const {
                return columns[idx].name;
            }
            void Dump() const;
        };

        // 预处理语句元数据
        struct PreparedStatementMetadata {
            uint32_t statement_id;
            uint16_t num_columns;
            uint16_t num_params;
            std::vector<ColumnDefinition> params_def;
            std::vector<ColumnDefinition> columns_def;

            void Dump() const;
        };
    }
}

#endif
