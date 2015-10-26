#include "httpclient.h"
#include "../3rd/http_parser/http_parser.h"
#include "../gateway/packet_base.h"
#include <netdb.h>
#include <stdio.h>
#include <iostream>

namespace base
{
    namespace http
    {
        using namespace std;

        class PacketHttpRequest : public gateway::PacketOutBase
        {
        public:
            PacketHttpRequest(uint32_t approx_size, memory::MemoryPool& mempool)
                : gateway::PacketOutBase(approx_size, mempool) {
                SkipTo(0);
            }
        };

        int HttpClient::message_begin_cb(http_parser* parser)
        {
            HttpClient* client = static_cast<HttpClient*>(parser->data);
            client->resp_body_.reserve(1024);
            return 0;
        }

        int HttpClient::body_cb(http_parser* parser, const char* at, size_t len)
        {
            HttpClient* client = static_cast<HttpClient*>(parser->data);
            client->resp_body_.append(at, len);
            return 0;
        }

        int HttpClient::message_complete_cb(http_parser* parser)
        {
            HttpClient* client = static_cast<HttpClient*>(parser->data);
            client->handler_.OnHttpResponse(client, client->resp_body_.c_str());
            // 主动关闭, TODO 后期可优化keep-alive,制作一个HttpClient池
            client->Close();
            return 0;
        }

        static struct http_parser_settings settings;

        static struct http_parser_settings_initializer {
            http_parser_settings_initializer() {
                memset(&settings, 0, sizeof(settings));
                settings.on_body = HttpClient::body_cb;
                settings.on_message_begin = HttpClient::message_begin_cb;
                settings.on_message_complete = HttpClient::message_complete_cb;
            }
        } _settings_initializer;

        HttpClient::HttpClient(memory::MemoryPool& mempool, HttpClientEventHandler& handler) : net::Client(mempool), handler_(handler), req_(new PacketHttpRequest(120, mempool))
        {
            http_parser_init(&parser_, HTTP_RESPONSE);
            parser_.data = this;
        }

        HttpClient::~HttpClient()
        {
            SAFE_DELETE(req_);
        }

        void HttpClient::SendAsync(const char* uri0, HttpMethod method, const char* querystring)
        {
            http_parser_url uri;
            if (http_parser_parse_url(uri0, strlen(uri0), 0, &uri)) {
                handler_.OnHttpError(this, "parser uri fail");
                return;
            } else {
                if ((uri.field_set & (1 << UF_HOST)) == 0) {
                    handler_.OnHttpError(this, "parser uri fail: no host");
                    return;
                }
                string host(uri0 + uri.field_data[UF_HOST].off, uri.field_data[UF_HOST].len);
                int port = uri.port == 0 ? 80 : uri.port;

                string path("/");
                if ((uri.field_set & (1 << UF_PATH)) != 0) {
                    path.assign(uri0 + uri.field_data[UF_PATH].off, uri.field_data[UF_PATH].len);
                }
                //cout << "req:" << host << "," << path << "," << querystring << endl;

                WriteHttpRequest(method, host, path.c_str(), querystring);
                Connect(host.c_str(), port);
            }
        }

        void HttpClient::OnReceive(std::size_t count)
        {
            vector<memory::RefMemoryChunk> data;
            PopReceive(data, count);
            for (vector<memory::RefMemoryChunk>::iterator it = data.begin(); it != data.end(); ++it) {
                size_t r = http_parser_execute(&parser_, &settings, (*it).data(), (*it).count());
                if (r != (*it).count()) {
                    handler_.OnHttpError(this, "parser error");
                    return;
                }
            }
        }

        void HttpClient::OnConnect()
        {
            Flush();
        }

        void HttpClient::OnConnectFail(int eno, const char* reason)
        {
            handler_.OnHttpError(this, reason);
        }

        void HttpClient::OnClose()
        {
            handler_.OnHttpClose(this);
        }

        void HttpClient::WriteHttpRequest(HttpMethod method, const string& host, const string& path, const char* query)
        {
            size_t query_len = strlen(query);
            if (method == HTTP_METHOD_GET) {
                req_->WriteRaw("GET ", 4);
                req_->WriteRaw(path.c_str(), path.length());
                req_->WriteRaw("?", 1);
                req_->WriteRaw(query, query_len);
                req_->WriteRaw(" HTTP/1.1\r\n", 11);
                req_->WriteRaw("HOST: ", 6);
                req_->WriteRaw(host.c_str(), host.length());
                req_->WriteRaw("\r\n", 2);
                req_->WriteRaw("\r\n", 2);
            } else {
                req_->WriteRaw("POST ", 5);
                req_->WriteRaw(path.c_str(), path.length());
                req_->WriteRaw(" HTTP/1.1\r\n", 11);
                req_->WriteRaw("HOST: ", 6);
                req_->WriteRaw(host.c_str(), host.length());
                req_->WriteRaw("\r\n", 2);
                req_->WriteRaw("Content-Type: application/x-www-form-urlencoded\r\n", 49);
                req_->WriteRaw("Content-Length: ", 16);
                char temp[32];
                sprintf(temp, "%lu", query_len);
                req_->WriteRaw(temp, strlen(temp));
                req_->WriteRaw("\r\n\r\n", 4);
                req_->WriteRaw(query, query_len);
            }
        }

        void HttpClient::Flush()
        {
            std::vector<memory::RefMemoryChunk> data = req_->FetchData();
            PushSend(data);
        }
    }
}
