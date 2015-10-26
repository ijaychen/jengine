#include "lua-dbo.h"
#include "../helper.h"
#include "../../logger.h"
#include "../../dbo/connection.h"
#include "../../dbo/preparedstatement.h"
#include <boost/bind.hpp>
#include <iostream>

namespace base
{
    namespace lua
    {
        namespace dbo
        {
            using namespace std;

            static const char KEY_CONN_LIST = 'k';
            static const char KEY_CORO_LIST = 'k';
            static int32_t conn_id_cur = 0;

            enum DbType {
                DB_INT8 = 1,
                DB_INT16,
                DB_INT32,
                DB_INT64,
                DB_FLOAT,
                DB_DOUBLE,
                DB_STRING,
            };

            class LuaDbConnect
            {
            public:
                LuaDbConnect(int32_t poolid) {
                    conn_ = base::dbo::Connection::Create(poolid);
                    conn_->Retain();
                }
                ~LuaDbConnect() {
                    conn_->Release();
                }

                int Execute(lua_State* L) {
                    if (conn_->pending()) {
                        return luaL_error(L, "another statement under progress");
                    }
                    const char* sql = luaL_checkstring(L, 2);
                    base::dbo::PreparedStatement& pstmt = conn_->CreatePreparedStatement(sql);
                    int top = lua_gettop(L);
                    for (int i = 3; i <= top; ++i) {
                        switch (lua_type(L, i)) {
                            case LUA_TNIL: {
                                pstmt.SetNull();
                            }
                            break;
                            case LUA_TBOOLEAN: {
                                pstmt.SetBoolean(lua_toboolean(L, i));
                            }
                            break;
                            case LUA_TNUMBER: {
                                lua_Integer x = lua_tointeger(L, i);
                                lua_Number n = lua_tonumber(L, i);
                                if ((lua_Number)x == n) {
                                    pstmt.SetInt64(x);
                                } else {
                                    pstmt.SetDouble(n);
                                }
                            }
                            break;
                            case LUA_TSTRING: {
                                size_t len;
                                const char* param = lua_tolstring(L, i, &len);
                                pstmt.SetString(param, len);
                            }
                            break;
                            case LUA_TTABLE: {
                                lua_rawgeti(L, i, 1);
                                lua_rawgeti(L, i, 2);
                                int dbtype = luaL_checkinteger(L, -2);
                                switch (dbtype) {
                                    case DB_INT8:
                                        pstmt.SetInt8(lua_tointeger(L, -1));
                                        break;
                                    case DB_INT16:
                                        pstmt.SetInt16(lua_tointeger(L, -1));
                                        break;
                                    case DB_INT32:
                                        pstmt.SetInt32(lua_tointeger(L, -1));
                                        break;
                                    case DB_INT64:
                                        pstmt.SetInt64(lua_tointeger(L, -1));
                                        break;
                                    case DB_FLOAT:
                                        pstmt.SetFloat(lua_tonumber(L, -1));
                                        break;
                                    case DB_DOUBLE:
                                        pstmt.SetDouble(lua_tonumber(L, -1));
                                        break;
                                    case DB_STRING: {
                                        size_t len;
                                        const char* param = lua_tolstring(L, i, &len);
                                        pstmt.SetString(param, len);
                                    }
                                    break;
                                }
                            }
                            break;
                            default:
                                return luaL_error(L, "unsupport param type %d=>%s", i, lua_typename(L, lua_type(L, i)));
                        }
                    }

                    // 保存当前协程
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_CORO_LIST);
                    lua_pushthread(L);
                    lua_rawsetp(L, -2, this);

                    // 拿到主线程的lua_State
                    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                    lua_State* L0 = lua_tothread(L, -1);

                    pstmt.Execute(boost::bind(&LuaDbConnect::HandleExecuteResult, this, L0, _1));
                    return lua_yield(L, 0);
                }

            private:
                void HandleExecuteResult(lua_State* L0, base::dbo::ResultSet& rs) {
                    lua_rawgetp(L0, LUA_REGISTRYINDEX, &KEY_CORO_LIST);
                    lua_rawgetp(L0, -1, this);
                    lua_State* L = nullptr;
                    if (lua_isthread(L0, -1)) {
                        L = lua_tothread(L0, -1);
                    } else {
                        return;
                    }
                    if (rs.HasError()) {
                        lua_pushboolean(L, false);
                        lua_pushstring(L, rs.error_message());
                    } else {
                        lua_pushboolean(L, true);
                        lua_newtable(L);
                        lua_pushstring(L, "last_insert_id");
                        lua_pushinteger(L, rs.last_insert_id());
                        lua_rawset(L, -3);
                        lua_pushstring(L, "affected_rows");
                        lua_pushinteger(L, rs.affected_rows());
                        lua_rawset(L, -3);
                        int idx = 0;
                        while (rs.Next()) {
                            lua_newtable(L);
                            int col = 0;
                            for (vector<base::dbo::ColumnDefinition>::const_iterator it = rs.metadata().columns.begin(); it != rs.metadata().columns.end(); ++it) {
                                const base::dbo::ColumnDefinition& meta = *it;
                                lua_pushstring(L, meta.name.c_str());
                                ++col;
                                if (rs.IsNull(col)) {
                                    lua_pushnil(L);
                                } else {
                                    switch (meta.type) {
                                        case base::dbo::internal::MYSQL_TYPE_TINY:
                                            lua_pushinteger(L, rs.GetInt8(col));
                                            break;
                                        case base::dbo::internal::MYSQL_TYPE_SHORT:
                                        case base::dbo::internal::MYSQL_TYPE_YEAR:
                                            lua_pushinteger(L, rs.GetInt16(col));
                                            break;
                                        case base::dbo::internal::MYSQL_TYPE_LONG:
                                        case base::dbo::internal::MYSQL_TYPE_INT24:
                                            lua_pushinteger(L, rs.GetInt32(col));
                                            break;
                                        case base::dbo::internal::MYSQL_TYPE_LONGLONG:
                                            lua_pushinteger(L, rs.GetInt64(col)); // FIXME lua not support int64
                                            break;
                                        case base::dbo::internal::MYSQL_TYPE_FLOAT:
                                            lua_pushnumber(L, rs.GetFloat(col));
                                            break;
                                        case base::dbo::internal::MYSQL_TYPE_DOUBLE:
                                            lua_pushnumber(L, rs.GetDouble(col));
                                            break;
                                        case base::dbo::internal::MYSQL_TYPE_VARCHAR:
                                        case base::dbo::internal::MYSQL_TYPE_BIT:
                                        case base::dbo::internal::MYSQL_TYPE_TINY_BLOB:
                                        case base::dbo::internal::MYSQL_TYPE_MEDIUM_BLOB:
                                        case base::dbo::internal::MYSQL_TYPE_LONG_BLOB:
                                        case base::dbo::internal::MYSQL_TYPE_BLOB:
                                        case base::dbo::internal::MYSQL_TYPE_VAR_STRING:
                                        case base::dbo::internal::MYSQL_TYPE_STRING: {
                                            string field = rs.GetString(col);
                                            lua_pushstring(L, field.c_str());
                                        }
                                        break;
                                        default:
                                            lua_pushnil(L);
                                            LOG_ERROR("not support field type: %s\n", meta.type);
                                            break;
                                    }
                                }
                                lua_rawset(L, -3);
                            }
                            lua_rawseti(L, -2, ++idx);
                        }
                    }
                    lua_resume(L, nullptr, 2);
                }
                base::dbo::Connection* conn_;
            };

            struct UD_LuaDbConnect {
                LuaDbConnect* ptr;
            };

            static const char* MT_CONN = "MT_CONN";

            static int conn_mt_execute(lua_State* L)
            {
                UD_LuaDbConnect* ud = static_cast<UD_LuaDbConnect*>(luaL_checkudata(L, 1, MT_CONN));
                if (ud->ptr) {
                    return ud->ptr->Execute(L);
                } else {
                    return luaL_error(L, "the connection has been closed");
                }
            }

            static int conn_mt_close(lua_State* L)
            {
                UD_LuaDbConnect* ud = static_cast<UD_LuaDbConnect*>(luaL_checkudata(L, 1, MT_CONN));
                SAFE_DELETE(ud->ptr);
                return 0;
            }

            static int conn_mt_gc(lua_State* L)
            {
                UD_LuaDbConnect* ud = static_cast<UD_LuaDbConnect*>(luaL_checkudata(L, 1, MT_CONN));
                SAFE_DELETE(ud->ptr);
                return 0;
            }

            static int connect(lua_State* L)
            {
                int32_t poolid = luaL_optinteger(L, 1, 0);
                if (!base::dbo::Connection::is_valid_poolid(poolid)) {
                    return luaL_error(L, "invalid connection poolid");
                }
                UD_LuaDbConnect* ud = static_cast<UD_LuaDbConnect*>(lua_newuserdata(L, sizeof(UD_LuaDbConnect)));
                luaL_setmetatable(L, MT_CONN);
                ud->ptr = new LuaDbConnect(poolid);
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_CONN_LIST);
                lua_pushvalue(L, -2);
                lua_rawseti(L, -2, ++conn_id_cur);
                lua_pop(L, 1);
                return 1;
            }

            static int _luaopen_dbo(lua_State* L)
            {
                luaL_Reg mt_conn[] = {
                    {"execute", conn_mt_execute},
                    {"close", conn_mt_close},
                    {"__gc", conn_mt_gc},
                    {nullptr, nullptr}
                };

                if (luaL_newmetatable(L, MT_CONN) != 1) {
                    LOG_ERROR("duplicate metatable name: %s\n", MT_CONN);
                    return 0;
                }
                luaL_setfuncs(L, mt_conn, 0);
                lua_pushstring(L, "__index");
                lua_pushvalue(L, -2);
                lua_rawset(L, -3);
                lua_pop(L, 1);

                // 用于保存所有连接对象
                lua_newtable(L);
                lua_pushvalue(L, -1);
                lua_pushstring(L, "__mode");
                lua_pushstring(L, "v");
                lua_rawset(L, -3);
                lua_setmetatable(L, -2);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_CONN_LIST);

                // 用于保存所有挂起的协程
                lua_newtable(L);
                lua_pushvalue(L, -1);
                lua_pushstring(L, "__mode");
                lua_pushstring(L, "v");
                lua_rawset(L, -3);
                lua_setmetatable(L, -2);
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_CORO_LIST);

                luaL_Reg dbo_funcs[] = {
                    {"connect", connect},
                    {nullptr, nullptr}
                };
                luaL_newlib(L, dbo_funcs);

                // convert the mysql type to lua value
#define __convert_to_lua(T) do { lua_pushstring(L, #T); lua_pushinteger(L, T); lua_rawset(L, -3); } while (false)
                __convert_to_lua(DB_INT8);
                __convert_to_lua(DB_INT16);
                __convert_to_lua(DB_INT32);
                __convert_to_lua(DB_INT64);
                __convert_to_lua(DB_FLOAT);
                __convert_to_lua(DB_DOUBLE);
                __convert_to_lua(DB_STRING);
#undef __convert_to_lua

                return 1;
            }

            void luaopen_dbo(lua_State* L)
            {
                luaL_requiref(L, "dbo", &_luaopen_dbo, 0);
                lua_pop(L, 1);
            }

            void luaclose_dbo(lua_State* L)
            {
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_CONN_LIST);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_isuserdata(L, -1)) {
                        UD_LuaDbConnect* ud = static_cast<UD_LuaDbConnect*>(luaL_checkudata(L, -1, MT_CONN));
                        SAFE_DELETE(ud->ptr);
                    }
                    lua_pop(L, 1);
                }
            }
        }
    }
}
