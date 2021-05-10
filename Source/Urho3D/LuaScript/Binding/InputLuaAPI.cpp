#include "../../Core/Context.h"
#include "../../Input/Input.h"
#include <sol/sol.hpp>

using namespace Urho3D;
Urho3D::Context* GetContext(lua_State* L);

int sol2_InputLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    
    lua.new_usertype<Input>("Input", sol::constructors<Input(Context*)>(),
        "GetNumJoysticks", &Input::GetNumJoysticks,
        "AddScreenJoystick", &Input::AddScreenJoystick,
        "SetScreenJoystickVisible", &Input::SetScreenJoystickVisible,
        "GetNumJoysticks", &Input::GetNumJoysticks,
        "SetMouseMode", &Input::SetMouseMode,
        "GetNumTouches", &Input::GetNumTouches,
        "GetTouch", &Input::GetTouch,
        "touchEmulation", sol::property(&Input::GetTouchEmulation, &Input::SetTouchEmulation),
        "mouseVisible", sol::property(&Input::IsMouseVisible, &Input::SetMouseVisible),
        "mouseMode", sol::property(&Input::GetMouseMode, &Input::SetMouseMode)
        );
    lua["MM_ABSOLUTE"] = MM_ABSOLUTE;
    lua["MM_RELATIVE"] = MM_RELATIVE;
    lua["MM_WRAP"] = MM_WRAP;
    lua["MM_FREE"] = MM_FREE;
    lua["MM_INVALID"] = MM_INVALID;
    auto context = GetContext(lua.lua_state());
    lua["input"] = context->GetSubsystem<Input>();
    return 0;
}