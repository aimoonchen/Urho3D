#include "../../Core/Context.h"
#include "../../Resource/ResourceCache.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(lua_State* L);

int sol2_ResourceLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    auto context = GetContext(lua.lua_state());
    lua["cache"] = context->GetSubsystem<Urho3D::ResourceCache>();
    return 0;
}