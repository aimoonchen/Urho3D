//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Precompiled.h"

#include "../Core/CoreEvents.h"
#include "../Core/ProcessUtils.h"
#include "../Core/Profiler.h"
#include "../Engine/EngineEvents.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../LuaScript/LuaFile.h"
#include "../LuaScript/LuaFunction.h"
#include "../LuaScript/LuaScript.h"
#include "../LuaScript/LuaScriptEventInvoker.h"
#include "../LuaScript/LuaScriptInstance.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Scene.h"

#include <sol/sol.hpp>
// extern "C"
// {
// #include "lualib.h"
// #include "lauxlib.h"
// #include "lua.h"
// }

//#include <toluapp/tolua++.h>
// #include "../LuaScript/ToluaUtils.h"

#include "../DebugNew.h"

extern int sol2_AudioLuaAPI_open(sol::state*);
extern int sol2_CoreLuaAPI_open(sol::state*);
extern int sol2_EngineLuaAPI_open(sol::state*);
extern int sol2_GraphicsLuaAPI_open(sol::state*);
extern int sol2_InputLuaAPI_open(sol::state*);
extern int sol2_IOLuaAPI_open(sol::state*);
extern int sol2_MathLuaAPI_open(sol::state*);
#ifdef URHO3D_NAVIGATION
extern int sol2_NavigationLuaAPI_open(sol::state*);
#endif
#ifdef URHO3D_NETWORK
extern int sol2_NetworkLuaAPI_open(sol::state*);
#endif
#ifdef URHO3D_DATABASE
extern int sol2_DatabaseLuaAPI_open(sol::state*);
#endif
#ifdef URHO3D_IK
extern int sol2_IKLuaAPI_open(sol::state*);
#endif
#ifdef URHO3D_PHYSICS
extern int sol2_PhysicsLuaAPI_open(sol::state*);
#endif
extern int sol2_ResourceLuaAPI_open(sol::state*);
extern int sol2_SceneLuaAPI_open(sol::state*);
extern int sol2_UILuaAPI_open(sol::state*);
#ifdef URHO3D_URHO2D
extern int sol2_Urho2DLuaAPI_open(sol::state*);
#endif
extern int sol2_LuaScriptLuaAPI_open(sol::state*);

/// Set context.
void SetContext(sol::state* lua, Urho3D::Context* context)
{
    (*lua)[".context"] = context;
}
/// Return context.
Urho3D::Context* GetContext(sol::state* lua)
{
    return (*lua)[".context"];
}
namespace Urho3D
{

LuaScript::LuaScript(Context* context) :
    Object(context),
    executeConsoleCommands_(false)
{
    RegisterLuaScriptLibrary(context_);

    luaState_ = std::make_unique<sol::state>();// luaL_newstate();
    if (!luaState_)
    {
        URHO3D_LOGERROR("Could not create Lua state");
        return;
    }

    lua_atpanic(luaState_->lua_state(), &LuaScript::AtPanic);
    luaState_->open_libraries();
    //luaL_openlibs(luaState_);
    RegisterLoader();
    //ReplacePrint();
    auto lua = luaState_.get();
    SetContext(lua, context_);
    sol2_MathLuaAPI_open(lua);
//     sol2_CoreLuaAPI_open(lua);
//     sol2_IOLuaAPI_open(lua);
    sol2_ResourceLuaAPI_open(lua);
    sol2_SceneLuaAPI_open(lua);
//     sol2_AudioLuaAPI_open(lua);
    sol2_EngineLuaAPI_open(lua);
//     sol2_GraphicsLuaAPI_open(lua);
//     sol2_InputLuaAPI_open(lua);
// #ifdef URHO3D_NAVIGATION
//     sol2_NavigationLuaAPI_open(lua);
// #endif
// #ifdef URHO3D_NETWORK
//     sol2_NetworkLuaAPI_open(lua);
// #endif
// #ifdef URHO3D_DATABASE
//     sol2_DatabaseLuaAPI_open(lua);
// #endif
// #ifdef URHO3D_IK
//     sol2_IKLuaAPI_open(lua);
// #endif
// #ifdef URHO3D_PHYSICS
//     sol2_PhysicsLuaAPI_open(lua);
// #endif
//     sol2_UILuaAPI_open(lua);
// #ifdef URHO3D_URHO2D
//     sol2_Urho2DLuaAPI_open(lua);
// #endif
//     sol2_LuaScriptLuaAPI_open(lua);
    

//    SetContext(luaState_.get(), context_);

    eventInvoker_ = new LuaScriptEventInvoker(context_);
    coroutineUpdate_ = GetFunction("coroutine.update");

    // Subscribe to post update
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(LuaScript, HandlePostUpdate));

    // Subscribe to console commands
    SetExecuteConsoleCommands(true);
}

LuaScript::~LuaScript()
{
    functionPointerToFunctionMap_.Clear();
    functionNameToFunctionMap_.Clear();

//     lua_State* luaState = luaState_;
//     luaState_ = nullptr;
    coroutineUpdate_ = nullptr;

//     if (luaState)
//         lua_close(luaState);
}

void LuaScript::AddEventHandler(const String& eventName, int index)
{
    LuaFunction* function = GetFunction(index);
    if (function)
        eventInvoker_->AddEventHandler(nullptr, eventName, function);
}

void LuaScript::AddEventHandler(const String& eventName, const String& functionName)
{
    LuaFunction* function = GetFunction(functionName);
    if (function)
        eventInvoker_->AddEventHandler(nullptr, eventName, function);
}

void LuaScript::AddEventHandler(Object* sender, const String& eventName, int index)
{
    if (!sender)
        return;

    LuaFunction* function = GetFunction(index);
    if (function)
        eventInvoker_->AddEventHandler(sender, eventName, function);
}

void LuaScript::AddEventHandler(Object* sender, const String& eventName, const String& functionName)
{
    if (!sender)
        return;

    LuaFunction* function = GetFunction(functionName);
    if (function)
        eventInvoker_->AddEventHandler(sender, eventName, function);
}

void LuaScript::RemoveEventHandler(const String& eventName)
{
    eventInvoker_->UnsubscribeFromEvent(eventName);
}

void LuaScript::RemoveEventHandler(Object* sender, const String& eventName)
{
    if (!sender)
        return;

    eventInvoker_->UnsubscribeFromEvent(sender, eventName);
}

void LuaScript::RemoveEventHandlers(Object* sender)
{
    if (!sender)
        return;

    eventInvoker_->UnsubscribeFromEvents(sender);
}

void LuaScript::RemoveAllEventHandlers()
{
    eventInvoker_->UnsubscribeFromAllEvents();
}

void LuaScript::RemoveEventHandlersExcept(const Vector<String>& exceptionNames)
{
    PODVector<StringHash> exceptionTypes(exceptionNames.Size());
    for (unsigned i = 0; i < exceptionTypes.Size(); ++i)
        exceptionTypes[i] = StringHash(exceptionNames[i]);

    eventInvoker_->UnsubscribeFromAllEventsExcept(exceptionTypes, true);
}

bool LuaScript::HasEventHandler(const String& eventName) const
{
    return eventInvoker_->HasSubscribedToEvent(eventName);
}

bool LuaScript::HasEventHandler(Object* sender, const String& eventName) const
{
    return eventInvoker_->HasSubscribedToEvent(sender, eventName);
}

bool LuaScript::ExecuteFile(const String& fileName)
{
    URHO3D_PROFILE(ExecuteFile);

#ifdef URHO3D_LUA_RAW_SCRIPT_LOADER
    if (ExecuteRawFile(fileName))
        return true;
#endif

    auto* cache = GetSubsystem<ResourceCache>();
    auto* luaFile = cache->GetResource<LuaFile>(fileName);
    return luaFile && luaFile->LoadAndExecute(luaState_->lua_state());
}

bool LuaScript::ExecuteString(const String& string)
{
    URHO3D_PROFILE(ExecuteString);
    auto L = luaState_->lua_state();
    if (luaL_dostring(L, string.CString()))
    {
        const char* message = lua_tostring(L, -1);
        URHO3D_LOGERRORF("Execute Lua string failed: %s", message);
        lua_pop(L, 1);
        return false;
    }

    return true;
}

bool LuaScript::LoadRawFile(const String& fileName)
{
    URHO3D_PROFILE(LoadRawFile);

    URHO3D_LOGINFO("Finding Lua file on file system: " + fileName);

    auto* cache = GetSubsystem<ResourceCache>();
    String filePath = cache->GetResourceFileName(fileName);

    if (filePath.Empty())
    {
        URHO3D_LOGINFO("Lua file not found: " + fileName);
        return false;
    }

    filePath = GetNativePath(filePath);

    URHO3D_LOGINFO("Loading Lua file from file system: " + filePath);
    auto L = luaState_->lua_state();
    if (luaL_loadfile(L, filePath.CString()))
    {
        const char* message = lua_tostring(L, -1);
        URHO3D_LOGERRORF("Load Lua file failed: %s", message);
        lua_pop(L, 1);
        return false;
    }

    URHO3D_LOGINFO("Lua file loaded: " + filePath);

    return true;
}

bool LuaScript::ExecuteRawFile(const String& fileName)
{
    URHO3D_PROFILE(ExecuteRawFile);

    if (!LoadRawFile(fileName))
        return false;
    auto L = luaState_->lua_state();
    if (lua_pcall(L, 0, 0, 0))
    {
        const char* message = lua_tostring(L, -1);
        URHO3D_LOGERRORF("Execute Lua file failed: %s", message);
        lua_pop(L, 1);
        return false;
    }

    return true;
}

bool LuaScript::ExecuteFunction(const String& functionName)
{
    LuaFunction* function = GetFunction(functionName);
    return function && function->BeginCall() && function->EndCall();
}

void LuaScript::SetExecuteConsoleCommands(bool enable)
{
    if (enable == executeConsoleCommands_)
        return;

    executeConsoleCommands_ = enable;
    if (enable)
        SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(LuaScript, HandleConsoleCommand));
    else
        UnsubscribeFromEvent(E_CONSOLECOMMAND);
}

void LuaScript::RegisterLoader()
{
    auto L = luaState_->lua_state();
    // Get package.loaders table
    lua_getglobal(L, "package");
    //lua_getfield(L, -1, "loaders");
    lua_getfield(L, -1, "preload");
    // Add LuaScript::Loader to the end of the table
    lua_pushinteger(L, lua_rawlen(L, -1) + 1);
    lua_pushcfunction(L, &LuaScript::Loader);
    lua_settable(L, -3);
    lua_pop(L, 2);
}

int LuaScript::AtPanic(lua_State* L)
{
    String errorMessage = luaL_checkstring(L, -1);
    URHO3D_LOGERROR("Lua error: Error message = '" + errorMessage + "'");
    lua_pop(L, 1);
    return 0;
}

int LuaScript::Loader(lua_State* L)
{
    // Get module name
    String fileName(luaL_checkstring(L, 1));

#ifdef URHO3D_LUA_RAW_SCRIPT_LOADER
    // First attempt to load lua script file from the file system
    // Attempt to load .luc file first, then fall back to .lua
    auto* lua = ::GetContext(L)->GetSubsystem<LuaScript>();
    if (lua->LoadRawFile(fileName + ".luc") || lua->LoadRawFile(fileName + ".lua"))
        return 1;
#endif

    auto* cache = ::GetContext(luaState_.get())->GetSubsystem<ResourceCache>();

    // Attempt to get .luc file first
    auto* lucFile = cache->GetResource<LuaFile>(fileName + ".luc", false);
    if (lucFile)
        return lucFile->LoadChunk(L) ? 1 : 0;

    // Then try to get .lua file. If this also fails, error is logged and
    // resource not found event is sent
    auto* luaFile = cache->GetResource<LuaFile>(fileName + ".lua");
    if (luaFile)
        return luaFile->LoadChunk(L) ? 1 : 0;

    return 0;
}

void LuaScript::ReplacePrint()
{
    static const struct luaL_Reg reg[] =
    {
        {"print", &LuaScript::Print},
        {nullptr, nullptr}
    };
    auto L = luaState_->lua_state();
    lua_getglobal(L, "_G");
    //luaL_register(L, nullptr, reg);
    luaL_setfuncs(L, reg, 0);
    //luaL_requiref(L, "print", &LuaScript::Print, 1);
    lua_pop(L, 1);
}

#define LUA_QL(x) "'" x "'"
//#define LUA_QS LUA_QL("%s")

int LuaScript::Print(lua_State* L)
{
    int n = lua_gettop(L);
    Vector<String> strings((unsigned)n);

    // Call the tostring function repeatedly for each arguments in the stack
    lua_getglobal(L, "tostring");
    for (int i = 1; i <= n; ++i)
    {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        const char* s = lua_tostring(L, -1);
        lua_pop(L, 1);
        if (s)
            strings[i - 1] = s;
        else
        {
            lua_pop(L, 1);
            return luaL_error(L, LUA_QL("tostring") " failed at index %d to return a string to " LUA_QL("print"), i);
        }
    }
    lua_pop(L, 1);

    URHO3D_LOGRAWF("%s\n", String::Joined(strings, "    ").CString());
    return 0;
}

LuaFunction* LuaScript::GetFunction(int index)
{
    auto L = luaState_->lua_state();
    if (!lua_isfunction(L, index))
        return nullptr;

    const void* functionPointer = lua_topointer(L, index);
    if (!functionPointer)
        return nullptr;

    HashMap<const void*, SharedPtr<LuaFunction> >::Iterator i = functionPointerToFunctionMap_.Find(functionPointer);
    if (i != functionPointerToFunctionMap_.End())
        return i->second_;

    SharedPtr<LuaFunction> function(new LuaFunction(L, index));
    functionPointerToFunctionMap_[functionPointer] = function;

    return function;
}

LuaFunction* LuaScript::GetFunction(const String& functionName, bool silentIfNotFound)
{
    auto L = luaState_->lua_state();
    if (!L)
        return nullptr;

    HashMap<String, SharedPtr<LuaFunction> >::Iterator i = functionNameToFunctionMap_.Find(functionName);
    if (i != functionNameToFunctionMap_.End())
        return i->second_;

    SharedPtr<LuaFunction> function;
    if (PushLuaFunction(L, functionName))
    {
        function = GetFunction(-1);
        functionNameToFunctionMap_[functionName] = function;
    }
    else if (!silentIfNotFound)
        URHO3D_LOGERRORF("%s", lua_tostring(L, -1));
    lua_pop(L, 1);

    return function;
}

void LuaScript::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if (coroutineUpdate_ && coroutineUpdate_->BeginCall())
    {
        using namespace PostUpdate;
        float timeStep = eventData[P_TIMESTEP].GetFloat();
        coroutineUpdate_->PushFloat(timeStep);
        coroutineUpdate_->EndCall();
    }

    // Collect garbage
    {
        URHO3D_PROFILE(LuaCollectGarbage);
        lua_gc(luaState_->lua_state(), LUA_GCSTEP, 0);
    }
}

void LuaScript::HandleConsoleCommand(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommand;
    if (eventData[P_ID].GetString() == GetTypeName())
        ExecuteString(eventData[P_COMMAND].GetString());
}

bool LuaScript::PushLuaFunction(lua_State* L, const String& functionName)
{
    Vector<String> splitNames = functionName.Split('.');

    String currentName = splitNames.Front();
    lua_getglobal(L, currentName.CString());

    if (splitNames.Size() > 1)
    {
        for (unsigned i = 0; i < splitNames.Size() - 1; ++i)
        {
            if (i)
            {
                currentName = currentName + "." + splitNames[i];
                lua_getfield(L, -1, splitNames[i].CString());
                lua_replace(L, -2);
            }
            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1);
                lua_pushstring(L, ("Could not find Lua table: Table name = '" + currentName + "'").CString());
                return false;
            }
        }

        currentName = currentName + "." + splitNames.Back();
        lua_getfield(L, -1, splitNames.Back().CString());
        lua_replace(L, -2);
    }

    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 1);
        lua_pushstring(L, ("Could not find Lua function: Function name = '" + currentName + "'").CString());
        return false;
    }

    return true;
}

void RegisterLuaScriptLibrary(Context* context)
{
    LuaFile::RegisterObject(context);
    LuaScriptInstance::RegisterObject(context);
}

}
