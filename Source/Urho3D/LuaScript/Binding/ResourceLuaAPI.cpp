#include "../../Core/Context.h"
#include "../../Resource/Resource.h"
#include "../../Resource/ResourceCache.h"
#include "../../Resource/Image.h"
#include "../../Resource/XMLFile.h"
#include "../../Graphics/Model.h"
#include "../../Graphics/Material.h"
#include "../../Graphics/Texture2D.h"
#include "../../Graphics/Animation.h"
#include "../../UI/Font.h"
#include <sol/sol.hpp>
#include "GetPush.h"

using namespace Urho3D;
int sol_lua_push(sol::types<Resource*>, lua_State* L, const Resource* obj)
{
    if (obj->GetTypeName() == "Texture2D")
    {
        return sol::make_object(L, static_cast<const Texture2D*>(obj)).push(L);
    }
    else if (obj->GetTypeName() == "Image")
    {
        return sol::make_object(L, static_cast<const Image*>(obj)).push(L);
    }
    else if (obj->GetTypeName() == "XMLFile")
    {
        return sol::make_object(L, static_cast<const XMLFile*>(obj)).push(L);
    }
    else if (obj->GetTypeName() == "Model")
    {
        return sol::make_object(L, static_cast<const Model*>(obj)).push(L);
    }
    else if (obj->GetTypeName() == "Material")
    {
        return sol::make_object(L, static_cast<const Material*>(obj)).push(L);
    }
    else if (obj->GetTypeName() == "Font")
    {
        return sol::make_object(L, static_cast<const Font*>(obj)).push(L);
    }
    else if (obj->GetTypeName() == "Animation")
    {
        return sol::make_object(L, static_cast<const Animation*>(obj)).push(L);
    }
    else
    {
        return sol::make_object(L, obj).push(L);
    }
}

Urho3D::Context* GetContext(lua_State* L);

int sol2_ResourceLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    auto context = GetContext(lua.lua_state());
    lua.new_usertype<ResourceCache>("ResourceCache",
        "GetResource", [](ResourceCache* obj, StringHash typeName, const String& filePath) { return obj->GetResource(typeName, filePath); }
    );
    lua.new_usertype<Image>("Image", sol::constructors<Image(Context*)>());
    lua.new_usertype<XMLFile>("XMLFile", sol::constructors<XMLFile(Context*)>());
    lua["cache"] = context->GetSubsystem<ResourceCache>();
    return 0;
}