#include "../../Core/ProcessUtils.h"
#include <sol/sol.hpp>

int sol2_CoreLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    lua["GetPlatform"] = []() { return Urho3D::GetPlatform().CString(); };
    return 0;
}