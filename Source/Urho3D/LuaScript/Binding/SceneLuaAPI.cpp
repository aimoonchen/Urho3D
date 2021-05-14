#include <sol/sol.hpp>
#include "GetPush.h"
#include "../../Core/Context.h"
#include "../../Scene/Node.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Component.h"
#include "../../Graphics/Light.h"
#include "../../Graphics/StaticModel.h"
#include "../../Graphics/AnimatedModel.h"
#include "../../Graphics/Octree.h"
#include "../../Graphics/Camera.h"
#include "../../Graphics/Zone.h"
#include "../../Graphics/DebugRenderer.h"
#include "../LuaScriptInstance.h"

using namespace Urho3D;

Urho3D::Context* GetContext(lua_State* L);

int sol_lua_push(sol::types<Component*>, lua_State* L, const Component* obj)
{
    if (obj)
    {
        if (obj->GetTypeName() == "StaticModel")
        {
            return sol::make_object(L, static_cast<const StaticModel*>(obj)).push(L);
        }
        else if (obj->GetTypeName() == "AnimatedModel")
        {
            return sol::make_object(L, static_cast<const AnimatedModel*>(obj)).push(L);
        }
        else if (obj->GetTypeName() == "Light")
        {
            return sol::make_object(L, static_cast<const Light*>(obj)).push(L);
        }
        else if (obj->GetTypeName() == "Octree")
        {
            return sol::make_object(L, static_cast<const Octree*>(obj)).push(L);
        }
        else if (obj->GetTypeName() == "Camera")
        {
            return sol::make_object(L, static_cast<const Camera*>(obj)).push(L);
        }
        else if (obj->GetTypeName() == "DebugRenderer")
        {
            return sol::make_object(L, static_cast<const DebugRenderer*>(obj)).push(L);
        }
        else if (obj->GetTypeName() == "Zone")
        {
            return sol::make_object(L, static_cast<const Zone*>(obj)).push(L);
        }
    }
    return sol::make_object(L, obj).push(L);
}

int sol2_SceneLuaAPI_open(sol::state* lua)
{
	auto context = GetContext(lua->lua_state());
	lua->new_usertype<Component>("Component");
    lua->new_usertype<Node>("Node",
        "scale",    sol::property(&Node::GetScale, [](Node* obj, const Vector3& scale) { obj->SetScale(scale); }/*sol::overload([](Node* obj, float scale) { obj->SetScale(scale); }, [](Node* obj, const Vector3& scale) { obj->SetScale(scale); })*/),
        "rotation", sol::property(&Node::GetRotation, &Node::SetRotation),
        "position", sol::property(&Node::GetPosition, &Node::SetPosition),
        "SetScale", sol::overload(sol::resolve<void(float)>(&Node::SetScale), sol::resolve<void(const Vector3&)>(&Node::SetScale)),
        "Translate", [](Node* obj, const Vector3& translate) { obj->Translate(translate); },
        "Yaw", [](Node* obj, float angle) { obj->Yaw(angle); },
		"direction", sol::property(&Node::GetDirection, &Node::SetDirection),
		"CreateChild", [](Node* obj, const String& name) { return obj->CreateChild(name); },
		"CreateComponent", [](Node* obj, StringHash type) { return obj->CreateComponent(type); },
        "GetComponent", sol::overload([](Node* obj, StringHash type) { return obj->GetComponent(type); }, [](Node* obj, StringHash type, bool recursive) { return obj->GetComponent(type, recursive); }),
        "CreateScriptObject", [lua](Node* obj, const String& name) {
            auto instance = obj->CreateComponent<LuaScriptInstance>();
            instance->CreateObject(name);
            return instance->GetScriptObject();
        });
	lua->new_usertype<Scene>("Scene",
		sol::call_constructor, sol::factories([context]() { return std::make_unique<Scene>(context); }),
		sol::base_classes, sol::bases<Node>()
	);
    return 0;
}