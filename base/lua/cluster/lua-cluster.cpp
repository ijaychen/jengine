#include "lua-cluster.h"
#include "../../logger.h"
#include "../helper.h"
#include "../../cluster/rpcservice.h"
#include "../../cluster/message.h"
#include "../../cluster/rpcstub.h"
#include "../../cluster/nodemonitor.h"
#include "../internal/lua-event.h"
#include "../modulelua.h"
#include <boost/bind.hpp>

namespace base
{
    namespace lua
    {
        namespace cluster
        {
            using namespace std;

            static const char KEY_RPC_SERVICE_LIST = 'k';
            static const char KEY_RPC_STUB_LIST = 'k';
            static const char KEY_RPC_CORO_LIST = 'k';          // rpc call 而挂起的coroutine列表
            static const char KEY_MAILBOX_LIST = 'k';
            static const char KEY_ASYNC_FETCH_LIST = 'k';
            static const char* MT_RPC_STUB = "MT_RPC_STUB";
            static const char* MT_MAILBOX = "MT_MAILBOX";
            static const char* MT_ASYNC_FETCH_LINKER = "MT_ASYNC_FETCH_LINKER";
            static const char* MT_ASYNC_FETCH_WEAK_K = "MT_ASYNC_FETCH_WEAK_K";
            static int32_t rpc_stub_list_id_cur = 0;
            static int32_t rpc_stub_coro_id_cur = 0;
            static int32_t async_fetch_id_cur = 0;

            /// helper functions

            // 写入lua数据至message中
            void write_lua_value(base::cluster::MessageOut& msgout, lua_State* L, int idx)
            {
                lua_pushvalue(L, idx);
                int type = lua_type(L, -1);
                msgout.WriteSByte(type);
                switch (type) {
                    case LUA_TNIL:
                        break;
                    case LUA_TNUMBER:
                        msgout.WriteDouble(lua_tonumber(L, -1));
                        break;
                    case LUA_TBOOLEAN:
                        msgout.WriteBoolean(lua_toboolean(L, -1));
                        break;
                    case LUA_TSTRING: {
                        size_t len;
                        const char* v = lua_tolstring(L, -1, &len);
                        msgout.WriteBits(v, len);
                    }
                    break;
                    case LUA_TTABLE: {
                        size_t len = lua_rawlen(L, -1);
                        msgout.WriteUShort(len);
                        for (size_t i = 1; i <= len; ++i) {
                            lua_rawgeti(L, -1, i);
                            write_lua_value(msgout, L, -1);
                            lua_pop(L, 1);
                        }
                        size_t len1 = 0;
                        lua_pushnil(L);
                        while (lua_next(L, -2)) {
                            ++len1;
                            lua_pop(L, 1);
                        }
                        msgout.WriteUShort(len1);
                        lua_pushnil(L);
                        while (lua_next(L, -2)) {
                            write_lua_value(msgout, L, -2);
                            write_lua_value(msgout, L, -1);
                            lua_pop(L, 1);
                        }
                    }
                    break;
                    case LUA_TLIGHTUSERDATA: {
                        void* ptr = lua_touserdata(L, -1);
                        msgout.WriteLong(reinterpret_cast<intptr_t>(ptr));
                    }
                    break;
                    default:
                        luaL_error(L, "unspported type in rpc method");
                        break;
                }
                lua_pop(L, 1);
            }

            // 从message中读取lua数据
            void read_lua_value(base::cluster::MessageIn& msgin, lua_State* L)
            {
                int type = msgin.ReadSByte();
                switch (type) {
                    case LUA_TNIL:
                        lua_pushnil(L);
                        break;
                    case LUA_TNUMBER: {
                        double v = msgin.ReadDouble();
                        lua_pushnumber(L, v);
                    }
                    break;
                    case LUA_TBOOLEAN: {
                        bool v = msgin.ReadBoolean();
                        lua_pushboolean(L, v);
                    }
                    break;
                    case LUA_TSTRING: {
                        string v = msgin.ReadString();
                        lua_pushstring(L, v.c_str());
                    }
                    break;
                    case LUA_TTABLE: {
                        lua_newtable(L);
                        size_t len = msgin.ReadUShort();
                        for (size_t i = 1; i <= len; ++i) {
                            read_lua_value(msgin, L);
                            lua_rawseti(L, -2, i);
                        }
                        len = msgin.ReadUShort();
                        for (size_t i = 0; i < len; ++i) {
                            read_lua_value(msgin, L);
                            read_lua_value(msgin, L);
                            lua_rawset(L, -3);
                        }
                    }
                    break;
                    case LUA_TLIGHTUSERDATA: {
                        intptr_t v = msgin.ReadLong();
                        lua_pushlightuserdata(L, (void*)v);
                    }
                    break;
                    default:
                        break;
                }
            }

            /// LuaRpcService
            class LuaRpcService : public base::cluster::RpcService
            {
            public:
                LuaRpcService(lua_State* _L, const char* service_name)
                    : RpcService(service_name, true), L(_L) {}
                virtual ~LuaRpcService() {}

            private:
                virtual void OnCall(const base::cluster::MailboxID& from, uint16_t session, base::cluster::MessageIn& msgin) {
                    AutoCleanupStack ac(L);
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_SERVICE_LIST);
                    lua_pushstring(L, service_name().c_str());
                    lua_rawget(L, -2);
                    if (lua_istable(L, -1)) {
                        string method_name;
                        msgin >> method_name;
                        lua_pushstring(L, method_name.c_str());
                        lua_rawget(L, -2);
                        int pos_before = lua_gettop(L);
                        if (lua_isfunction(L, -1)) {
                            uint16_t argc = msgin.ReadUShort();
                            for (uint16_t i = 0; i < argc; ++i) {
                                read_lua_value(msgin, L);
                            }
                            if (lua_pcall(L, argc, LUA_MULTRET, 0)) {
                                LOG_ERROR2("rpc call[%s] exec error: %s\n", method_name.c_str(), lua_tostring(L, -1));
                            } else {
                                base::cluster::MessageOut msgout(msgin.code(), 100, mempool());
                                int pos_after = lua_gettop(L);
                                if (pos_after < pos_before) {
                                    pos_after = pos_before;
                                }
                                msgout.WriteUShort(pos_after - pos_before + 1);
                                for (int i = pos_before; i <= pos_after; ++i) {
                                    write_lua_value(msgout, L, i);
                                }
                                Reply(from, session, msgout);
                            }
                        } else {
                            LOG_DEBUG("service[%s] has no method[%s] to call\n", service_name().c_str(), method_name.c_str());
                        }
                    }
                }

                virtual void OnCast(const base::cluster::MailboxID& from, base::cluster::MessageIn& msgin) {
                    AutoCleanupStack ac(L);
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_SERVICE_LIST);
                    lua_pushstring(L, service_name().c_str());
                    lua_rawget(L, -2);
                    if (lua_istable(L, -1)) {
                        string method_name;
                        msgin >> method_name;
                        lua_pushstring(L, method_name.c_str());
                        lua_rawget(L, -2);
                        if (lua_isfunction(L, -1)) {
                            uint16_t argc = msgin.ReadUShort();
                            for (uint16_t i = 0; i < argc; ++i) {
                                read_lua_value(msgin, L);
                            }
                            if (lua_pcall(L, argc, 0, 0)) {
                                LOG_ERROR2("rpc cast[%s] exec error: %s\n", method_name.c_str(), lua_tostring(L, -1));
                            }
                        } else {
                            LOG_DEBUG("service[%s] has no method[%s] to cast\n", service_name().c_str(), method_name.c_str());
                        }
                    }
                }

            private:
                lua_State* L;
            };

            static int service_object_gc(lua_State* L)
            {
                lua_getfield(L, 1, "__serobj");
                if (lua_isuserdata(L, -1)) {
                    LuaRpcService* serobj = static_cast<LuaRpcService*>(lua_touserdata(L, -1));
                    delete serobj;
                }
                return 0;
            }

            // 打开一个服务, 当传入的lua service object被垃圾回收时，服务自动清除
            // 不持有lua service object的对象引用
            static int open_service(lua_State* L)
            {
                if (!lua_isstring(L, 1)) {
                    return luaL_argerror(L, 1, "string excepted");
                }
                if (!lua_istable(L, 2)) {
                    return luaL_argerror(L, 2, "table excepted");
                }
                if (lua_getmetatable(L, 2) != 0) {
                    return luaL_argerror(L, 2, "service object shall have empty metatable");
                }
                const char* service_name = luaL_checkstring(L, 1);
                // 检查是否有重复的服务
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_SERVICE_LIST);
                lua_getfield(L, -1, service_name);
                if (!lua_isnil(L, -1)) {
                    return luaL_argerror(L, 1, "duplicate service name");
                }
                lua_pop(L, 1);

                //　绑定元表, 便于service object在垃圾回收时同时回收RpcService对象
                lua_newtable(L);
                lua_pushcfunction(L, service_object_gc);
                lua_setfield(L, -2, "__gc");
                lua_setmetatable(L, 2);

                // 拿到主线程的lua_State
                lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                lua_State* L0 = lua_tothread(L, -1);

                // 创建LuaRpcService对象
                LuaRpcService* serobj = new LuaRpcService(L0, service_name);
                if (!serobj->Setup()) {
                    delete serobj;
                    return luaL_error(L, "create service fail, maybe service name is duplicate");
                }

                // 保存起来
                lua_pushlightuserdata(L, serobj);
                lua_setfield(L, 2, "__serobj");
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_SERVICE_LIST);
                lua_pushvalue(L, 2);
                lua_setfield(L, -2, service_name);
                return 0;
            }

            // 手动关闭一个服务
            static int close_service(lua_State* L)
            {
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_SERVICE_LIST);
                lua_pushvalue(L, 1);
                lua_rawget(L, -2);
                if (lua_istable(L, -1)) {
                    // release LuaRpcService
                    lua_getfield(L, -1, "__serobj");
                    if (lua_isuserdata(L, -1)) {
                        LuaRpcService* serobj = static_cast<LuaRpcService*>(lua_touserdata(L, -1));
                        delete serobj;
                        lua_pushnil(L);
                        lua_setfield(L, -3, "__serobj");
                    }
                }
                return 0;
            }

            /// RpcStub相关

            struct UD_LuaRpcStub {
                base::cluster::RpcStub* stub;
                int32_t stub_list_id;
            };

            static void rpc_stub_call_callback(base::cluster::MessageIn& msgin, lua_State* L0, int32_t callback_id)
            {
                AutoCleanupStack ac(L0);
                // 拿到挂起的lua_State
                lua_rawgetp(L0, LUA_REGISTRYINDEX, &KEY_RPC_CORO_LIST);
                lua_rawgeti(L0, -1, callback_id);

                if (lua_isthread(L0, -1)) {
                    lua_State* L = lua_tothread(L0, -1);
                    lua_pop(L, lua_gettop(L));
                    uint16_t argc = msgin.ReadUShort();
                    for (uint16_t i = 0; i < argc; ++i) {
                        read_lua_value(msgin, L);
                    }
                    int r = lua_resume(L, L0, argc);
                    if (r != LUA_OK && r != LUA_YIELD) {
                        LOG_ERROR2("rpc call error: %s\n", lua_tostring(L, -1));
                    }
                }
            }

            static int rpc_stub_call(lua_State* L)
            {
                // 设置msgout的释放作用域非常重要，如果不释放掉，在lua_yield后，MessageOut的析构函数将不能得到调用
                // 所有使用lua_yield的地方都应检查C++对象的析构问题
                {
                    int argc = lua_gettop(L);
                    UD_LuaRpcStub* ud = static_cast<UD_LuaRpcStub*>(luaL_checkudata(L, lua_upvalueindex(1), MT_RPC_STUB));
                    const char* method_name = luaL_checkstring(L, lua_upvalueindex(2));
                    if (!ud->stub->ready()) {
                        return luaL_error(L, "rpc stub is not ready yet!\n");
                    }

                    // 拿到主线程的lua_State
                    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                    lua_State* L0 = lua_tothread(L, -1);

                    if (L == L0) {
                        return luaL_error(L, "can not do rpc call at main thread");
                    }

                    // 保存当前lua_State
                    int32_t callback_id = rpc_stub_coro_id_cur++;
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_CORO_LIST);
                    lua_pushthread(L);
                    lua_rawseti(L, -2, callback_id);

                    // 执行call
                    base::cluster::MessageOut msgout(0, 100, ud->stub->mempool());
                    msgout.WriteString(method_name + 5);
                    msgout.WriteUShort(argc);
                    for (int i = 1; i <= argc; ++i) {
                        write_lua_value(msgout, L, i);
                    }
                    ud->stub->CallWithoutLinker(msgout, boost::bind(&rpc_stub_call_callback, _1, L0, callback_id));
                }
                // 挂起
                return lua_yield(L, 0);
            }

            static int rpc_stub_cast(lua_State* L)
            {
                int argc = lua_gettop(L);
                UD_LuaRpcStub* ud = static_cast<UD_LuaRpcStub*>(luaL_checkudata(L, lua_upvalueindex(1), MT_RPC_STUB));
                const char* method_name = luaL_checkstring(L, lua_upvalueindex(2));
                base::cluster::MessageOut msgout(1, 100, ud->stub->mempool());
                msgout.WriteString(method_name + 5);
                msgout.WriteUShort(argc);
                for (int i = 1; i <= argc; ++i) {
                    write_lua_value(msgout, L, i);
                }
                ud->stub->Cast(msgout);
                return 0;
            }

            static int rpc_stub_meta_index(lua_State* L)
            {
                const char* method_name = luaL_checkstring(L, 2);
                if (strncmp(method_name, "call_", 5) == 0) {
                    lua_pushcclosure(L, rpc_stub_call, 2);
                    return 1;
                } else if (strncmp(method_name, "cast_", 5) == 0) {
                    lua_pushcclosure(L, rpc_stub_cast, 2);
                    return 1;
                } else {
                    return luaL_error(L, "rpc stub only support method whit prefix 'call_' or 'cast_'");
                }
            }

            static int rpc_stub_meta_gc(lua_State* L)
            {
                UD_LuaRpcStub* ud = static_cast<UD_LuaRpcStub*>(luaL_checkudata(L, 1, MT_RPC_STUB));
                SAFE_DELETE(ud->stub);
                return 0;
            }

            static void rpc_stub_setup_callback()
            {
                // THINK: work as yield before the rpc stub change to ready state
            }

            // 连接一个服务
            static int connect_service(lua_State* L)
            {
                const char* service_name = luaL_checkstring(L, 1);
                UD_LuaRpcStub* ud = static_cast<UD_LuaRpcStub*>(lua_newuserdata(L, sizeof(UD_LuaRpcStub)));
                ud->stub = new base::cluster::RpcStub(service_name);
                ud->stub->BeginSetup(boost::bind(&rpc_stub_setup_callback));
                luaL_setmetatable(L, MT_RPC_STUB);
                ud->stub_list_id = rpc_stub_list_id_cur++;
                // 保存起来
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_STUB_LIST);
                lua_pushvalue(L, -2);
                lua_rawseti(L, -2, ud->stub_list_id);
                lua_pop(L, 1);
                return 1;
            }

            /// 邮箱相关

            class LuaMailbox : public base::cluster::Mailbox::EventHandler
            {
            public:
                LuaMailbox(lua_State* L0) : L(L0), mailbox_(nullptr) {}
                virtual ~LuaMailbox() {
                    SAFE_DELETE(mailbox_);
                }

                int32_t pid() const {
                    return mailbox_->mbid().pid();
                }

                bool Setup() {
                    mailbox_ = base::cluster::Mailbox::Create(*this);
                    return mailbox_ != nullptr;
                }

                base::cluster::Mailbox& mailbox() {
                    return *mailbox_;
                }

                const base::cluster::MailboxID& mbid() const {
                    return mailbox_->mbid();
                }

            private:
                virtual void OnMessageReceive(base::cluster::MessageIn& msgin) {
                    // 通过pid找到对应的ud
                    AutoCleanupStack ac(L);
                    mailbox_->mbid().pid();
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_MAILBOX_LIST);
                    lua_rawgeti(L, -1, pid());
                    if (lua_isuserdata(L, -1)) {
                        lua_getuservalue(L, -1);
                        if (lua_istable(L, -1)) {
                            lua_rawgeti(L, -1, 1);
                            if (lua_isfunction(L, -1)) {
                                uint16_t argc = msgin.ReadUShort();
                                for (uint16_t i = 0; i < argc; ++i) {
                                    read_lua_value(msgin, L);
                                }
                                if (lua_pcall(L, argc, 0, 0)) {
                                    LOG_ERROR2("mailbox handler exec error: %s\n", lua_tostring(L, -1));
                                }
                            }
                        }
                    }
                }
                lua_State* L;
                base::cluster::Mailbox* mailbox_;
            };

            struct UD_Mailbox {
                LuaMailbox* mb;
            };

            static inline LuaMailbox* check_lua_mailbox(lua_State* L, int idx = 1)
            {
                UD_Mailbox* ud = static_cast<UD_Mailbox*>(luaL_checkudata(L, idx, MT_MAILBOX));
                return ud->mb;
            }

            // lua => mailbox:id()
            static int mailbox_meta_id(lua_State* L)
            {
                LuaMailbox* mb = check_lua_mailbox(L);
                lua_pushunsigned(L, mb->mbid().ValueID());
                return 1;
            }

            // lua => mailbox:send(to, ...)
            static int mailbox_meta_send(lua_State* L)
            {
                LuaMailbox* mb = check_lua_mailbox(L);
                uint32_t toid = luaL_checkunsigned(L, 2);
                base::cluster::MailboxID to;
                to.Parse(toid);

                base::cluster::MessageOut msgout(0, 120, mb->mailbox().mempool());
                int top = lua_gettop(L);
                msgout.WriteUShort(top - 2);
                for (int i = 3; i <= top; ++i) {
                    write_lua_value(msgout, L, i);
                }
                mb->mailbox().Cast(to, msgout);
                return 0;
            }

            static int mailbox_meta_close(lua_State* L)
            {
                UD_Mailbox* ud = static_cast<UD_Mailbox*>(luaL_checkudata(L, 1, MT_MAILBOX));
                SAFE_DELETE(ud->mb);
                return 0;
            }

            static int mailbox_meta_gc(lua_State* L)
            {
                UD_Mailbox* ud = static_cast<UD_Mailbox*>(luaL_checkudata(L, 1, MT_MAILBOX));
                SAFE_DELETE(ud->mb);
                return 0;
            }

            static int create_mailbox(lua_State* L)
            {
                if (!lua_isfunction(L, 1)) {
                    return luaL_argerror(L, 1, "function expected");
                }

                // 拿到主线程的lua_State
                lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                lua_State* L0 = lua_tothread(L, -1);

                LuaMailbox* mailbox = new LuaMailbox(L0);
                if (!mailbox->Setup()) {
                    delete mailbox;
                    return luaL_error(L, "create mailbox fail");
                }

                UD_Mailbox* ud = static_cast<UD_Mailbox*>(lua_newuserdata(L, sizeof(UD_Mailbox)));
                ud->mb = mailbox;
                luaL_setmetatable(L, MT_MAILBOX);

                // 将mailbox的lua处理函数保存起来
                lua_newtable(L);
                lua_pushvalue(L, 1);
                lua_rawseti(L, -2, 1);
                lua_setuservalue(L, -2);

                // 保存起来
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_MAILBOX_LIST);
                lua_pushvalue(L, -2);
                lua_rawseti(L, -2, ud->mb->pid());
                lua_pop(L, 1);
                return 1;
            }

            static int sendto(lua_State* L)
            {
                uint32_t toid = luaL_checkunsigned(L, 1);
                base::cluster::MailboxID to;
                to.Parse(toid);
                base::cluster::MessageOut msgout(0, 120, base::cluster::NodeMonitor::instance().mempool());
                msgout.SetTo(to);
                int top = lua_gettop(L);
                msgout.WriteUShort(top - 1);
                for (int i = 2; i <= top; ++i) {
                    write_lua_value(msgout, L, i);
                }
                base::cluster::NodeMonitor::instance().SendMessage(msgout);
                return 0;
            }

            struct UD_AsyncFetchLinker {
                base::cluster::AsyncFetchLinker* linker;
            };

            static int async_fetch_linker_meta_gc(lua_State* L)
            {
                UD_AsyncFetchLinker* ud = static_cast<UD_AsyncFetchLinker*>(luaL_checkudata(L, 1, MT_ASYNC_FETCH_LINKER));
                SAFE_RELEASE(ud->linker);
                return 0;
            }

            static void fetch_named_mailbox_callback(lua_State* L, const base::cluster::MailboxID& mbid, int32_t callback_id)
            {
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_ASYNC_FETCH_LIST);
                lua_rawgeti(L, -1, callback_id);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_isfunction(L, -2)) {
                        lua_pushvalue(L, -2);
                        lua_pushunsigned(L, mbid.ValueID());
                        if (lua_pcall(L, 1, 0, 0)) {
                            LOG_ERROR2("script error: %s", lua_tostring(L, -1));
                            lua_pop(L, 1);
                        }
                        UD_AsyncFetchLinker* ud = static_cast<UD_AsyncFetchLinker*>(luaL_checkudata(L, -1, MT_ASYNC_FETCH_LINKER));
                        SAFE_RELEASE(ud->linker);
                    }
                    break;
                }
            }

            static int fetch_named_mailbox(lua_State* L)
            {
                const char* matcher = luaL_checkstring(L, 1);
                if (!lua_isfunction(L, 2)) {
                    return luaL_argerror(L, 2, "function expected");
                }

                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_ASYNC_FETCH_LIST);
                // 保存回调信息
                int32_t callback_id = async_fetch_id_cur++;
                lua_newtable(L);
                luaL_setmetatable(L, MT_ASYNC_FETCH_WEAK_K);
                lua_pushvalue(L, 2);
                UD_AsyncFetchLinker* ud = static_cast<UD_AsyncFetchLinker*>(lua_newuserdata(L, sizeof(UD_AsyncFetchLinker)));
                luaL_setmetatable(L, MT_ASYNC_FETCH_LINKER);
                ud->linker = nullptr;
                lua_rawset(L, -3);
                lua_rawseti(L, -2, callback_id);

                // 拿到主线程的lua_State
                lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                lua_State* L0 = lua_tothread(L, -1);

                base::cluster::AsyncFetchLinker* linker = base::cluster::NodeMonitor::instance().FetchNamedMailboxWithLinker(matcher, boost::bind(&fetch_named_mailbox_callback, L0, _1, callback_id));
                if (linker != nullptr) {
                    ud->linker = linker;
                    linker->Retain();
                }
                return 0;
            }

            static int parse_mailboxid(lua_State* L)
            {
                uint32_t value = luaL_checkunsigned(L, 1);
                base::cluster::MailboxID mbid;
                mbid.Parse(value);
                lua_pushinteger(L, mbid.nodeid());
                lua_pushinteger(L, mbid.pid());
                return 2;
            }

            static int _luaopen_cluster(lua_State* L)
            {
                // 生成一个弱表元信息
                lua_newtable(L);
                lua_pushstring(L, "v");
                lua_setfield(L, -2, "__mode");

                // 创建用于存放service的表
                lua_newtable(L);
                lua_pushvalue(L, -2);
                lua_setmetatable(L, -2);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_RPC_SERVICE_LIST);

                // 创建用于存放stub的表
                lua_newtable(L);
                lua_pushvalue(L, -2);
                lua_setmetatable(L, -2);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_RPC_STUB_LIST);

                // 创建用于存放call而yield的coroutine列表
                lua_newtable(L);
                lua_pushvalue(L, -2);
                lua_setmetatable(L, -2);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_RPC_CORO_LIST);

                // 创建用于存放mailbox的列表
                lua_newtable(L);
                lua_pushvalue(L, -2);
                lua_setmetatable(L, -2);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_MAILBOX_LIST);

                // 生成并保存一个rpcstub的元表
                luaL_newmetatable(L, MT_RPC_STUB);
                lua_pushstring(L, "__index");
                lua_pushcfunction(L, &rpc_stub_meta_index);
                lua_rawset(L, -3);
                lua_pushstring(L, "__gc");
                lua_pushcfunction(L, &rpc_stub_meta_gc);
                lua_rawset(L, -3);
                lua_pop(L, 1);

                // 生成用于保存async_fetch的列表
                lua_newtable(L);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_ASYNC_FETCH_LIST);

                // 生成并保存一个用于async_fetch的元表
                luaL_newmetatable(L, MT_ASYNC_FETCH_LINKER);
                lua_pushstring(L, "__gc");
                lua_pushcfunction(L, &async_fetch_linker_meta_gc);
                lua_rawset(L, -3);
                lua_pop(L, 1);

                // 生成用于async_fetch的回调信息为weak-key表
                luaL_newmetatable(L, MT_ASYNC_FETCH_WEAK_K);
                lua_pushstring(L, "k");
                lua_setfield(L, -2, "__mode");
                lua_pop(L, 1);

                // 生成并保存一个mailbox元表
                luaL_Reg mt_mailbox[] = {
                    {"id", mailbox_meta_id},
                    {"send", mailbox_meta_send},
                    {"close", mailbox_meta_close},
                    {nullptr, nullptr}
                };

                luaL_newmetatable(L, MT_MAILBOX);
                lua_pushstring(L, "__index");
                lua_newtable(L);
                luaL_setfuncs(L, mt_mailbox, 0);
                lua_rawset(L, -3);
                lua_pushstring(L, "__gc");
                lua_pushcfunction(L, &mailbox_meta_gc);
                lua_rawset(L, -3);
                lua_pop(L, 1);

                luaL_Reg cluster_funcs[] = {
                    {"open_service", open_service},
                    {"close_service", close_service},
                    {"connect_service", connect_service},
                    {"create_mailbox", create_mailbox},
                    {"sendto", sendto},
                    {"fetch_named_mailbox", fetch_named_mailbox},
                    {"parse_mailboxid", parse_mailboxid},
                    {nullptr, nullptr},
                };
                luaL_newlib(L, cluster_funcs);
                internal::lua_event_c_new(L, "named_mailbox_up", "named_mailbox_down", "node_up", "node_down", nullptr);
                return 1;
            }

            void lua_cluster_on_named_mailbox_up(lua_State* L, const char* name, uint32_t mbid)
            {
                lua_getglobal(L, "package");
                lua_getfield(L, -1, "loaded");
                lua_getfield(L, -1, "cluster");
                lua_getfield(L, -1, "event");
                lua_pushstring(L, "named_mailbox_up");
                lua_pushstring(L, name);
                lua_pushunsigned(L, mbid);
                internal::lua_event_c_trigger(L, 4);
            }

            void lua_cluster_on_named_mailbox_down(lua_State* L, const char* name)
            {
                lua_getglobal(L, "package");
                lua_getfield(L, -1, "loaded");
                lua_getfield(L, -1, "cluster");
                lua_getfield(L, -1, "event");
                lua_pushstring(L, "named_mailbox_down");
                lua_pushstring(L, name);
                internal::lua_event_c_trigger(L, 3);
            }

            void lua_cluster_on_node_up(lua_State* L, const char* name, uint32_t nodeid)
            {
                lua_getglobal(L, "package");
                lua_getfield(L, -1, "loaded");
                lua_getfield(L, -1, "cluster");
                lua_getfield(L, -1, "event");
                lua_pushstring(L, "node_up");
                lua_pushstring(L, name);
                lua_pushunsigned(L, nodeid);
                internal::lua_event_c_trigger(L, 4);
            }

            void lua_cluster_on_node_down(lua_State* L, const char* name, uint32_t nodeid)
            {
                lua_getglobal(L, "package");
                lua_getfield(L, -1, "loaded");
                lua_getfield(L, -1, "cluster");
                lua_getfield(L, -1, "event");
                lua_pushstring(L, "node_down");
                lua_pushstring(L, name);
                lua_pushunsigned(L, nodeid);
                internal::lua_event_c_trigger(L, 4);
            }

            void luaopen_cluster(lua_State* L)
            {
                luaL_requiref(L, "cluster", &_luaopen_cluster, 0);
                lua_pop(L, 1);
            }

            void luaclose_cluster(lua_State* L)
            {
                // 手动关闭所有service
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_SERVICE_LIST);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_istable(L, -1)) {
                        // release LuaRpcService
                        lua_getfield(L, -1, "__serobj");
                        if (lua_isuserdata(L, -1)) {
                            LuaRpcService* serobj = static_cast<LuaRpcService*>(lua_touserdata(L, -1));
                            delete serobj;
                            lua_pushnil(L);
                            lua_setfield(L, -3, "__serobj");
                        }
                        lua_pop(L, 1);
                    }
                    lua_pop(L, 1);
                }

                // 手动关闭所有stub
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_RPC_STUB_LIST);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_isuserdata(L, -1)) {
                        UD_LuaRpcStub* ud = static_cast<UD_LuaRpcStub*>(luaL_checkudata(L, -1, MT_RPC_STUB));
                        SAFE_DELETE(ud->stub);
                    }
                    lua_pop(L, 1);
                }

                // 手动关闭所有mailbox
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_MAILBOX_LIST);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_isuserdata(L, -1)) {
                        UD_Mailbox* ud = static_cast<UD_Mailbox*>(luaL_checkudata(L, -1, MT_MAILBOX));
                        SAFE_DELETE(ud->mb);
                    }
                    lua_pop(L, 1);
                }

                // 清理所有async_fetch
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_ASYNC_FETCH_LIST);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_isuserdata(L, -1)) {
                        UD_AsyncFetchLinker* ud = static_cast<UD_AsyncFetchLinker*>(luaL_checkudata(L, -1, MT_ASYNC_FETCH_LINKER));
                        SAFE_DELETE(ud->linker);
                    }
                    lua_pop(L, 1);
                }
            }
        }
    }
}
