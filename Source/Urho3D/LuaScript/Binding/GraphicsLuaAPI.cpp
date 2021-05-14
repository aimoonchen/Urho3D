#include "../../Core/Context.h"
#include "../../Resource/Resource.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/Renderer.h"
#include "../../Graphics/Camera.h"
#include "../../Graphics/Texture2D.h"
#include "../../Graphics/Drawable.h"
#include "../../Graphics/StaticModel.h"
#include "../../Graphics/Animation.h"
#include "../../Graphics/AnimationState.h"
#include "../../Graphics/AnimatedModel.h"
#include "../../Graphics/Model.h"
#include "../../Graphics/Material.h"
#include "../../Graphics/Zone.h"
#include "../../Graphics/Light.h"
#include "../../Scene/Component.h"

#include <sol/sol.hpp>
#include "GetPush.h"

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
    lua.new_usertype<BiasParameters>("BiasParameters",
		sol::call_constructor, sol::factories([](float constantBias, float slopeScaledBias) { return BiasParameters(constantBias, slopeScaledBias); }));
    lua.new_usertype<CascadeParameters>("CascadeParameters",
		sol::call_constructor, sol::factories([](float split1, float split2, float split3, float split4, float fadeStart) { return CascadeParameters(split1, split2, split3, split4, fadeStart); }));
    lua.new_usertype<Resource>("Resource", sol::constructors<Resource(Context*)>());
    lua.new_usertype<Material>("Material", sol::constructors<Material(Context*)>(),
		sol::base_classes, sol::bases<Resource>());
	lua.new_usertype<ResourceWithMetadata>("ResourceWithMetadata", sol::constructors<ResourceWithMetadata(Context*)>(),
		sol::base_classes, sol::bases<Resource>());
    lua.new_usertype<Animation>("Animation",
		"length", sol::property(&Animation::GetLength),
		sol::base_classes, sol::bases<ResourceWithMetadata>());
    lua.new_usertype<Model>("Model", sol::constructors<Model(Context*)>(),
		sol::base_classes, sol::bases<ResourceWithMetadata>());
	lua.new_usertype<Texture>("Texture", sol::constructors<Texture(Context*)>(),
		sol::base_classes, sol::bases<ResourceWithMetadata>());
	lua.new_usertype<Texture2D>("Texture2D", sol::constructors<Texture2D(Context*)>(),
		"width",sol::property(&Texture2D::GetWidth),
		"height", sol::property(&Texture2D::GetHeight),
		sol::base_classes, sol::bases<Texture>());
	lua.new_usertype<Viewport>("Viewport",// sol::constructors<Viewport(Context*), Viewport(Context*, Scene*, Camera*)>(),
		sol::call_constructor, sol::factories(
			[context]() { return std::make_unique<Viewport>(context); },
			[context](Scene* scene, Camera* camera) { return new Viewport(context, scene, camera);/*return std::make_unique<Viewport>(context, scene, camera);*/ }));
	lua.new_usertype<Camera>("Camera",
		"farClip", sol::property(&Camera::GetFarClip, &Camera::SetFarClip),
		sol::base_classes, sol::bases<Component>());
	lua.new_usertype<Graphics>("Graphics",
		"SetWindowIcon", &Graphics::SetWindowIcon,
		"windowTitle", sol::property(&Graphics::GetWindowTitle, &Graphics::SetWindowTitle));
	lua.new_usertype<Renderer>("Renderer", sol::constructors<Renderer(Context*)>(),
		"SetViewport", &Renderer::SetViewport);
	lua.new_usertype<Drawable>("Drawable",
		"castShadows", sol::property(&Drawable::GetCastShadows, &Drawable::SetCastShadows),
		sol::base_classes, sol::bases<Component>());
    lua.new_usertype<Zone>("Zone",
		"boundingBox",	sol::property([](Zone* obj) { return obj->GetBoundingBox(); }, &Zone::SetBoundingBox),
		"ambientColor", sol::property(&Zone::GetAmbientColor, &Zone::SetAmbientColor),
		"fogColor",		sol::property(&Zone::GetFogColor, &Zone::SetFogColor),
		"fogStart",		sol::property(&Zone::GetFogStart, &Zone::SetFogStart),
		"fogEnd",		sol::property(&Zone::GetFogEnd, &Zone::SetFogEnd),
		sol::base_classes, sol::bases<Drawable, Component>());
	lua.new_usertype<Light>("Light",
		"lightType", sol::property(&Light::GetLightType, &Light::SetLightType),
		"color", sol::property(&Light::GetColor, &Light::SetColor),
		"shadowBias", sol::property(&Light::GetShadowBias, &Light::SetShadowBias),
		"shadowCascade", sol::property(&Light::GetShadowCascade, &Light::SetShadowCascade),
		sol::base_classes, sol::bases<Drawable, Component>());
	lua.new_usertype<StaticModel>("StaticModel",
		"model", sol::property(&StaticModel::GetModel, &StaticModel::SetModel),
		"material", sol::property([](StaticModel* obj) { return obj->GetMaterial(0); }, [](StaticModel* obj, Material* mtl ) { obj->SetMaterial(mtl); }),
		sol::base_classes, sol::bases<Drawable, Component>());
	lua.new_usertype<AnimationState>("AnimationState",
		"AddTime", &AnimationState::AddTime,
		"weight", sol::property(&AnimationState::GetWeight, &AnimationState::SetWeight),
		"looped", sol::property(&AnimationState::IsLooped, &AnimationState::SetLooped),
		"time", sol::property(&AnimationState::GetTime, &AnimationState::SetTime));
	lua.new_usertype<AnimatedModel>("AnimatedModel",
		"AddAnimationState", &AnimatedModel::AddAnimationState,
		"GetAnimationState", sol::overload(sol::resolve<AnimationState*(Animation*) const>(&AnimatedModel::GetAnimationState),
			sol::resolve<AnimationState*(const String&) const>(&AnimatedModel::GetAnimationState),
			sol::resolve<AnimationState*(StringHash) const>(&AnimatedModel::GetAnimationState),
			sol::resolve<AnimationState*(unsigned) const>(&AnimatedModel::GetAnimationState)),
		"model", sol::property(&AnimatedModel::GetModel, [](AnimatedModel* obj, Model* model) { obj->SetModel(model); }),
		sol::base_classes, sol::bases<StaticModel, Drawable, Component>());
	lua["graphics"] = context->GetSubsystem<Graphics>();
	lua["renderer"] = context->GetSubsystem<Renderer>();
	RegisterConst(lua);
	
	return 0;
}