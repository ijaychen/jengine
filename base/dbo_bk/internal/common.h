#ifndef BASE_DBO_INTERNAL_COMMON_H
#define BASE_DBO_INTERNAL_COMMON_H

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            // mysql中的常量定义

            // 服务器状态
            enum MysqlServerStatus {
                SERVER_STATUS_IN_TRANS = 0x0001,
                SERVER_STATUS_AUTOCOMMIT = 0x0002 ,
                SERVER_MORE_RESULTS_EXISTS = 0x0008,
                SERVER_STATUS_NO_GOOD_INDEX_USED = 0x0010,
                SERVER_STATUS_NO_INDEX_USED = 0x0020,
                SERVER_STATUS_CURSOR_EXISTS = 0x0040,
                SERVER_STATUS_LAST_ROW_SENT = 0x0080,
                SERVER_STATUS_DB_DROPPED = 0x0100,
                SERVER_STATUS_NO_BACKSLASH_ESCAPES = 0x0200,
                SERVER_STATUS_METADATA_CHANGED = 0x0400,
                SERVER_QUERY_WAS_SLOW = 0x0800,
                SERVER_PS_OUT_PARAMS = 0x1000,
            };

            // 特性
            enum MysqlCapabilityType {
                CLIENT_CONNECT_WITH_DB = 0x00000008,
                CLIENT_IGNORE_SPACE = 0x00000100,
                CLIENT_PROTOCOL_41 = 0x00000200,
                CLIENT_INTERACTIVE = 0x00000400,
                CLIENT_SSL = 0x00000800,
                CLIENT_TRANSACTIONS = 0x00002000,
                CLIENT_SECURE_CONNECTION = 0x00008000,
                CLIENT_MULTI_STATEMENTS = 0x00010000,
                CLIENT_MULTI_RESULTS = 0x00020000,
                CLIENT_PS_MULTI_RESULTS = 0x00040000, // TODO
                CLIENT_PLUGIN_AUTH = 0x00080000,
                CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA = 0x00200000,
            };

            // 命令
            enum MysqlCommandType {
                COM_SLEEP = 0x00,
                COM_QUIT = 0x01,
                COM_INIT_DB = 0x02,
                COM_QUERY = 0x03,               // *
                COM_FIELD_LIST = 0x04,
                COM_CREATE_DB = 0x05,
                COM_DROP_DB = 0x06,
                COM_REFRESH = 0x07,
                COM_SHUTDOWN = 0x08,
                COM_STATISTICS = 0x09,
                COM_PROCESS_INFO = 0x0a,
                COM_CONNECT = 0x0b,
                COM_PROCESS_KILL = 0x0c,
                COM_DEBUG = 0x0d,
                COM_PING = 0x0e,
                COM_TIME = 0x0f,
                COM_DELAYED_INSERT = 0x10,
                COM_CHANGE_USER = 0x11,
                COM_BINLOG_DUMP = 0x12,
                COM_TABLE_DUMP = 0x13,
                COM_CONNECT_OUT = 0x14,
                COM_REGISTER_SLAVE = 0x15,
                COM_STMT_PREPARE = 0x16,        // *
                COM_STMT_EXECUTE = 0x17,        // *
                COM_STMT_SEND_LONG_DATA = 0x18, // *
                COM_STMT_CLOSE = 0x19,          // *
                COM_STMT_RESET = 0x1a,          // *
                COM_SET_OPTION = 0x1b,
                COM_STMT_FETCH = 0x1c,          // *
                COM_DAEMON = 0x1d,
                COM_BINLOG_DUMP_GTID = 0x1e,
            };

            // 协议头标识
            enum MysqlPacketFlag {
                PACKET_OK = 0x00,
                PACKET_ERR = 0xFF,
                PACKET_EOF = 0xFE,
            };

            // 字段类型
            enum MysqlFieldType {
                MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
                MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
                MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
                MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
                MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
                MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
                MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
                MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
                MYSQL_TYPE_BIT,
                MYSQL_TYPE_NEWDECIMAL = 246,
                MYSQL_TYPE_ENUM = 247,
                MYSQL_TYPE_SET = 248,
                MYSQL_TYPE_TINY_BLOB = 249,
                MYSQL_TYPE_MEDIUM_BLOB = 250,
                MYSQL_TYPE_LONG_BLOB = 251,
                MYSQL_TYPE_BLOB = 252,
                MYSQL_TYPE_VAR_STRING = 253,
                MYSQL_TYPE_STRING = 254,
                MYSQL_TYPE_GEOMETRY = 255
            };

            // 列标记
            enum MysqlColumnFlags {
                MYSQL_COLUMN_FLAG_NOT_NULL = 1,
                MYSQL_COLUMN_FLAG_PRIMARY_KEY = 2,
                MYSQL_COLUMN_FLAG_UNIQUE_KEY = 4,
                MYSQL_COLUMN_FLAG_MULTIPLE_KEY = 8,
                MYSQL_COLUMN_FLAG_BLOB = 16,
                MYSQL_COLUMN_FLAG_UNSIGNED = 32,
                MYSQL_COLUMN_FLAG_ZERO_FILL = 64,
                MYSQL_COLUMN_FLAG_BINARY = 128,
                MYSQL_COLUMN_FLAG_ENUM = 256,
                MYSQL_COLUMN_FLAG_AUTO_INCREMENT = 512,
                MYSQL_COLUMN_FLAG_TIMESTAMP = 1024,
                MYSQL_COLUMN_FLAG_SET = 2048,
                MYSQL_COLUMN_FLAG_NUMBER = 32768
            };

            enum CursorType {
                CURSOR_TYPE_NO_CURSOR = 0x00,
                CURSOR_TYPE_READ_ONLY = 0x01,
                CURSOR_TYPE_FOR_UPDATE = 0x02,
                CURSOR_TYPE_SCROLLABLE = 0x04,
            };

            enum ProtocolType {
                PROTOCOL_TEXT,
                PROTOCOL_BINARY,
            };

            enum CharacterSet {
                CHARSET_LATIN1 = 8,
                CHARSET_UTF8 = 33,
                CHARSET_BINARY = 63,
            };
        };
    }
}

#endif
