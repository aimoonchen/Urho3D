#include "../../Core/Context.h"
#include "../../Resource/ResourceCache.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(sol::state* lua);

int sol2_ResourceLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    auto context = GetContext(luaState);
    lua["cache"] = context->GetSubsystem<Urho3D::ResourceCache>();
    return 0;
}