#pragma once
#include "../Urho3D/Graphics/GraphicsDefs.h"
#include "bgfx/bgfx.h"
uint64_t bgfxRSBend(Urho3D::BlendMode mode, bool alphaToCoverage = false);
uint64_t bgfxRSCull(Urho3D::CullMode mode);
uint64_t bgfxRSDepthCompare(Urho3D::CompareMode mode);
uint64_t bgfxRSStencilCompare(Urho3D::CompareMode mode);
uint64_t bgfxRSStencilFail(Urho3D::StencilOp fail);
uint64_t bgfxRSDepthFail(Urho3D::StencilOp fail);
uint64_t bgfxRSDepthPass(Urho3D::StencilOp pass);
uint64_t bgfxRSDepthWrite(bool enable);
uint64_t bgfxRSAlphaToCoverage(bool enable);
uint8_t bgfxCubeMapSide(Urho3D::CubeMapFace);