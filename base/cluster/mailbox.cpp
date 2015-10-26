#include "mailbox.h"
#include "nodemonitor.h"

namespace base
{
    namespace cluster
    {
        Mailbox::Mailbox(Mailbox::EventHandler& handler, const char* name)
            : mempool_(NodeMonitor::instance().mempool()), handler_(handler), name_(name) {}

        Mailbox::~Mailbox()
        {
            NodeMonitor::instance().UnRegister(this);
        }

        Mailbox* Mailbox::Create(Mailbox::EventHandler& handler, const char* name, bool sys)
        {
            Mailbox* mb = new Mailbox(handler, name);
            if (!NodeMonitor::instance().Register(mb, sys)) {
                delete mb;
                mb = nullptr;
            }
            return mb;
        }

        void Mailbox::Cast(const MailboxID& to, MessageOut& msgout)
        {
            msgout.SetFrom(mbid_);
            msgout.SetTo(to);
            NodeMonitor::instance().SendMessage(msgout);
        }
    }
}

