#include "lua-timer.h"
#include "../../event/dispatcher.h"
#include "../../logger.h"
#include "../../action/actioninterval.h"
#include "../../action/executor.h"
#include "../helper.h"
#include "../modulelua.h"
#include <lua.hpp>
#include <boost/bind.hpp>

namespace base
{
    namespace lua
    {
        namespace internal
        {
            using namespace std;

            const static char KEY_TIMEOUT_CB = 'k';             // 所有回调信息列表
            const static char KEY_WAIT_CO = 'k';                // 挂起的coroutine列表
            const static char KEY_INTERVAL_CB = 'k';            // 重复执行的函数
            static int32_t timeout_callback_id_ = 0;
            static int32_t coroutine_callback_id_ = 0;
            static int32_t interval_callback_id_ = 0;

            // 用于支持timer.set_interval功能的action
            class ActionLuaInterval : public action::ActionInterval
            {
            public:
                ActionLuaInterval(lua_State* _L, int32_t callback_id, int32_t interval)
                    : action::ActionInterval(interval), L(_L), callback_id_(callback_id), done_(false) {}

            private:
                virtual void OnInterval(int64_t tick) {
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_INTERVAL_CB);
                    lua_rawgeti(L, -1, callback_id_);
                    if (lua_isfunction(L, -1)) {
                        lua_pushunsigned(L, tick);
                        int r = lua_pcall(L, 1, 1, 0);
                        if (r) {
                            LOG_ERROR2("script error: %s\n", lua_tostring(L, -1));
                            lua_pop(L, 1);
                            done_ = true;
                        } else {
                            done_ = lua_toboolean(L, -1);
                        }
                    }
                }
                virtual bool IsDone() {
                    if (done_) {
                        lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_INTERVAL_CB);
                        lua_pushnil(L);
                        lua_rawgeti(L, -2, callback_id_);
                    }
                    lua_pop(L, lua_gettop(L));
                    return done_;
                }
            private:
                lua_State* L;
                int32_t callback_id_;
                bool done_;
            };

            static void wait_callback(lua_State* L0, int32_t coid)
            {
                lua_rawgetp(L0, LUA_REGISTRYINDEX, &KEY_WAIT_CO);
                lua_rawgeti(L0, -1, coid);
                if (lua_isthread(L0, -1)) {
                    lua_State* L = lua_tothread(L0, -1);
                    int r = lua_resume(L, L0, 0);
                    if (r != LUA_OK && r != LUA_YIELD) {
                        LOG_ERROR2("script error: %s\n", lua_tostring(L, -1));
                        lua_pop(L, 1);
                    }
                }
                // 清除掉记录
                lua_pushnil(L0);
                lua_rawseti(L0, -3, coid);

                lua_pop(L0, lua_gettop(L0));
            }

            // 将当前协程挂起一段时间, 不能在主协程中执行此操作
            static int wait(lua_State* L)
            {
                {
                    uint32_t delay = luaL_checkunsigned(L, 1);
                    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                    lua_State* L0 = lua_tothread(L, -1);

                    // 不能挂起主线程
                    if (L == L0) {
                        return luaL_error(L, "can not do timer.wait in main thread");
                    }

                    int32_t coid = coroutine_callback_id_++;
                    lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_WAIT_CO);
                    lua_pushthread(L);
                    lua_rawseti(L, -2, coid);

                    g_dispatcher->quicktimer().SetTimeout(boost::bind(&wait_callback, L0, coid), delay);
                }
                return lua_yield(L, 0);
            }

            static void timeout_callback(lua_State* L, int32_t callback_id)
            {
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_TIMEOUT_CB);
                lua_rawgeti(L, -1, callback_id);
                if (lua_isfunction(L, -1)) {
                    // 如果还存在数据引用
                    if (lua_pcall(L, 0, 0, 0)) {
                        LOG_ERROR2("script error: %s\n", lua_tostring(L, -1));
                        lua_pop(L, 1);
                    }
                    lua_pushnil(L);
                    lua_rawseti(L, -2, callback_id);
                } else {
                    // 已被垃圾回收
                    lua_pushnil(L);
                    lua_rawseti(L, -3, callback_id);
                }
                lua_pop(L, lua_gettop(L));
            }

            // 注册一个延时回调函数，该回调函数超时后将在lua的主线程中执行
            // set_timeout 并不持有该回调函数的引用，注意保持回调函数在被执行前不被垃圾回收
            static int set_timeout(lua_State* L)
            {
                if (!lua_isfunction(L, 1)) {
                    return luaL_argerror(L, 1, "function expected");
                }
                uint32_t delay = luaL_checkunsigned(L, 2);
                int32_t callback_id = timeout_callback_id_++;

                // 将回调函数保存起来
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_TIMEOUT_CB);
                lua_pushvalue(L, 1);
                lua_rawseti(L, -2, callback_id);

                // 拿到主线程的lua_State
                lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                lua_State* L0 = lua_tothread(L, -1);

                // 注册回调函数
                g_dispatcher->quicktimer().SetTimeout(boost::bind(&timeout_callback, L0, callback_id),
                                                      delay);
                return 0;
            }

            // 注册一个重复执行的函数，直到函数超出生命周期或是返回true
            static int set_interval(lua_State* L)
            {
                if (!lua_isfunction(L, 1)) {
                    return luaL_argerror(L, 1, "function expected");
                }
                uint32_t delay = luaL_checkunsigned(L, 2);
                int32_t callback_id = interval_callback_id_++;

                // 将回调函数保存起来
                lua_rawgetp(L, LUA_REGISTRYINDEX, &KEY_INTERVAL_CB);
                lua_pushvalue(L, 1);
                lua_rawseti(L, -2, callback_id);

                // 拿到主线程的lua_State
                lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
                lua_State* L0 = lua_tothread(L, -1);

                ActionLuaInterval* act = new ActionLuaInterval(L0, callback_id, delay);
                g_module_lua->executor()->RunAction(act);
                act->Release();
                return 0;
            }

            static int _luaopen_timer(lua_State* L)
            {
                // 生成一个弱表元信息
                lua_newtable(L);
                lua_pushstring(L, "v");
                lua_setfield(L, -2, "__mode");

                // 注册一个表用于存放超时回调列表
                lua_newtable(L);
                lua_pushvalue(L, -2);
                lua_setmetatable(L, -2);        // set as weak-v table
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_TIMEOUT_CB);

                // 注册一个用于存放挂起coroutine的列表
                lua_newtable(L);
                lua_pushvalue(L, -2);
                lua_setmetatable(L, -2);        // set as weak-v table
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_WAIT_CO);

                // 注册一个用于存放interval的回调列表
                lua_newtable(L);
                lua_pushvalue(L, -2);
                lua_setmetatable(L, -2);        // set as weak-v table
                lua_rawsetp(L, LUA_REGISTRYINDEX, &KEY_INTERVAL_CB);

                luaL_Reg timer_funcs[] = {
                    {"wait", wait},
                    {"sleep", wait},            // timer.wait(delay) == timer.sleep(delay)
                    {"set_timeout", set_timeout},
                    {"set_interval", set_interval},
                    {nullptr, nullptr}
                };
                luaL_newlib(L, timer_funcs);
                return 1;
            }

            void luaopen_timer(lua_State* L)
            {
                luaL_requiref(L, "timer", &_luaopen_timer, 0);
                lua_pop(L, 1);
            }
        }
    }
}
