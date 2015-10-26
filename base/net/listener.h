#ifndef BASE_NET_LISTENER_H
#define BASE_NET_LISTENER_H

#include "../global.h"
#include "../event/eventio.h"

namespace base
{
    namespace net
    {
        class Client;
        class Listener;

        // 侦听
        class Listener : public event::EventIO
        {
        public:
            struct EventHandler {
                virtual ~EventHandler() {}
                virtual void OnListenerAccept(Listener* sender, int clientfd) = 0;
            };
            Listener(EventHandler& eventhandler) : eventhandler_(eventhandler) {}
            virtual ~Listener();

            // 侦听指定地址
            bool Bind(const char* ipaddr, int port);

        private:
            virtual void OnEventIOReadable();
            virtual void OnEventIOWriteable();
            virtual void OnEventIOClose() {}

            void OnAccept(int clientfd) {
                eventhandler_.OnListenerAccept(this, clientfd);
            }

            EventHandler& eventhandler_;
            DISABLE_COPY(Listener)
        };
    }
}
#endif // LISTENER_H
