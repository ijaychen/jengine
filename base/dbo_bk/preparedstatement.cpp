#include "preparedstatement.h"
#include "internal/realconnection.h"
#include "internal/packet.h"
#include "internal/realconnectionpool.h"
#include "connection.h"
#include "../logger.h"

#define mysql_unsigned(v) (0x8000 | v)     // unsigned flag

namespace base
{
    namespace dbo
    {
        using namespace std;
        using namespace base::dbo::internal;

        PreparedStatement::PreparedStatement(Connection& conn, const char* sql)
            : StatementBase(conn), sql_(sql), cursor_flag_(0), metadata_pstmt_(nullptr), phase_(PREPARE_NEW),
              params_(new PacketOut(RealConnectionPoolManager::instance().mempool(), 200)), params_idx_(0)
        {
            params_->SkipTo(0);
        }

        PreparedStatement::~PreparedStatement()
        {
            SAFE_DELETE(params_);
        }

        void PreparedStatement::SetNull()
        {
            EnsureParamBitmapCapacity();
            uint32_t byte_pos = params_idx_ / 8;
            uint32_t bit_pos = params_idx_ % 8;
            bitmap_[byte_pos] |= 1 << bit_pos;

            params_type_.push_back(MYSQL_TYPE_NULL);
            ++params_idx_;
        }

        void PreparedStatement::SetInt8(int8_t param)
        {
            params_->WriteFixedInteger<1>(param);

            params_type_.push_back(MYSQL_TYPE_TINY);
            ++params_idx_;
        }

        void PreparedStatement::SetUInt8(uint8_t param)
        {
            params_->WriteFixedInteger<1>(param);

            params_type_.push_back(mysql_unsigned(MYSQL_TYPE_TINY));
            ++params_idx_;
        }

        void PreparedStatement::SetInt16(int16_t param)
        {
            params_->WriteFixedInteger<2>(param);

            params_type_.push_back(MYSQL_TYPE_SHORT);
            ++params_idx_;
        }

        void PreparedStatement::SetUInt16(uint16_t param)
        {
            params_->WriteFixedInteger<2>(param);

            params_type_.push_back(mysql_unsigned(MYSQL_TYPE_SHORT));
            ++params_idx_;
        }

        void PreparedStatement::SetInt32(int32_t param)
        {
            params_->WriteFixedInteger<4>(param);

            params_type_.push_back(MYSQL_TYPE_LONG);
            ++params_idx_;
        }

        void PreparedStatement::SetUInt32(uint32_t param)
        {
            params_->WriteFixedInteger<4>(param);

            params_type_.push_back(mysql_unsigned(MYSQL_TYPE_LONG));
            ++params_idx_;
        }

        void PreparedStatement::SetInt64(int64_t param)
        {
            params_->WriteFixedInteger<8>(param);

            params_type_.push_back(MYSQL_TYPE_LONGLONG);
            ++params_idx_;
        }

        void PreparedStatement::SetUInt64(uint64_t param)
        {
            params_->WriteFixedInteger<8>(param);

            params_type_.push_back(mysql_unsigned(MYSQL_TYPE_LONGLONG));
            ++params_idx_;
        }

        void PreparedStatement::SetString(const string& param)
        {
            params_->WriteLengthEncodedString(param);

            params_type_.push_back(MYSQL_TYPE_VARCHAR);
            ++params_idx_;
        }

        void PreparedStatement::SetString(const char* param, uint32_t len)
        {
            params_->WriteLengthEncodedString(param, len);

            params_type_.push_back(MYSQL_TYPE_VARCHAR);
            ++params_idx_;
        }

        void PreparedStatement::SetBinary(const string& param)
        {
            params_->WriteLengthEncodedString(param);

            params_type_.push_back(MYSQL_TYPE_STRING);
            ++params_idx_;
        }

        void PreparedStatement::SetFloat(float param)
        {
            params_->WriteFixedInteger<4>(param);

            params_type_.push_back(MYSQL_TYPE_FLOAT);
            ++params_idx_;
        }

        void PreparedStatement::SetDouble(double param)
        {
            params_->WriteFixedInteger<8>(param);

            params_type_.push_back(MYSQL_TYPE_DOUBLE);
            ++params_idx_;
        }

        void PreparedStatement::SetDateTime(const tm& dt)
        {
            if (dt.tm_hour != 0 || dt.tm_min != 0 || dt.tm_sec != 0) {
                params_->WriteFixedInteger<1>(7);
                params_->WriteFixedInteger<2>(dt.tm_year);
                params_->WriteFixedInteger<1>(dt.tm_mon);
                params_->WriteFixedInteger<1>(dt.tm_mday);
                params_->WriteFixedInteger<1>(dt.tm_hour);
                params_->WriteFixedInteger<1>(dt.tm_min);
                params_->WriteFixedInteger<1>(dt.tm_sec);
            } else {
                if (dt.tm_year == 0 && dt.tm_mon == 0 && dt.tm_mday == 0) {
                    params_->WriteFixedInteger<1>(0);
                } else {
                    params_->WriteFixedInteger<1>(4);
                    params_->WriteFixedInteger<2>(dt.tm_year);
                    params_->WriteFixedInteger<1>(dt.tm_mon);
                    params_->WriteFixedInteger<1>(dt.tm_mday);
                }
            }
            params_type_.push_back(MYSQL_TYPE_DATETIME);
            ++params_idx_;
        }

        void PreparedStatement::Execute(const boost::function<void(ResultSet&)>& cb)
        {
            phase_ = PREPARE_WAIT;
            cb_ = cb;
            CallRealExecute();
        }

        void PreparedStatement::OnCallRealExecute()
        {
            metadata_pstmt_ = realconn()->FetchCachedPreparedStatement(sql_);
            if (metadata_pstmt_ == nullptr) {
                phase_ = PREPARE_WAIT_PREPARE_RESPONSE;
                PacketOut pktout(realconn()->mempool(), 30, FetchSequenceID());
                pktout.WriteFixedInteger<1>(COM_STMT_PREPARE);
                pktout.WriteFixedString(sql_);
                realconn()->Send(pktout);
            } else {
                HandlePrepareFinish();
            }
        }

        static void parse_prepare_ok(internal::PacketIn& pktin, PreparedStatementMetadata& meta)
        {
            pktin.ReadFixedInteger<4>(&meta.statement_id);
            pktin.ReadFixedInteger<2>(&meta.num_columns);
            pktin.ReadFixedInteger<2>(&meta.num_params);
            //
            pktin.Skip(1);
            uint16_t warning_count = 0;
            pktin.ReadFixedInteger<2>(&warning_count);
            if (warning_count != 0) {
                LOG_WARN("prepare with %u warnings count\n", warning_count);
            }
            //
            meta.columns_def.reserve(meta.num_columns);
            meta.params_def.reserve(meta.num_params);
        }

        void PreparedStatement::HandleResponse(internal::PacketIn& pktin)
        {
            switch (phase_) {
                case PREPARE_WAIT_PREPARE_RESPONSE: {
                    uint8_t flag;
                    pktin.ReadFixedInteger<1>(&flag);
                    if (flag == PACKET_OK) {
                        metadata_pstmt_ = new PreparedStatementMetadata;
                        realconn()->SavePreparedStatementCache(sql_, metadata_pstmt_);
                        parse_prepare_ok(pktin, *metadata_pstmt_);
                        if (metadata_pstmt_->num_params > 0) {
                            phase_ = PREPARE_WAIT_PREPARE_RESPONSE_PARAM_DEF;
                        } else if (metadata_pstmt_->num_columns > 0) {
                            phase_ = PREPARE_WAIT_PREPARE_RESPONSE_COLUMN_DEF;
                        } else {
                            phase_ = PREPARE_PREPARE_OK;
                            HandlePrepareFinish();
                        }
                    } else if (flag == PACKET_ERR) {
                        pktin.ReadFixedInteger<2>(&rs_.error_code_);
                        pktin.ReadFixedString(rs_.error_state_, 6);
                        pktin.ReadRestOfPacketString(rs_.error_message_);
                        HandleFinish();
                    } else {
                        LOG_ERROR("bad response: %u\n", flag);
                    }
                }
                break;
                case PREPARE_WAIT_PREPARE_RESPONSE_PARAM_DEF: {
                    uint8_t flag;
                    pktin.ReadFixedInteger<1>(&flag);
                    if (flag == PACKET_EOF) {
                        if (metadata_pstmt_->num_columns > 0) {
                            phase_ = PREPARE_WAIT_PREPARE_RESPONSE_COLUMN_DEF;
                        } else {
                            phase_ = PREPARE_PREPARE_OK;
                            HandlePrepareFinish();
                        }
                    } else {
                        pktin.Skip(-1);
#ifdef HAS_CXX11
                        metadata_pstmt_->params_def.emplace_back();
#else
                        metadata_pstmt_->params_def.push_back(ColumnDefinition());
#endif
                        metadata_pstmt_->params_def.back().Parse(pktin);
                    }
                }
                break;
                case PREPARE_WAIT_PREPARE_RESPONSE_COLUMN_DEF: {
                    uint8_t flag;
                    pktin.ReadFixedInteger<1>(&flag);
                    if (flag == PACKET_EOF) {
                        phase_ = PREPARE_PREPARE_OK;
                        HandlePrepareFinish();
                    } else {
                        pktin.Skip(-1);
#ifdef HAS_CXX11
                        metadata_pstmt_->columns_def.emplace_back();
#else
                        metadata_pstmt_->columns_def.push_back(ColumnDefinition());
#endif
                        metadata_pstmt_->columns_def.back().Parse(pktin);
                    }
                }
                break;
                case PREPARE_WAIT_EXEC_RESPONSE: {
                    if (rs_.Parse(pktin, dbo::internal::PROTOCOL_BINARY)) {
                        HandleFinish();
                    }
                }
                break;
                default: {
                    throw Exception("dbo: unexception packet receive");
                }
                break;
            }
        }

        void PreparedStatement::HandleClose()
        {
            rs_.error_code_ = 1;
            rs_.error_message_ = "mysql server disconnected";
            if (cb_) {
                cb_(rs_);
            }
        }

        void PreparedStatement::HandleFinish()
        {
            phase_ = PREPARE_FINISH;
            if (rs_.HasError()) {
                LOG_ERROR("dbo error: code= %u, %s, sql=%s\n", rs_.error_code(), rs_.error_message(), sql_.c_str());
            }
            cb_(rs_);
        }

        void PreparedStatement::HandlePrepareFinish()
        {
            assert(metadata_pstmt_->num_columns == metadata_pstmt_->columns_def.size());
            assert(metadata_pstmt_->num_params == metadata_pstmt_->params_def.size());
            if (metadata_pstmt_->num_params != params_idx_) {
                throw Exception("dbo: param size missmatch!");
            }
            ResetSequenceID();
            PacketOut pktout(realconn()->mempool(), 100, FetchSequenceID());
            pktout.WriteFixedInteger<1>(COM_STMT_EXECUTE);
            pktout.WriteFixedInteger<4>(metadata_pstmt_->statement_id);
            pktout.WriteFixedInteger<1>(cursor_flag_);
            pktout.WriteFixedInteger<4>(1);

            if (metadata_pstmt_->num_params > 0) {
                // null bitmap
                EnsureParamBitmapCapacity();
                pktout.WriteFixedString(bitmap_.data(), bitmap_.size());
                pktout.WriteFixedInteger<1>(1); // bound flag

                // params type
                for (uint32_t i = 0; i < params_type_.size(); ++i) {
                    pktout.WriteFixedInteger<2>(params_type_[i]);
                }

                // params value
                pktout.AppendData(params_->FetchData());
            } else {
                pktout.WriteFixedInteger<1>(0); // bound flag
            }

            realconn()->Send(pktout);
            phase_ = PREPARE_WAIT_EXEC_RESPONSE;
        }

    }
}
