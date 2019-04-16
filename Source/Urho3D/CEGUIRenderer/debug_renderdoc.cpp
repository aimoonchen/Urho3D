/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

//#include "bgfx_p.h"
#include <windows.h>
#include <malloc.h>
#define BX_PLATFORM_WINDOWS 1
#define BX_PLATFORM_LINUX 1


#if BX_PLATFORM_WINDOWS || BX_PLATFORM_LINUX
#	if BX_PLATFORM_WINDOWS
#		include <psapi.h>
#	endif // BX_PLATFORM_WINDOWS
#	include "renderdoc/renderdoc_app.h"

namespace bgfx
{
	bool findModule(const char* _name)
	{
#if BX_PLATFORM_WINDOWS
		HANDLE process = GetCurrentProcess();
		DWORD size;
		BOOL result = EnumProcessModules(process
						, NULL
						, 0
						, &size
						);
		if (0 != result)
		{
			HMODULE* modules = (HMODULE*)_alloca(size);
			result = EnumProcessModules(process
				, modules
				, size
				, &size
				);

			if (0 != result)
			{
				char moduleName[MAX_PATH];
				for (uint32_t ii = 0, num = uint32_t(size/sizeof(HMODULE) ); ii < num; ++ii)
				{
					result = GetModuleBaseNameA(process
								, modules[ii]
								, moduleName
								, MAX_PATH//BX_COUNTOF(moduleName)
								);
					if (0 != result
					/*&&  0 == bx::strCmpI(_name, moduleName)*/ )
					{
						return true;
					}
				}
			}
		}
#endif // BX_PLATFORM_WINDOWS
		//BX_UNUSED(_name);
		return false;
	}

	pRENDERDOC_GetAPI RENDERDOC_GetAPI;
	static RENDERDOC_API_1_1_2* s_renderDoc = NULL;
	static void* s_renderDocDll = NULL;

	void* loadRenderDoc()
	{
		if (NULL != s_renderDoc)
		{
			return s_renderDocDll;
		}

		// Skip loading RenderDoc when IntelGPA is present to avoid RenderDoc crash.
		if (findModule(false/*BX_ARCH_32BIT*/ ? "shimloader32.dll" : "shimloader64.dll") )
		{
			return NULL;
		}

// 		void* renderDocDll = bx::dlopen(
// #if BX_PLATFORM_WINDOWS
// 				"renderdoc.dll"
// #else
// 				"./librenderdoc.so"
// #endif // BX_PLATFORM_WINDOWS
// 				);
		void* renderDocDll = (void*)::LoadLibraryA("renderdoc.dll");
		if (NULL != renderDocDll)
		{
			//RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)bx::dlsym(renderDocDll, "RENDERDOC_GetAPI");
// 			const int32_t symbolMax = _symbol.getLength() + 1;
// 			char* symbol = (char*)alloca(symbolMax);
// 			bx::strCopy(symbol, symbolMax, _symbol);
			RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)::GetProcAddress((HMODULE)renderDocDll, "RENDERDOC_GetAPI");

			if (NULL != RENDERDOC_GetAPI
			&&  1 == RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&s_renderDoc) )
			{
				s_renderDoc->SetCaptureFilePathTemplate("temp/bgfx"/*BGFX_CONFIG_RENDERDOC_LOG_FILEPATH*/);

 				s_renderDoc->SetFocusToggleKeys(NULL, 0);

// 				RENDERDOC_InputButton captureKeys[] = BGFX_CONFIG_RENDERDOC_CAPTURE_KEYS;
// 				s_renderDoc->SetCaptureKeys(captureKeys, BX_COUNTOF(captureKeys) );

				s_renderDoc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync,      1);
				s_renderDoc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1);

				s_renderDoc->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);

				s_renderDocDll = renderDocDll;
			}
			else
			{
				//bx::dlclose(renderDocDll);
				::FreeLibrary((HMODULE)renderDocDll);
				renderDocDll = NULL;
			}
		}

		return renderDocDll;
	}

	void unloadRenderDoc(void* _renderdocdll)
	{
		if (NULL != _renderdocdll)
		{
			// BK - Once RenderDoc is loaded there shouldn't be calls
			// to Shutdown or unload RenderDoc DLL.
			// https://github.com/bkaradzic/bgfx/issues/1192
			//
			// s_renderDoc->Shutdown();
			// bx::dlclose(_renderdocdll);
		}
	}

	void renderDocTriggerCapture()
	{
		if (NULL != s_renderDoc)
		{
			s_renderDoc->TriggerCapture();
		}
	}

} // namespace bgfx

#else

namespace bgfx
{

	void* loadRenderDoc()
	{
		return nullptr;
	}

	void unloadRenderDoc(void*)
	{
	}

	void renderDocTriggerCapture()
	{
	}

}

#endif // BX_PLATFORM_WINDOWS || BX_PLATFORM_LINUX
