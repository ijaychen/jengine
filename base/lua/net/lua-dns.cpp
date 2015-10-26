#include "lua-dns.h"
#include "../../global.h"
extern "C" {
#include "../../3rd/tadns-1.1/tadns.h"
}
#include "../../event/eventio.h"
#include "../../logger.h"
#include <lua.hpp>
#include <iostream>

namespace base
{
    namespace lua
    {
        namespace net
        {
            using namespace std;

            class DnsPoller : public base::event::EventIO
            {
            public:
                DnsPoller() : dns_(nullptr) {
                    dns_ = dns_init();
                }
                virtual ~DnsPoller() {
                    dns_fini(dns_);
                    dns_ = nullptr;
                }

                dns* tadns() {
                    return dns_;
                }

                void Open() {
                    int fd = dns_get_fd(dns_);
                    AddToDispatcher(fd, base::event::IO_READABLE);
                }

            private:
                virtual void OnEventIOReadable() {
                    dns_poll(dns_);
                }
                virtual void OnEventIOWriteable() {
                }
                virtual void OnEventIOClose() {
                }

            private:
                dns* dns_;
            };

            static const char KEY_QUERY_LIST = 'k';
            static int32_t query_id_cur = 0;

            static DnsPoller* g_dns_poller = nullptr;

            struct query_context_t {
                lua_State* L0;
                int32_t callback_id;
                int32_t flag;
            };

            static void query_callback(dns_cb_data* data)
            {
                query_context_t* ctx = static_cast<query_context_t*>(data->context);
                lua_State* L;
                lua_State* L0 = ctx->L0;

                if (ctx->flag == 2) {
                    L0 = ctx->L0;
                    lua_rawgetp(L0, LUA_REGISTRYINDEX, &KEY_QUERY_LIST);
                    lua_rawgeti(L0, -1, ctx->callback_id);
                    if (!lua_isthread(L0, -1)) {
                        delete ctx;
                        return;
                    }
                    L = lua_tothread(L0, -1);
                } else {
                    L = ctx->L0;
                    ctx->flag = 1;
                }

                switch (data->error) {
                    case DNS_OK:
                        lua_pushboolean(L, true);
                        lua_pushfstring(L, "%d.%d.%d.%d",
                                        data->addr[0],
                                        data->addr[1],
                                        data->addr[2],
                                        data->addr[3]);
                        break;
                    case DNS_DOES_NOT_EXIST:
                        lua_pushboolean(L, false);
                        lua_pushstring(L, "DNS_DOES_NOT_EXIST");
                        break;
                    case DNS_TIMEOUT:
                        lua_pushboolean(L, false);
                        lua_pushstring(L, "DNS_TIMEOUT");
                        break;
                    case DNS_ERROR:
                        lua_pushboolean(L, false);
                        lua_pushstring(L, "DNS_ERROR");
                        break;
                }

                if (ctx->flag == 2) {
                    int r = lua_resume(L, L0, 2);
                    if (r != LUA_OK && r != LUA_YIELD) {
                        LOG_ERROR2("script error: %s\n", lua_tostring(L, -1));
                    }
                    lua_rawgetp(L0, LUA_REGISTRYINDEX, &KEY_QUERY_LIST);
                    lua_pushnil(L0);
                    lua_rawseti(L0, -2, ctx->callback_id);
                    delete ctx;
                }
            }

            static int query(lua_State* L)
            {
                const char* domain = luaL_checkstring(L, 1);
                query_context_t* ctx = new query_context_t;
                ctx->flag = 0;
                ctx->L0 = L;

                dns_queue(g_dns_poller->tadns(), ctx, domain, DNS_A_RECORD, &query_callback);
                if (ctx->flag == 0) {
                    // 拿到主线程的lua_State
                    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                    ctx->L0 = lua_tothread(L, -1);
                    int32_t callback_id = query_id_cur++;
                    ctx->callback_id = callback_id;
                    // 保存当前lua_State
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_QUERY_LIST);
                    lua_pushthread(L);
                    lua_rawseti(L, -2, callback_id);
                    ctx->flag = 2;
                    return lua_yield(L, 0);
                } else {
                    delete ctx;
                    return 2;
                }
            }

            static int _luaopen_dns(lua_State* L)
            {
                g_dns_poller = new DnsPoller;
                g_dns_poller->Open();

                lua_newtable(L);
                lua_newtable(L);
                lua_pushstring(L, "v");
                lua_setfield(L, -2, "__mode");
                lua_setmetatable(L, -2);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_QUERY_LIST);

                luaL_Reg dns_funcs[] = {
                    {"query", query},
                    {nullptr, nullptr}
                };
                luaL_newlib(L, dns_funcs);
                return 1;
            }

            void luaopen_dns(lua_State* L)
            {
                luaL_requiref(L, "dns", &_luaopen_dns, 0);
                lua_pop(L, 1);
            }

            void luaclose_dns(lua_State* L)
            {
                g_dns_poller->Close();
                SAFE_RELEASE(g_dns_poller);
            }
        }
    }
}
