#ifndef BASE_LUA_DBO_LUA_DBO_H
#define BASE_LUA_DBO_LUA_DBO_H

class lua_State;

namespace base
{
    namespace lua
    {
        namespace dbo
        {
            /**
             * ---- example
             * 
             * local dbo = require('dbo')
             * 
             * local thread = coroutine.create(function()
             *     local conn = dbo.connect(0)
             *     
             *     local ok, rs = conn:execute('insert into user (username, level, gold) values (?, ?, ?)', 'jimmy', 45, 8888)
             *     local ok, rs = conn:execute('select * from user where uid = ?', 12)
             *     local ok, rs = conn:execute('update user set level = ? where username = ?', 77, 'jimmy')
             *     
             *     if ok then
             *        print(rs.affected_rows, rs.last_insert_id)
             *        -- if has data, row_count = #rs
             *        if #rs > 0 then
             *              for i, row in ipairs(rs) do
             *                      print(i, row.field1, row.field2)
             *              end
             *        end
             *     else
             *        print('error :', rs)
             *     end
             *     conn:close()     -- do not forget release the connection
             * end)
             * 
             * coroutine.resume(thread)
             * 
             */
            
            void luaopen_dbo(lua_State* L);
            void luaclose_dbo(lua_State* L);
        }
    }
}

#endif
