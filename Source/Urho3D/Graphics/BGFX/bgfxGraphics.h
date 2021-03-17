#pragma once
#include "../Urho3D/Graphics/GraphicsDefs.h"
#include "bgfx/bgfx.h"
uint64_t bgfxRSBend(Urho3D::BlendMode mode, bool alphaToCoverage = false);
uint64_t bgfxRSCull(Urho3D::CullMode mode);
uint64_t bgfxRSCompare(Urho3D::CompareMode mode);
uint64_t bgfxRSDepthWrite(bool enable);
uint64_t bgfxRSAlphaToCoverage(bool enable);