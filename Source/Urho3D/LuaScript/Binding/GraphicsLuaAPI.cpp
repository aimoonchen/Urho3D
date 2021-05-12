#include "../../Core/Context.h"
#include "../../Resource/Resource.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/Renderer.h"
#include "../../Graphics/Camera.h"
#include "../../Graphics/Texture2D.h"
#include "../../Graphics/Drawable.h"
#include "../../Graphics/StaticModel.h"
#include "../../Graphics/Model.h"
#include "../../Graphics/Material.h"
#include "../../Graphics/Light.h"
#include "../../Scene/Component.h"

#include <sol/sol.hpp>

Urho3D::Context* GetContext(lua_State* L);
using namespace Urho3D;

static void RegisterConst(sol::state& lua)
{
    lua["LIGHT_DIRECTIONAL"]	= LIGHT_DIRECTIONAL;
    lua["LIGHT_SPOT"]			= LIGHT_SPOT;
    lua["LIGHT_POINT"]			= LIGHT_POINT;
}

int sol2_GraphicsLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
	auto context = GetContext(lua.lua_state());
    lua.new_usertype<Resource>("Resource", sol::constructors<Resource(Context*)>());
    lua.new_usertype<Material>("Material", sol::constructors<Material(Context*)>(),
		sol::base_classes, sol::bases<Resource>());
	lua.new_usertype<ResourceWithMetadata>("ResourceWithMetadata", sol::constructors<ResourceWithMetadata(Context*)>(),
		sol::base_classes, sol::bases<Resource>());
	lua.new_usertype<Texture>("Texture", sol::constructors<Texture(Context*)>(),
		sol::base_classes, sol::bases<ResourceWithMetadata>());
	lua.new_usertype<Texture2D>("Texture2D", sol::constructors<Texture2D(Context*)>(),
		"width",sol::property(&Texture2D::GetWidth),
		"height", sol::property(&Texture2D::GetHeight),
		sol::base_classes, sol::bases<Texture>());
	lua.new_usertype<Viewport>("Viewport",// sol::constructors<Viewport(Context*), Viewport(Context*, Scene*, Camera*)>(),
		sol::call_constructor, sol::factories(
			[context]() { return std::make_unique<Viewport>(context); },
			[context](Scene* scene, Camera* camera) { return new Viewport(context, scene, camera);/*return std::make_unique<Viewport>(context, scene, camera);*/ })
		);
	lua.new_usertype<Camera>("Camera", sol::constructors<Camera(Context*)>(),
		sol::base_classes, sol::bases<Component>());
	lua.new_usertype<Graphics>("Graphics", sol::constructors<Graphics(Context*)>(),
		"SetWindowIcon", &Graphics::SetWindowIcon,
		"windowTitle", sol::property(&Graphics::GetWindowTitle, [](Graphics* obj, const std::string& title){ obj->SetWindowTitle(title.c_str()); }/*&Graphics::SetWindowTitle*/) // sol::property(&Graphics::GetWindowTitle, &Graphics::SetWindowTitle)
	);
	lua.new_usertype<Renderer>("Renderer", sol::constructors<Renderer(Context*)>(),
		"SetViewport", &Renderer::SetViewport
	);
    lua.new_usertype<Drawable>("Drawable", sol::base_classes, sol::bases<Component>());
    lua.new_usertype<Light>("Light", sol::constructors<Light(Context*)>(),
		"lightType", sol::property(&Light::GetLightType, &Light::SetLightType),
		sol::base_classes, sol::bases<Drawable>());
    lua.new_usertype<Model>("Model", sol::constructors<Model(Context*)>(),
		sol::base_classes, sol::bases<ResourceWithMetadata>());
	lua.new_usertype<StaticModel>("StaticModel", sol::constructors<StaticModel(Context*)>(),
		"model", sol::property(&StaticModel::GetModel, &StaticModel::SetModel),
		"material", sol::property([](StaticModel* obj) { return obj->GetMaterial(0); }, [](StaticModel* obj, Material* mtl ) { obj->SetMaterial(mtl); }),
		sol::base_classes, sol::bases<Drawable>());
	
	lua["graphics"] = context->GetSubsystem<Graphics>();
	lua["renderer"] = context->GetSubsystem<Renderer>();
	RegisterConst(lua);
	
	return 0;
}