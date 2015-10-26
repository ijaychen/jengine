#include "statement.h"
#include "../logger.h"
#include "resultset.h"
#include "connection.h"
#include "internal/packet.h"
#include "internal/common.h"
#include "internal/realconnection.h"
#include "internal/rowreadertext.h"
#include "resultset.h"

namespace base
{
    namespace dbo
    {
        using namespace std;
        using namespace base::dbo::internal;

        /// StatementBase
        StatementBase::~StatementBase()
        {
        }

        void StatementBase::CallRealExecute()
        {
            if (IsWait() && conn_.realconn_ != nullptr) {
                realconn_ = conn_.realconn_;    // set realconn
                OnCallRealExecute();
            }
        }

        void StatementBase::HandleRawResponse(PacketIn& pktin)
        {
            if (sequenct_id_ == pktin.sequence_id()) {
                ++sequenct_id_;
                HandleResponse(pktin);
            } else {
                LOG_WARN("mysql packet sequence_id mismatch! except:%u, current:%u\n", sequenct_id_, pktin.sequence_id());
            }
        }

        /// Statement
        Statement::~Statement()
        {
        }

        void Statement::Execute(const boost::function<void(ResultSet&)>& cb)
        {
            assert(phase_ == STATEMENT_NEW);
            phase_ = STATEMENT_WAIT;
            cb_ = cb;
            CallRealExecute();
        }

        void Statement::OnCallRealExecute()
        {
            phase_ = STATEMENT_WAIT_RESPONSE;
            PacketOut pktout(realconn()->mempool(), 20, FetchSequenceID());
            pktout.WriteFixedInteger<1>(internal::COM_QUERY);
            pktout.WriteFixedString(sql_);
            realconn()->Send(pktout);
        }

        void Statement::HandleResponse(PacketIn& pktin)
        {
            if (phase_ != STATEMENT_WAIT_RESPONSE) {
                LOG_ERROR("unexpected response\n");
                return;
            }
            if (rs_.Parse(pktin, PROTOCOL_TEXT)) {
                phase_ = STATEMENT_FINISH;
                cb_(rs_);
            }
        }

        void Statement::HandleClose()
        {
            rs_.error_code_ = 1;
            rs_.error_message_ = "mysql server disconnected";
            if (cb_) {
                cb_(rs_);
            }
        }
    }
}
