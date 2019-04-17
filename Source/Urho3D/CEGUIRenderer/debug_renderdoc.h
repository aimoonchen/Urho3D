#pragma once

namespace bgfx
{
	URHO3D_API void* loadRenderDoc();
	void unloadRenderDoc(void*);
	void renderDocTriggerCapture();

}