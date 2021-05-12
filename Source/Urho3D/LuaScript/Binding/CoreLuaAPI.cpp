#include "../../Core/ProcessUtils.h"
#include "../../Core/Variant.h"
#include <sol/sol.hpp>

using namespace Urho3D;

template <typename Handler>
bool sol_lua_check(sol::types<String>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
{
    int absolute_index = lua_absindex(L, index);
    bool success = sol::stack::check<const char*>(L, absolute_index, handler);
    tracking.use(1);

    return success;
}

String sol_lua_get(sol::types<String>, lua_State* L, int index, sol::stack::record& tracking)
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

int sol2_CoreLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    lua["GetPlatform"] = []() { return Urho3D::GetPlatform().CString(); };
//     lua.new_usertype<String>("String",
//         sol::call_constructor, sol::factories(
//             []() { return String(); },
//             [](const char* str) { return String(str); },
//             [](char* str) { return String(str); },
//             [](const wchar_t* str) { return String(str); },
//             [](wchar_t* str) { return String(str); })
//     );
//     lua.new_usertype<WString>("WString",
//         sol::call_constructor, sol::factories([]() { return WString(); }, [](const String& str) { return WString(str); })
//     );
    lua.new_usertype<Variant>("Variant",
        sol::call_constructor, sol::factories(
            []() { return Variant(); },
            [](int value) { return Variant(value); },
            [](long long value) { return Variant(value); },
            [](unsigned value) { return Variant(value); },
            [](unsigned long long value) { return Variant(value); },
            [](const StringHash& value) { return Variant(value); },
            [](bool value) { return Variant(value); },
            [](float value) { return Variant(value); },
            [](double value) { return Variant(value); },
            [](const Vector2& value) { return Variant(value); },
            [](const Vector3& value) { return Variant(value); },
            [](const Vector4& value) { return Variant(value); },
            [](const Quaternion& value) { return Variant(value); },
            [](const Color& value) { return Variant(value); },
            [](const String& value) { return Variant(value); },
            [](const ResourceRef& value) { return Variant(value); },
            [](const ResourceRefList& value) { return Variant(value); },
            [](const VariantVector& value) { return Variant(value); },
            [](const VariantMap& value) { return Variant(value); },
            [](const StringVector& value) { return Variant(value); },
            [](const Rect& value) { return Variant(value); },
            [](const IntRect& value) { return Variant(value); },
            [](const IntVector2& value) { return Variant(value); },
            [](const IntVector3& value) { return Variant(value); },
            [](const Matrix3& value) { return Variant(value); },
            [](const Matrix3x4& value) { return Variant(value); },
            [](const Matrix4& value) { return Variant(value); }
        ),
        "GetInt", &Variant::GetInt,
        "GetUInt", &Variant::GetUInt,
        "GetInt64", &Variant::GetInt64,
        "GetUInt64", &Variant::GetUInt64,
        "GetBool", &Variant::GetBool,
        "GetFloat", &Variant::GetFloat
    );
    lua.new_usertype<VariantMap>("VariantMap", sol::constructors<VariantMap()>(),
        sol::meta_function::index, [](VariantMap* map, std::string key) { return (*map)[key.c_str()]; }
        );
    return 0;
}