#pragma once
#include "../Urho3D/Graphics/GraphicsDefs.h"
#include "bgfx/bgfx.h"
namespace bgfx {
	uint64_t Urho3DBlendToBGFXBlend(Urho3D::BlendMode mode, bool alphaToCoverage = false);
}