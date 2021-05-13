#include "../../Core/ProcessUtils.h"
#include "../../Core/Variant.h"

#include <sol/sol.hpp>
#include "GetPush.h"
using namespace Urho3D;

int sol2_CoreLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    lua["GetPlatform"] = []() { return Urho3D::GetPlatform().CString(); };
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
        "GetInt",       &Variant::GetInt,
        "GetUInt",      &Variant::GetUInt,
        "GetInt64",     &Variant::GetInt64,
        "GetUInt64",    &Variant::GetUInt64,
        "GetBool",      &Variant::GetBool,
        "GetFloat",     &Variant::GetFloat
    );
    lua.new_usertype<VariantMap>("VariantMap",// sol::constructors<VariantMap()>(),
        sol::meta_function::index, [](VariantMap* map, StringHash key) { return (*map)[key]; }
        );
    return 0;
}