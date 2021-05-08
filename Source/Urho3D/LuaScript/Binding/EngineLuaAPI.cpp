#include "../../Core/Context.h"
#include "../../Engine/Engine.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(sol::state* lua);

int sol2_EngineLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    auto context = GetContext(luaState);
    lua["engine"] = context->GetSubsystem<Urho3D::Engine>();
    return 0;
}
