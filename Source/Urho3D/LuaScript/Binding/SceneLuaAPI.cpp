#include "../../Core/Context.h"
#include "../../Scene/Node.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Component.h"
#include <sol/sol.hpp>

using namespace Urho3D;

Urho3D::Context* GetContext(lua_State* L);

int sol2_SceneLuaAPI_open(sol::state* lua)
{
	auto context = GetContext(lua->lua_state());
//    lua->new_usertype<RefCounted>("RefCounted", sol::constructors<Node(Context*)>());
    lua->new_usertype<Context>("Context", sol::constructors<Context()>());
	lua->new_usertype<Component>("Component", sol::constructors<Component(Context*)>());
	lua->new_usertype<Node>("Node", sol::constructors<Node(Context*)>());
	lua->new_usertype<Scene>("Scene", sol::constructors<Scene(Context*)>(),
//		sol::call_constructor, sol::factories([context]() { return Scene(context); }),
		"CreateComponent", [](Scene* obj, const char* name) { return obj->CreateComponent(name); },// &Scene::CreateComponent,
		"CreateChild", [](Scene* obj, const char* name) { return obj->CreateChild(name); },// &Scene::CreateChild
		sol::base_classes, sol::bases<Node>()
	);
    return 0;
}