#include "../../Core/Context.h"
#include "../../Scene/Node.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Component.h"
#include "../../Graphics/Light.h"
#include "../../Graphics/StaticModel.h"
#include "../../Graphics/Octree.h"
#include "../../Graphics/Camera.h"
#include <sol/sol.hpp>

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
    }
    return sol::make_object(L, obj).push(L);
}

int sol2_SceneLuaAPI_open(sol::state* lua)
{
	auto context = GetContext(lua->lua_state());
//    lua->new_usertype<RefCounted>("RefCounted", sol::constructors<Node(Context*)>());
    lua->new_usertype<Context>("Context", sol::constructors<Context()>());
	lua->new_usertype<Component>("Component", sol::constructors<Component(Context*)>());
    lua->new_usertype<Node>("Node", sol::constructors<Node(Context*)>(),
        "scale", sol::property(&Node::GetScale, [](Node* obj, const Vector3& scale) { obj->SetScale(scale); }/*sol::overload([](Node* obj, float scale) { obj->SetScale(scale); }, [](Node* obj, const Vector3& scale) { obj->SetScale(scale); })*/),
        "rotation", sol::property(&Node::GetRotation, &Node::SetRotation),
        "position", sol::property(&Node::GetPosition, &Node::SetPosition),
        "SetScale", [](Node* obj, float scale) { obj->SetScale(scale); },
        "Translate", [](Node* obj, const Vector3& translate) { obj->Translate(translate); },
		"direction", sol::property(&Node::GetDirection, &Node::SetDirection),
		"CreateChild", [](Node* obj, const String& name) { return obj->CreateChild(name); }, // sol::overload(sol::resolve<Node*(const String&, CreateMode, unsigned, bool)>(&Node::CreateChild)),//
		"CreateComponent", [](Node* obj, const StringHash& type) { return obj->CreateComponent(type); }, // &Node::CreateComponent,
        "GetComponent", [](Node* obj, const StringHash& type) { return obj->GetComponent(type); } // Node::GetComponent
		);
	lua->new_usertype<Scene>("Scene",// sol::constructors<Scene(Context*)>(),
		sol::call_constructor, sol::factories([context]() { return std::make_unique<Scene>(context); }),
		sol::base_classes, sol::bases<Node>()
	);
    return 0;
}