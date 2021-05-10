#include "../../Core/Context.h"
#include "../../Resource/Resource.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/Texture2D.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(lua_State* L);
using namespace Urho3D;

int sol2_GraphicsLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    lua.new_usertype<Resource>("Resource", sol::constructors<Resource(Context*)>());
	lua.new_usertype<ResourceWithMetadata>("ResourceWithMetadata", sol::constructors<ResourceWithMetadata(Context*)>(),
		sol::base_classes, sol::bases<Resource>());
	lua.new_usertype<Texture>("Texture", sol::constructors<Texture(Context*)>(),
		sol::base_classes, sol::bases<ResourceWithMetadata>());
	lua.new_usertype<Texture2D>("Texture2D", sol::constructors<Texture2D(Context*)>(),
		"width",sol::property(&Texture2D::GetWidth),
		"height", sol::property(&Texture2D::GetHeight),
		sol::base_classes, sol::bases<Texture>()
	);
	lua.new_usertype<Graphics>("Graphics", sol::constructors<Graphics(Context*)>(),
		"SetWindowIcon", &Graphics::SetWindowIcon,
		"windowTitle", sol::property([](Graphics* obj) { return obj->GetWindowTitle().CString(); },
                      [](Graphics* obj, std::string title) { obj->SetWindowTitle(title.c_str());}) // sol::property(&Graphics::GetWindowTitle, &Graphics::SetWindowTitle)
	);

	auto context = GetContext(lua.lua_state());
	lua["graphics"] = context->GetSubsystem<Graphics>();
	return 0;
}