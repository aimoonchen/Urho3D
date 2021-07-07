
//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "EffekseerUrho3D.RenderState.h"
#include "EffekseerUrho3D.RendererImplemented.h"

#include "EffekseerUrho3D.IndexBuffer.h"
#include "EffekseerUrho3D.ModelRenderer.h"
#include "EffekseerUrho3D.Shader.h"
#include "EffekseerUrho3D.VertexBuffer.h"

namespace EffekseerUrho3D
{
	
namespace ModelShaders
{

namespace Unlit
{
#define DISTORTION 0
#define LIGHTING 0
namespace Lightweight
{
#define SOFT_PARTICLE 0
#include "Shaders/Model.inl"
#undef SOFT_PARTICLE
}
namespace SoftParticle
{
#define SOFT_PARTICLE 1
#include "Shaders/Model.inl"
#undef SOFT_PARTICLE
}
namespace CanvasItem
{
#include "Shaders/Model2D.inl"
}
#undef LIGHTING
#undef DISTORTION
}

namespace Lighting
{
#define DISTORTION 0
#define LIGHTING 1
namespace Lightweight
{
#define SOFT_PARTICLE 0
#include "Shaders/Model.inl"
#undef SOFT_PARTICLE
}
namespace SoftParticle
{
#define SOFT_PARTICLE 1
#include "Shaders/Model.inl"
#undef SOFT_PARTICLE
}
namespace CanvasItem
{
#include "Shaders/Model2D.inl"
}
#undef LIGHTING
#undef DISTORTION
}

namespace Distortion
{
#define DISTORTION 1
#define LIGHTING 0
namespace Lightweight
{
#define SOFT_PARTICLE 0
#include "Shaders/Model.inl"
#undef SOFT_PARTICLE
}
namespace SoftParticle
{
#define SOFT_PARTICLE 1
#include "Shaders/Model.inl"
#undef SOFT_PARTICLE
}
namespace CanvasItem
{
#include "Shaders/Model2D.inl"
}
#undef LIGHTING
#undef DISTORTION
}

}

static Urho3D::StringHash GetValidUniform(Shader* shader, const char* name)
{
    auto uniform_name = Urho3D::StringHash(name);
    if (shader->HasUniform(uniform_name))
    {
        return uniform_name;
    }
    else
    {
        return "";
    }
};

template <int N>
void ModelRenderer::InitRenderer()
{
    auto applyPSAdvancedRendererParameterTexture = [](Shader* shader, int32_t offset) -> void {
        shader->SetTextureSlot(0 + offset, GetValidUniform(shader, "s_sampler_alphaTex"));
        shader->SetTextureSlot(1 + offset, GetValidUniform(shader, "s_sampler_uvDistortionTex"));
        shader->SetTextureSlot(2 + offset, GetValidUniform(shader, "s_sampler_blendTex"));
        shader->SetTextureSlot(3 + offset, GetValidUniform(shader, "s_sampler_blendAlphaTex"));
        shader->SetTextureSlot(4 + offset, GetValidUniform(shader, "s_sampler_blendUVDistortionTex"));
    };
//     auto shader_ad_unlit_ = m_shaders[(size_t)RendererShaderType::AdvancedUnlit].get();
//     auto shader_ad_lit_ = m_shaders[(size_t)RendererShaderType::AdvancedLit].get();
//     auto shader_ad_distortion_ = m_shaders[(size_t)RendererShaderType::AdvancedBackDistortion].get();
    auto shader_unlit_ = m_shaders[(size_t)EffekseerRenderer::RendererShaderType::Unlit].get();
    auto shader_lit_ = m_shaders[(size_t)EffekseerRenderer::RendererShaderType::Lit].get();
    auto shader_distortion_ = m_shaders[(size_t)EffekseerRenderer::RendererShaderType::BackDistortion].get();
//     shader_ad_lit_->SetVertexConstantBufferSize(
//         sizeof(::EffekseerRenderer::ModelRendererAdvancedVertexConstantBuffer<N>));
//     shader_ad_unlit_->SetVertexConstantBufferSize(
//         sizeof(::EffekseerRenderer::ModelRendererAdvancedVertexConstantBuffer<N>));
//     shader_ad_distortion_->SetVertexConstantBufferSize(
//         sizeof(::EffekseerRenderer::ModelRendererAdvancedVertexConstantBuffer<N>));
//     shader_ad_lit_->SetPixelConstantBufferSize(sizeof(::EffekseerRenderer::PixelConstantBuffer));
//     shader_ad_unlit_->SetPixelConstantBufferSize(sizeof(::EffekseerRenderer::PixelConstantBuffer));
//     shader_ad_distortion_->SetPixelConstantBufferSize(sizeof(::EffekseerRenderer::PixelConstantBufferDistortion));

    shader_lit_->SetVertexConstantBufferSize(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<N>));
    shader_unlit_->SetVertexConstantBufferSize(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<N>));
    shader_distortion_->SetVertexConstantBufferSize(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<N>));
    shader_lit_->SetPixelConstantBufferSize(sizeof(::EffekseerRenderer::PixelConstantBuffer));
    shader_unlit_->SetPixelConstantBufferSize(sizeof(::EffekseerRenderer::PixelConstantBuffer));
    shader_distortion_->SetPixelConstantBufferSize(sizeof(::EffekseerRenderer::PixelConstantBufferDistortion));

//     for (auto& shader : {/*shader_ad_lit_, */shader_lit_}) {
//         shader->SetTextureSlot(0, GetValidUniform(shader, "sDiffMap"));
//         shader->SetTextureSlot(1, GetValidUniform(shader, "sNormalMap"));
//     }
//    applyPSAdvancedRendererParameterTexture(shader_ad_lit_, 2);
//    shader_lit_->SetTextureSlot(2, shader_lit_->GetUniformId("Sampler_sampler_depthTex"));
//    shader_ad_lit_->SetTextureSlot(7, shader_ad_lit_->GetUniformId("Sampler_sampler_depthTex"));

    for (auto& shader : {/*shader_ad_unlit_, */shader_unlit_})
    {
        shader->SetTextureSlot(0, GetValidUniform(shader, "sDiffMap"));
    }
//    applyPSAdvancedRendererParameterTexture(shader_ad_unlit_, 1);
    shader_unlit_->SetTextureSlot(1, GetValidUniform(shader_unlit_, "sNormalMap"));
//    shader_ad_unlit_->SetTextureSlot(6, shader_ad_unlit_->GetUniformId("Sampler_sampler_depthTex"));

    for (auto& shader : {/*shader_ad_distortion_, */shader_distortion_})
    {
        shader->SetTextureSlot(0, GetValidUniform(shader, "sDiffMap"));
        shader->SetTextureSlot(2, GetValidUniform(shader, "sSpecMap"));
    }
//    applyPSAdvancedRendererParameterTexture(shader_ad_distortion_, 2);
    shader_distortion_->SetTextureSlot(1, GetValidUniform(shader_distortion_, "sNormalMap"));
//    shader_ad_distortion_->SetTextureSlot(7, shader_ad_distortion_->GetUniformId("Sampler_sampler_depthTex"));

    Shader* shaders[4];
    shaders[0] = nullptr;// shader_ad_lit_;
    shaders[1] = nullptr;// shader_ad_unlit_;
    shaders[2] = shader_lit_;
    shaders[3] = shader_unlit_;

    for (int32_t i = 0; i < 4; i++) {
        if (i <= 2) continue;

        auto isAd = i < 2;

        int vsOffset = 0;
        shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shaders[i], "mCameraProj"), vsOffset);
        vsOffset += sizeof(Effekseer::Matrix44);
        if (VertexType == EffekseerRenderer::ModelRendererVertexType::Instancing) {
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shaders[i], "mModel_Inst"), vsOffset, N);
        } else {
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shaders[i], "mModel"), vsOffset, N);
        }
        vsOffset += sizeof(Effekseer::Matrix44) * N;
        shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fUV"), vsOffset, N);
        vsOffset += sizeof(float[4]) * N;
        if (isAd) {
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fAlphaUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fUVDistortionUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fBlendUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fBlendAlphaUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fBlendUVDistortionUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fFlipbookParameter"), vsOffset);
            vsOffset += sizeof(float[4]) * 1;
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fFlipbookIndexAndNextRate"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fModelAlphaThreshold"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
        }
        shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fModelColor"), vsOffset, N);
        vsOffset += sizeof(float[4]) * N;
        shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fLightDirection"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fLightColor"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "fLightAmbient"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        shaders[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders[i], "mUVInversed"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        AssignPixelConstantBuffer(shaders[i]);
    }

    Shader* shaders_d[2];
    shaders_d[0] = nullptr;// shader_ad_distortion_;
    shaders_d[1] = shader_distortion_;

    for (int32_t i = 0; i < 2; i++) {
        auto isAd = i < 1;
        if (isAd) {
            continue;
        }
        int vsOffset = 0;
        shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shaders_d[i], "mCameraProj"), vsOffset);
        vsOffset += sizeof(Effekseer::Matrix44);
        if (VertexType == EffekseerRenderer::ModelRendererVertexType::Instancing) {
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shaders_d[i], "mModel_Inst"), vsOffset, N);
        } else {
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shaders_d[i], "mModel"), vsOffset, N);
        }
        vsOffset += sizeof(Effekseer::Matrix44) * N;
        shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fUV"), vsOffset,
                                              N);
        vsOffset += sizeof(float[4]) * N;
        if (isAd) {
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fAlphaUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fUVDistortionUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fBlendUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fBlendAlphaUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fBlendUVDistortionUV"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fFlipbookParameter"), vsOffset);
            vsOffset += sizeof(float[4]) * 1;
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fFlipbookIndexAndNextRate"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
            shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fModelAlphaThreshold"), vsOffset, N);
            vsOffset += sizeof(float[4]) * N;
        }
        shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fModelColor"), vsOffset, N);
        vsOffset += sizeof(float[4]) * N;
        shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fLightDirection"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fLightColor"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "fLightAmbient"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        shaders_d[i]->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shaders_d[i], "mUVInversed"), vsOffset);
        vsOffset += sizeof(float[4]) * 1;
        AssignDistortionPixelConstantBuffer(shaders_d[i]);
    }
}

static const int InstanceCount = 10;

ModelRenderer::ModelRenderer(RendererImplemented* renderer)
	: m_renderer(renderer)
{
	using namespace EffekseerRenderer;
	using namespace EffekseerUrho3D::ModelShaders;

	m_shaders[(size_t)RendererShaderType::Unlit] = Shader::Create(renderer->GetUrho3DGraphics(), "Effekseer/UnlitModel", RendererShaderType::Unlit);
	m_shaders[(size_t)RendererShaderType::Unlit]->SetVertexConstantBufferSize(sizeof(ModelRendererVertexConstantBuffer<40>));
	m_shaders[(size_t)RendererShaderType::Unlit]->SetPixelConstantBufferSize(sizeof(PixelConstantBuffer));
	m_shaders[(size_t)RendererShaderType::Unlit]->Compile(Shader::RenderType::SpatialLightweight, Unlit::Lightweight::code, Unlit::Lightweight::decl);
	m_shaders[(size_t)RendererShaderType::Unlit]->Compile(Shader::RenderType::SpatialDepthFade, Unlit::SoftParticle::code, Unlit::SoftParticle::decl);
	m_shaders[(size_t)RendererShaderType::Unlit]->Compile(Shader::RenderType::CanvasItem, Unlit::CanvasItem::code, Unlit::CanvasItem::decl);

	m_shaders[(size_t)RendererShaderType::Lit] = Shader::Create(renderer->GetUrho3DGraphics(), "Effekseer/UnlitModel"/*"Model_Basic_Lighting"*/, RendererShaderType::Lit);
	m_shaders[(size_t)RendererShaderType::Lit]->SetVertexConstantBufferSize(sizeof(ModelRendererVertexConstantBuffer<40>));
	m_shaders[(size_t)RendererShaderType::Lit]->SetPixelConstantBufferSize(sizeof(PixelConstantBuffer));
	m_shaders[(size_t)RendererShaderType::Lit]->Compile(Shader::RenderType::SpatialLightweight, Lighting::Lightweight::code, Lighting::Lightweight::decl);
	m_shaders[(size_t)RendererShaderType::Lit]->Compile(Shader::RenderType::SpatialDepthFade, Lighting::SoftParticle::code, Lighting::SoftParticle::decl);
	m_shaders[(size_t)RendererShaderType::Lit]->Compile(Shader::RenderType::CanvasItem, Lighting::CanvasItem::code, Lighting::CanvasItem::decl);

	m_shaders[(size_t)RendererShaderType::BackDistortion] = Shader::Create(renderer->GetUrho3DGraphics(), "Effekseer/BackDistortionModel", RendererShaderType::BackDistortion);
	m_shaders[(size_t)RendererShaderType::BackDistortion]->SetVertexConstantBufferSize(sizeof(ModelRendererVertexConstantBuffer<40>));
	m_shaders[(size_t)RendererShaderType::BackDistortion]->SetPixelConstantBufferSize(sizeof(PixelConstantBuffer));
	m_shaders[(size_t)RendererShaderType::BackDistortion]->Compile(Shader::RenderType::SpatialLightweight, Distortion::Lightweight::code, Distortion::Lightweight::decl);
	m_shaders[(size_t)RendererShaderType::BackDistortion]->Compile(Shader::RenderType::SpatialDepthFade, Distortion::SoftParticle::code, Distortion::SoftParticle::decl);
	m_shaders[(size_t)RendererShaderType::BackDistortion]->Compile(Shader::RenderType::CanvasItem, Distortion::CanvasItem::code, Distortion::CanvasItem::decl);
    
	if (false/*renderer->GetDeviceType() == OpenGLDeviceType::OpenGL3 || renderer->GetDeviceType() == OpenGLDeviceType::OpenGLES3*/) {
        VertexType = EffekseerRenderer::ModelRendererVertexType::Instancing;
        InitRenderer<InstanceCount>();
    } else {
        InitRenderer<1>();
    }
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ModelRenderer::~ModelRenderer()
{
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ModelRendererRef ModelRenderer::Create(RendererImplemented* renderer)
{
	assert(renderer != NULL);

	return ModelRendererRef(new ModelRenderer(renderer));
}

void ModelRenderer::BeginRendering(const efkModelNodeParam& parameter, int32_t count, void* userData)
{
	BeginRendering_(m_renderer, parameter, count, userData);
}

void ModelRenderer::Rendering(const efkModelNodeParam& parameter, const InstanceParameter& instanceParameter, void* userData)
{
	Rendering_<RendererImplemented>(m_renderer, parameter, instanceParameter, userData);
}

void ModelRenderer::EndRendering(const efkModelNodeParam& parameter, void* userData)
{
	if (parameter.ModelIndex < 0)
	{
		return;
	}

    Effekseer::ModelRef model{ nullptr };

	if (parameter.IsProceduralMode)
	{
		model = parameter.EffectPointer->GetProceduralModel(parameter.ModelIndex);
	}
	else
	{
		model = parameter.EffectPointer->GetModel(parameter.ModelIndex);
	}

	if (model == nullptr)
	{
		return;
	}

//     model->StoreBufferToGPU(graphicsDevice_.Get());
//     if (!model->GetIsBufferStoredOnGPU())
//     {
//         return;
//     }
// 
//     if (m_renderer->GetRenderMode() == Effekseer::RenderMode::Wireframe)
//     {
//         model->GenerateWireIndexBuffer(graphicsDevice_.Get());
//         if (!model->GetIsWireIndexBufferGenerated())
//         {
//             return;
//         }
//     }
 
//	m_renderer->SetModel(model);

	using namespace EffekseerRenderer;
    if (VertexType == EffekseerRenderer::ModelRendererVertexType::Instancing) {
        EndRendering_<RendererImplemented, Shader, Effekseer::Model, true, InstanceCount>(
            m_renderer, m_shaders[(size_t)RendererShaderType::AdvancedLit].get(),
            m_shaders[(size_t)RendererShaderType::AdvancedUnlit].get(),
            m_shaders[(size_t)RendererShaderType::AdvancedBackDistortion].get(),
            m_shaders[(size_t)RendererShaderType::Lit].get(), m_shaders[(size_t)RendererShaderType::Unlit].get(),
            m_shaders[(size_t)RendererShaderType::BackDistortion].get(), parameter, userData);
    } else {
        EndRendering_<RendererImplemented, Shader, Effekseer::Model, false, 1>(
            m_renderer, m_shaders[(size_t)RendererShaderType::AdvancedLit].get(),
            m_shaders[(size_t)RendererShaderType::AdvancedUnlit].get(),
            m_shaders[(size_t)RendererShaderType::AdvancedBackDistortion].get(),
            m_shaders[(size_t)RendererShaderType::Lit].get(), m_shaders[(size_t)RendererShaderType::Unlit].get(),
            m_shaders[(size_t)RendererShaderType::BackDistortion].get(), parameter, userData);
    }
//	m_renderer->SetModel(nullptr);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
