#include "../../Core/Context.h"
#include "../../Input/Input.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(lua_State* L);

int sol2_InputLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    auto context = GetContext(lua.lua_state());
    lua["input"] = context->GetSubsystem<Urho3D::Input>();
    return 0;
}