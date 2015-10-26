#ifndef HTTPHELPER_H
#define HTTPHELPER_H
#include "../memory/memorypool.h"
#include "../http/httpclient.h"

#include "httpclient.h"
namespace base
{
    namespace http
    {
        using base::memory::MemoryPool;
        class HttpHelper
        {
        public:
            static HttpHelper instance();
            virtual ~HttpHelper();
            int http_send_async_bydomain( const char* uri, base::http::HttpMethod method,   const char* querystring, MemoryPool& pool, base::http::HttpClientEventHandler&  handler );      
            int http_send_async ( const char* uri, base::http::HttpMethod method,   const char* querystring, MemoryPool& pool, base::http::HttpClientEventHandler&  handler );
          
        private:
            HttpHelper();
        };
    }
}

#endif // HTTPHELPER_H
