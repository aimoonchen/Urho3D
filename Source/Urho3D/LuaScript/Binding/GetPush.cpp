#include "GetPush.h"

using namespace Urho3D;

String sol_lua_get(sol::types<String>, lua_State* L, int index, sol::stack::record& tracking)
{
    tracking.use(1);
    size_t len;
    const char* p = lua_tolstring(L, index, &len);
    return String(p, (unsigned int)len);
}

Urho3D::String sol_lua_get(sol::types<const Urho3D::String&>, lua_State* L, int index, sol::stack::record& tracking)
{
    tracking.use(1);
    size_t len;
    const char* p = lua_tolstring(L, index, &len);
    return String(p, (unsigned int)len);
}

int sol_lua_push(lua_State* L, const String& str)
{
    lua_pushlstring(L, str.CString(), str.Length());
    return 1;
}

StringHash sol_lua_get(sol::types<StringHash>, lua_State* L, int index, sol::stack::record& tracking)
{
    tracking.use(1);
    size_t len;
    const char* p = lua_tolstring(L, index, &len);
    return StringHash(p);
}
// 
// int sol_lua_push(lua_State* L, StringHash str)
// {
//      const auto& cstr = str.ToString();
//      return sol::stack::push(L, cstr.CString());
// }