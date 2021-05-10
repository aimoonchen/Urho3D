#include "../../Core/Context.h"
#include "../../UI/UI.h"
#include "../../UI/UIElement.h"
#include "../../UI/Sprite.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(lua_State* L);

using namespace Urho3D;

int sol_lua_push(sol::types<UIElement*>, lua_State* L, const UIElement* obj)
{
    if (obj->GetTypeName() == "Sprite")
    {
        return sol::make_object(L, static_cast<const Sprite*>(obj)).push(L);
    }
    else
    {
        return sol::make_object(L, obj).push(L);
    }
}

int sol2_UILuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    lua.new_usertype<UIElement>("UIElement", sol::constructors<UIElement(Context*)>(),
        "SetSize", sol::overload([](UIElement* obj, int w, int h) {obj->SetSize(w, h); }, [](UIElement* obj, IntVector2 v2) {obj->SetSize(v2); }),
        "CreateChild", [&lua](UIElement* obj, std::string typeName) {
            return obj->CreateChild(typeName.c_str());
        }
//         "SetTexture", [](UIElement* obj, Resource* res) {
//             auto sprite = dynamic_cast<Sprite*>(obj);
//             if (sprite)
//                 sprite->SetTexture((Texture*)res);
//         }
    );
    lua.new_usertype<Sprite>("Sprite", sol::constructors<Sprite(Context*)>(),
        "SetTexture", &Sprite::SetTexture,
        "SetScale", sol::overload([](Sprite* obj, float sx, float sy) {obj->SetScale(sx, sy); }, [](Sprite* obj, Vector2 v2) {obj->SetScale(v2); }),
        "SetAlignment", &Sprite::SetAlignment,
        "hotSpot", sol::property(&Sprite::GetHotSpot, [](Sprite* obj, IntVector2 v2) {obj->SetHotSpot(v2); }),
        "opacity", sol::property(&Sprite::GetOpacity, &Sprite::SetOpacity),
        "priority", sol::property(&Sprite::GetPriority, &Sprite::SetPriority),
        sol::base_classes, sol::bases<UIElement>()
    );
    lua.new_usertype<UI>("UI", sol::constructors<UI(Context*)>(),
        "root", sol::property(&UI::GetRoot)
    );
    auto context = GetContext(lua);
    lua["ui"] = context->GetSubsystem<UI>();
    return 0;
}