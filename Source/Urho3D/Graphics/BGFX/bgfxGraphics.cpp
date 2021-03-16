
#include "bgfx/bgfx.h"
#include "../Urho3D/Graphics/GraphicsDefs.h"

namespace bgfx {
	uint64_t Urho3DBlendToBGFXBlend(Urho3D::BlendMode mode, bool alphaToCoverage)
	{
        uint64_t flag = 0;
        switch (mode) {
        case Urho3D::BLEND_ADD:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
            break;
        case Urho3D::BLEND_MULTIPLY:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
            break;
        case Urho3D::BLEND_ALPHA:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
            break;
        case Urho3D::BLEND_ADDALPHA:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
            break;
        case Urho3D::BLEND_PREMULALPHA:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
            break;
        case Urho3D::BLEND_INVDESTALPHA:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_INV_DST_ALPHA, BGFX_STATE_BLEND_DST_ALPHA);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
            break;
        case Urho3D::BLEND_SUBTRACT:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB);
            break;
        case Urho3D::BLEND_SUBTRACTALPHA:
            flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
            flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB);
            break;
        default:
            break;
        }

        if (alphaToCoverage) {
            flag |= BGFX_STATE_BLEND_ALPHA_TO_COVERAGE;
        }
        return flag;
	}
}