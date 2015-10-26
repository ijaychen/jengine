#ifndef RESPONSE_H
#define RESPONSE_H

#include <base/global.h>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>

namespace gateserver
{
    class PlayerSession;
    class Response
    {
    public:
        Response ( PlayerSession& ps );
        ~Response() {};

        void SetSession ( uint16_t session ) {
            session_ = session;
        }      
        //@reason 1.帐号在别处登录 2.服务器维护 3.检测到异常操作 4.内部错误
        void SendLogout ( uint8_t reason );
    private:
        PlayerSession& ps_;
        uint16_t session_;
    };
}
#endif // RESPONSE_H
