#ifndef BASE_BOOTABLE_H
#define BASE_BOOTABLE_H

#include <boost/function.hpp>

namespace base
{
    class Bootable
    {
    public:
        virtual ~Bootable() {}
        void BeginSetup(const boost::function<void(bool)>& cb) {
            if (cb_setup_) {
                cb(false);
                return;
            }
            cb_setup_ = cb;
            OnBeginSetup();
        }
        void BeginCleanup(const boost::function<void()>& cb) {
            if (cb_cleanup_) {
                return;
            }
            cb_cleanup_ = cb;
            OnBeginCleanup();
        }

    protected:
        void NotifySetupFinish(bool result) {
            cb_setup_(result);
        }
        void NotifyCleanupFinish() {
            cb_cleanup_();
        }

    private:
        virtual void OnBeginSetup() = 0;
        virtual void OnBeginCleanup() = 0;
        boost::function<void(bool)> cb_setup_;
        boost::function<void()> cb_cleanup_;
    };
}

#endif
