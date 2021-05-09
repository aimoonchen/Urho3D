#include "LuaScript/LuaScript.h"
#include "LuaScript/LuaScriptInstance.h"
using namespace Urho3D;
#pragma warning(disable : 4800)

static void RegisterEventName(const String eventName) { GetEventNameRegister().RegisterString(eventName.CString()); }

static LuaScript* GetLuaScript(lua_State* L) { return GetContext(L)->GetSubsystem<LuaScript>(); }

#define LuaScriptAddEventHandler GetLuaScript(tolua_S)->AddEventHandler
#define LuaScriptRemoveEventHandler GetLuaScript(tolua_S)->RemoveEventHandler
#define LuaScriptRemoveEventHandlers GetLuaScript(tolua_S)->RemoveEventHandlers
#define LuaScriptRemoveAllEventHandlers GetLuaScript(tolua_S)->RemoveAllEventHandlers
#define LuaScriptRemoveEventHandlersExcept GetLuaScript(tolua_S)->RemoveEventHandlersExcept
#define LuaScriptHasSubscribedToEvent GetLuaScript(tolua_S)->HasEventHandler

#define LuaScriptSendEvent GetLuaScript(tolua_S)->SendEvent
#define LuaScriptSetExecuteConsoleCommands GetLuaScript(tolua_S)->SetExecuteConsoleCommands
#define LuaScriptGetExecuteConsoleCommands GetLuaScript(tolua_S)->GetExecuteConsoleCommands

#define LuaScriptSetGlobalVar GetLuaScript(tolua_S)->SetGlobalVar
#define LuaScriptGetGlobalVar GetLuaScript(tolua_S)->GetGlobalVar
#define LuaScriptGetGlobalVars GetLuaScript(tolua_S)->GetGlobalVars