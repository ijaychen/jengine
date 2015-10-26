#include "resultset.h"
#include "internal/packet.h"
#include "internal/rowreadertext.h"
#include "internal/rowreaderbinary.h"
#include "../logger.h"
#include <iostream>

namespace base
{
    namespace dbo
    {
        using namespace std;
        using namespace base::dbo::internal;

        ResultSet::ResultSet()
            : error_code_(0), affected_rows_(0), last_insert_id_(0),
              status_flag_(0), warnings_(0), phase_(PARSE_NEW), rowreaders_index_(-1)
        {
        }

        ResultSet::~ResultSet()
        {
            for (vector<RowReader*>::iterator it = rowreaders_.begin(); it != rowreaders_.end(); ++it) {
                delete *it;
            }
            rowreaders_.clear();
        }

        bool ResultSet::Parse(internal::PacketIn& pktin, ProtocolType protocol)
        {
            switch (phase_) {
                case PARSE_NEW: {
                    uint8_t first = 0;
                    pktin.ReadFixedInteger<1>(&first);
                    switch (first) {
                        case PACKET_OK: {
                            // OK_Packet
                            pktin.ReadVariableInteger(&affected_rows_);
                            pktin.ReadVariableInteger(&last_insert_id_);
                            // we only support CLIENT_PROTOCOL_41
                            pktin.ReadFixedInteger<2>(&status_flag_);
                            pktin.ReadFixedInteger<2>(&warnings_);
                            pktin.ReadRestOfPacketString(info_);
                            if (warnings_ > 0) {
                                LOG_ERROR("SQL warnings count:%u, %s\n", warnings_, info_.c_str());
                            }
                            phase_ = PARSE_FINISH;
                        }
                        break;
                        case PACKET_ERR: {
                            // ERR_Packet
                            pktin.ReadFixedInteger<2>(&error_code_);
                            pktin.ReadRestOfPacketString(error_message_);
                            phase_ = PARSE_FINISH;
                        }
                        break;
                        case 0xFB: {
                            // not support yet
                            error_code_ = 1;
                            error_message_ = "not support 0xFB";
                            phase_ = PARSE_FINISH;
                        }
                        break;
                        default: {
                            pktin.Skip(-1);
                            pktin.ReadVariableInteger(&metadata_.num_columns);
                            metadata_.columns.reserve(metadata_.num_columns);
                            phase_ = PARSE_COLUMN_DEFINITION;
                        }
                        break;
                    }
                }
                break;
                case PARSE_COLUMN_DEFINITION: {
                    uint8_t first = 0;
                    pktin.ReadFixedInteger<1>(&first);
                    if (first == PACKET_EOF) {
                        phase_ = PARSE_RESULTSET_ROW;
                    } else {
                        pktin.Skip(-1);
#ifdef HAS_CXX11
                        metadata_.columns.emplace_back();
#else
                        metadata_.columns.push_back(ColumnDefinition());
#endif
                        metadata_.columns.back().Parse(pktin);
                    }
                }
                break;
                case PARSE_RESULTSET_ROW: {
                    uint8_t first = 0;
                    pktin.ReadFixedInteger<1>(&first);
                    if (first == PACKET_EOF || first == PACKET_ERR) {
                        // TODO ? more resultset flag?  wraning?
                        phase_ = PARSE_FINISH;
                    } else {
                        RowReader* reader = nullptr;
                        if (protocol == dbo::internal::PROTOCOL_BINARY) {
                            reader = new RowReaderBinary(metadata_);
                        } else {
                            reader = new RowReaderText(metadata_);
                        }
                        rowreaders_.push_back(reader);
                        reader->ParseRowPacket(pktin);
                    }
                }
                break;
                default: {
                    throw Exception("unexpected, may be we receive another resultset, but we do not support yet");
                }
                break;
            }
            return phase_ == PARSE_FINISH;
        }
    }
}

