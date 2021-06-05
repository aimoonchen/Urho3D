#pragma once
#include <sol/sol.hpp>
#include "../../Math/StringHash.h"
template <typename Handler>
bool sol_lua_check(sol::types<const Urho3D::String&>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
    int absolute_index = lua_absindex(L, index);
    bool success = sol::stack::check<const char*>(L, absolute_index, handler);
    tracking.use(1);

    return success;
}
template <typename Handler>
bool sol_lua_check(sol::types<Urho3D::String>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
    int absolute_index = lua_absindex(L, index);
    bool success = sol::stack::check<const char*>(L, absolute_index, handler);
    tracking.use(1);

    return success;
}
Urho3D::String sol_lua_get(sol::types<const Urho3D::String&>, lua_State* L, int index, sol::stack::record& tracking);
Urho3D::String sol_lua_get(sol::types<Urho3D::String>, lua_State* L, int index, sol::stack::record& tracking);
int sol_lua_push(lua_State* L, const Urho3D::String& str);

template <typename Handler>
bool sol_lua_check(sol::types<Urho3D::StringHash>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
    int absolute_index = lua_absindex(L, index);
    bool success = sol::stack::check<const char*>(L, absolute_index, handler);
    tracking.use(1);

    return success;
}
Urho3D::StringHash sol_lua_get(sol::types<Urho3D::StringHash>, lua_State* L, int index, sol::stack::record& tracking);
// int sol_lua_push(lua_State* L, StringHash str);

//namespace Urho3D {
//class UIElement;
//}
//int sol_lua_push(sol::types<Urho3D::UIElement*>, lua_State* L, const Urho3D::UIElement* obj);