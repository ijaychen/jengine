#ifndef BASE_HTTP_HTTPCLIENT_H
#define BASE_HTTP_HTTPCLIENT_H

#include "../net/client.h"
#include "../3rd/http_parser/http_parser.h"

namespace base
{
    namespace http
    {
        enum HttpMethod {
            HTTP_METHOD_GET = 1,
            HTTP_METHOD_POST = 2,
        };

        class PacketHttpRequest;
        class HttpClient;

        struct HttpClientEventHandler {
            virtual ~HttpClientEventHandler() {}
            virtual void OnHttpClose(HttpClient* client) = 0;
            virtual void OnHttpResponse(HttpClient* client, const char* body) = 0;
            virtual void OnHttpError(HttpClient* client, const char* msg) = 0;
        };

        class HttpClient : public net::Client
        {
        public:
            HttpClient(memory::MemoryPool& mempool, HttpClientEventHandler& handler);
            virtual ~HttpClient();

            void SendAsync(const char* uri, HttpMethod method, const char* querystring);

        private:
            virtual void OnConnect();
            virtual void OnConnectFail(int eno, const char* reason);
            virtual void OnReceive(std::size_t count);
            virtual void OnClose();
            HttpClientEventHandler& handler_;

            void WriteHttpRequest(HttpMethod method, const std::string& host, const std::string& path, const char* query);
            void Flush();
            
            PacketHttpRequest* req_;
            http_parser parser_;

            static int message_begin_cb(http_parser* parser);
            static int body_cb(http_parser* parser, const char* at, std::size_t len);
            static int message_complete_cb(http_parser* parser);
            std::string resp_body_;
            friend class http_parser_settings_initializer;
        };
    }
}

#endif // HTTPCLIENT_H
