set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -O0 -std=c++98 -Wl,-rpath,../lib")
include_directories("/home/jason/workspace/server/server/")
include_directories("/home/jason/software/boost_1_53_0")
link_directories("../base")
add_executable(gateserver
gatewaymanager.cpp
gateserver.cpp
gateengine.cpp
gatesession.cpp
response.cpp
sessionmanager.cpp
)

target_link_libraries(gateserver base dl readline rt crypto)
