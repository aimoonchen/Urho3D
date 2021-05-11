#include "../../Core/Context.h"
#include "../../Input/Input.h"
#include "../../Input/InputConstants.h"
#include <sol/sol.hpp>

using namespace Urho3D;
Urho3D::Context* GetContext(lua_State* L);

static void RegisterKeyboard(sol::state& lua)
{
    lua["KEY_F1"] = KEY_F1;
    lua["KEY_F2"] = KEY_F2;
    lua["KEY_SELECT"] = KEY_SELECT;
    lua["KEY_ESCAPE"] = KEY_ESCAPE;
    lua["KEY_1"] = KEY_1;
    lua["KEY_2"] = KEY_2;
    lua["KEY_3"] = KEY_3;
    lua["KEY_4"] = KEY_4;
    lua["KEY_5"] = KEY_5;
    lua["KEY_6"] = KEY_6;
    lua["KEY_7"] = KEY_7;
    lua["KEY_8"] = KEY_8;
    lua["KEY_9"] = KEY_9;
}

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
        "mouseMode", sol::property(&Input::GetMouseMode, [](Input* obj, MouseMode mode) { obj->SetMouseMode(mode); })
        );
    //
    lua["MM_ABSOLUTE"] = MM_ABSOLUTE;
    lua["MM_RELATIVE"] = MM_RELATIVE;
    lua["MM_WRAP"] = MM_WRAP;
    lua["MM_FREE"] = MM_FREE;
    lua["MM_INVALID"] = MM_INVALID;
    
    RegisterKeyboard(lua);

    auto context = GetContext(lua.lua_state());
    lua["input"] = context->GetSubsystem<Input>();
    return 0;
}