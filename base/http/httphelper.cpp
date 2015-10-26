#include "httphelper.h"
#include <netdb.h>
#include "../logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
namespace base
{
    namespace http
    {
        HttpHelper::HttpHelper()
        {

        }

        HttpHelper::~HttpHelper()
        {

        }
        HttpHelper HttpHelper::instance()
        {
            static HttpHelper ins;
            return ins;
        }
        int HttpHelper::http_send_async ( const char* uri, HttpMethod method, const char* querystring, MemoryPool& pool, HttpClientEventHandler& handler )
        {
            base::http::HttpClient* client = new base::http::HttpClient ( pool , handler );
            base::http::HttpMethod method1 = ( method == base::http::HTTP_METHOD_GET ) ? base::http::HTTP_METHOD_GET : base::http::HTTP_METHOD_POST;
            client->SendAsync ( uri, method1, querystring );
            return 999;
        }

        int HttpHelper::http_send_async_bydomain ( const char* uri, HttpMethod method, const char* querystring, MemoryPool& pool, HttpClientEventHandler& handler )
        {
            //TODO 性能太差，需要优化
            struct hostent *host;
            if ( ( host=gethostbyname ( uri ) ) ==NULL ) {
                LOG_DEBUG ( "gethostbyname error" );
            }
            struct in_addr   temp=  * ( ( struct in_addr * ) host->h_addr );
            char *ip = inet_ntoa ( temp );
            std::string ipstr("http://");
            ipstr.append(ip);
            ipstr.append("/");
            return http_send_async(ipstr.c_str(),method,querystring,pool,handler);
        }



    }
}
