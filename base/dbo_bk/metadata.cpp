#include "metadata.h"
#include "internal/packet.h"
#include <iostream>

namespace base
{
    namespace dbo
    {
        using namespace std;

        void ColumnDefinition::Dump() const
        {
            cout << "column=> schema:" << schema << ", table:" << table << ", org_table:" << org_table
                 << ", name:" << name << ", org_name:" << org_name << ", type=" << type << ", flag=" << flag
                 << ", fixed_length:" << fixed_length << ", column_length:" << column_length << ", charset:"
                 << (uint32_t)charset << ", decimals:" << (uint32_t)decimals << endl;
        }

        void ColumnDefinition::Parse(internal::PacketIn& pktin)
        {
            pktin.ReadLengthEncodedString(catalog);
            pktin.ReadLengthEncodedString(schema);
            pktin.ReadLengthEncodedString(table);
            pktin.ReadLengthEncodedString(org_table);
            pktin.ReadLengthEncodedString(name);
            pktin.ReadLengthEncodedString(org_name);
            pktin.ReadVariableInteger(&fixed_length);
            pktin.ReadFixedInteger<2>(&charset);
            pktin.ReadFixedInteger<4>(&column_length);
            pktin.ReadFixedInteger<1>(&type);
            pktin.ReadFixedInteger<2>(&flag);
            pktin.ReadFixedInteger<1>(&decimals);
            pktin.ReadFixedInteger<2>(&filter);
            // COM_FIELD_LIST not support
        }
        
        void ResultSetMetadata::Dump() const
        {
            for (uint32_t i = 0; i < columns.size(); ++i) {
                columns[i].Dump();
            }
        }

        void PreparedStatementMetadata::Dump() const
        {
            cout << "preparedStatement: stmtid=" << statement_id << ", num_columns:" << num_columns << ", num_params:" << num_params << endl;
        }
    }
}
