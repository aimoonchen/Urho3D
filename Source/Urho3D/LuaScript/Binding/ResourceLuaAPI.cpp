#include "../../Core/Context.h"
#include "../../Resource/Resource.h"
#include "../../Resource/ResourceCache.h"
#include "../../Graphics/Texture2D.h"
#include <sol/sol.hpp>

Urho3D::Context* GetContext(lua_State* L);
using namespace Urho3D;
int sol_lua_push(sol::types<Resource*>, lua_State* L, const Resource* obj)
{
    if (obj->GetTypeName() == "Texture2D")
    {
        return sol::make_object(L, static_cast<const Texture2D*>(obj)).push(L);
    }
    else
    {
        return sol::make_object(L, obj).push(L);
    }
}
int sol2_ResourceLuaAPI_open(sol::state* luaState)
{
    auto& lua = *luaState;
    auto context = GetContext(lua.lua_state());
    lua.new_usertype<ResourceCache>("ResourceCache", sol::constructors<ResourceCache(Context*)>(),
        "GetResource", [&lua](ResourceCache* obj, std::string typeName, std::string filePath) {
            return obj->GetResource(typeName.c_str(), filePath.c_str());
//            auto ret = obj->GetResource(typeName.c_str(), filePath.c_str());
//             if (typeName == "Texture2D")
//             {
//                 sol::stack::push_userdata(lua, (Texture2D*)ret);
//             }
        }
    );
    lua["cache"] = context->GetSubsystem<ResourceCache>();
    return 0;
}