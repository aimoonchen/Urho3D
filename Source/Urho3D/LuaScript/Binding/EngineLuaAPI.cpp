#include "../../Core/Context.h"
#include "../../Engine/Engine.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(lua_State* L);

int sol2_EngineLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    auto context = GetContext(lua.lua_state());
    lua["engine"] = context->GetSubsystem<Urho3D::Engine>();
    return 0;
}
