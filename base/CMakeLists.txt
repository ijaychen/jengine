set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -O3 -std=c++98")

include_directories(${DIR_LUA_INCLUDE} ${DIR_BOOST_INCLUDE})
include_directories("/home/jason/software/boost_1_53_0")

add_library(base  SHARED
3rd/http_parser/http_parser.c
3rd/tadns-1.1/tadns.c
event/eventio.cpp
event/eventtimeout.cpp
event/dispatcher.cpp
utils/crypto.cpp
utils/random.cpp
utils/string.cpp
utils/time.cpp
utils/configure.cpp
thread/task.cpp 
thread/threadpool.cpp
command/runner.cpp
command/runnermgr.cpp 
file/utils.cpp
logger.cpp
memory/memorychunk.cpp 
memory/memorypool.cpp
net/utils.cpp
net/client.cpp
net/listener.cpp
gateway/usersession.cpp
gateway/packet_base.cpp 
gateway/packet.cpp
gateway/userclient.cpp
gateway/gateway.cpp
http/httpclient.cpp
framework.cpp
objectmaintainer.cpp  
object.cpp
exception.cpp 
autoreleasepool.cpp 
http/httphelper.cpp 
)
