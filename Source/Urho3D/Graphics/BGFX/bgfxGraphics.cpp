
#include "bgfx/bgfx.h"
#include "../Urho3D/Graphics/GraphicsDefs.h"

uint64_t bgfxRSBend(Urho3D::BlendMode mode, bool alphaToCoverage)
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
    if (alphaToCoverage)
    {
        flag |= BGFX_STATE_BLEND_ALPHA_TO_COVERAGE;
    }
    return flag;
}

uint64_t bgfxRSCull(Urho3D::CullMode mode)
{
    return (mode == Urho3D::CULL_NONE) ? 0 : (((mode == Urho3D::CULL_CCW) ? BGFX_STATE_CULL_CCW : BGFX_STATE_CULL_CW));
}

uint64_t bgfxRSDepthCompare(Urho3D::CompareMode mode)
{
    uint64_t flag = 0;
    switch (mode) {
    case Urho3D::CMP_ALWAYS:
        flag = BGFX_STATE_DEPTH_TEST_ALWAYS;
        break;
    case Urho3D::CMP_EQUAL:
        flag = BGFX_STATE_DEPTH_TEST_EQUAL;
        break;
    case Urho3D::CMP_NOTEQUAL:
        flag = BGFX_STATE_DEPTH_TEST_NOTEQUAL;
        break;
    case Urho3D::CMP_LESS:
        flag = BGFX_STATE_DEPTH_TEST_LESS;
        break;
    case Urho3D::CMP_LESSEQUAL:
        flag = BGFX_STATE_DEPTH_TEST_LEQUAL;
        break;
    case Urho3D::CMP_GREATER:
        flag = BGFX_STATE_DEPTH_TEST_GREATER;
        break;
    case Urho3D::CMP_GREATEREQUAL:
        flag = BGFX_STATE_DEPTH_TEST_GEQUAL;
        break;
    default:
        break;
    }
    return flag;
}

uint64_t bgfxRSStencilFail(Urho3D::StencilOp fail)
{
    switch (fail) {
    case Urho3D::OP_KEEP:
        return BGFX_STENCIL_OP_FAIL_S_KEEP;
        break;
    case Urho3D::OP_ZERO:
        return BGFX_STENCIL_OP_FAIL_S_ZERO;
        break;
    case Urho3D::OP_REF:
        return BGFX_STENCIL_OP_FAIL_S_REPLACE;
        break;
    case Urho3D::OP_INCR:
        return BGFX_STENCIL_OP_FAIL_S_INCR;
        break;
    case Urho3D::OP_DECR:
        return BGFX_STENCIL_OP_FAIL_S_DECR;
        break;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSDepthFail(Urho3D::StencilOp fail)
{
    switch (fail) {
    case Urho3D::OP_KEEP:
        return BGFX_STENCIL_OP_FAIL_Z_KEEP;
        break;
    case Urho3D::OP_ZERO:
        return BGFX_STENCIL_OP_FAIL_Z_ZERO;
        break;
    case Urho3D::OP_REF:
        return BGFX_STENCIL_OP_FAIL_Z_REPLACE;
        break;
    case Urho3D::OP_INCR:
        return BGFX_STENCIL_OP_FAIL_Z_INCR;
        break;
    case Urho3D::OP_DECR:
        return BGFX_STENCIL_OP_FAIL_Z_DECR;
        break;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSDepthPass(Urho3D::StencilOp pass)
{
    switch (pass) {
    case Urho3D::OP_KEEP:
        return BGFX_STENCIL_OP_PASS_Z_KEEP;
        break;
    case Urho3D::OP_ZERO:
        return BGFX_STENCIL_OP_PASS_Z_ZERO;
        break;
    case Urho3D::OP_REF:
        return BGFX_STENCIL_OP_PASS_Z_REPLACE;
        break;
    case Urho3D::OP_INCR:
        return BGFX_STENCIL_OP_PASS_Z_INCR;
        break;
    case Urho3D::OP_DECR:
        return BGFX_STENCIL_OP_PASS_Z_DECR;
        break;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSStencilCompare(Urho3D::CompareMode mode)
{
    switch (mode) {
    case Urho3D::CMP_ALWAYS:
        return BGFX_STENCIL_TEST_ALWAYS;
        break;
    case Urho3D::CMP_EQUAL:
        return BGFX_STENCIL_TEST_EQUAL;
        break;
    case Urho3D::CMP_NOTEQUAL:
        return BGFX_STENCIL_TEST_NOTEQUAL;
        break;
    case Urho3D::CMP_LESS:
        return BGFX_STENCIL_TEST_LESS;
        break;
    case Urho3D::CMP_LESSEQUAL:
        return BGFX_STENCIL_TEST_LEQUAL;
        break;
    case Urho3D::CMP_GREATER:
        return BGFX_STENCIL_TEST_GREATER;
        break;
    case Urho3D::CMP_GREATEREQUAL:
        return BGFX_STENCIL_TEST_GEQUAL;
        break;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSDepthWrite(bool enable)
{
    return enable ? BGFX_STATE_WRITE_Z : 0;
}

uint64_t bgfxRSAlphaToCoverage(bool enable)
{
    return enable ? BGFX_STATE_BLEND_ALPHA_TO_COVERAGE : 0;
}