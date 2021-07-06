
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
    // 	auto& uniforms = shader->uniforms_;
    // 	auto it = uniforms.find(name);
    // 	if (it != uniforms.end()) {
    // 		return it->second;
    // 	}
    // 	else {
    // 		return { /*UINT16_MAX*/ };
    // 	}
};

ModelRenderer::ModelRenderer(RendererImplemented* renderer)
	: m_renderer(renderer)
{
	using namespace EffekseerRenderer;
	using namespace EffekseerUrho3D::ModelShaders;

	m_shaders[(size_t)RendererShaderType::Unlit] = Shader::Create(renderer->GetUrho3DGraphics(), "Effekseer/Unlit", RendererShaderType::Unlit);
	m_shaders[(size_t)RendererShaderType::Unlit]->SetVertexConstantBufferSize(sizeof(ModelRendererVertexConstantBuffer<40>));
	m_shaders[(size_t)RendererShaderType::Unlit]->SetPixelConstantBufferSize(sizeof(PixelConstantBuffer));
	m_shaders[(size_t)RendererShaderType::Unlit]->Compile(Shader::RenderType::SpatialLightweight, Unlit::Lightweight::code, Unlit::Lightweight::decl);
	m_shaders[(size_t)RendererShaderType::Unlit]->Compile(Shader::RenderType::SpatialDepthFade, Unlit::SoftParticle::code, Unlit::SoftParticle::decl);
	m_shaders[(size_t)RendererShaderType::Unlit]->Compile(Shader::RenderType::CanvasItem, Unlit::CanvasItem::code, Unlit::CanvasItem::decl);

	m_shaders[(size_t)RendererShaderType::Lit] = Shader::Create(renderer->GetUrho3DGraphics(), "Effekseer/Unlit"/*"Model_Basic_Lighting"*/, RendererShaderType::Lit);
	m_shaders[(size_t)RendererShaderType::Lit]->SetVertexConstantBufferSize(sizeof(ModelRendererVertexConstantBuffer<40>));
	m_shaders[(size_t)RendererShaderType::Lit]->SetPixelConstantBufferSize(sizeof(PixelConstantBuffer));
	m_shaders[(size_t)RendererShaderType::Lit]->Compile(Shader::RenderType::SpatialLightweight, Lighting::Lightweight::code, Lighting::Lightweight::decl);
	m_shaders[(size_t)RendererShaderType::Lit]->Compile(Shader::RenderType::SpatialDepthFade, Lighting::SoftParticle::code, Lighting::SoftParticle::decl);
	m_shaders[(size_t)RendererShaderType::Lit]->Compile(Shader::RenderType::CanvasItem, Lighting::CanvasItem::code, Lighting::CanvasItem::decl);

	m_shaders[(size_t)RendererShaderType::BackDistortion] = Shader::Create(renderer->GetUrho3DGraphics(), "Effekseer/BackDistortion", RendererShaderType::BackDistortion);
	m_shaders[(size_t)RendererShaderType::BackDistortion]->SetVertexConstantBufferSize(sizeof(ModelRendererVertexConstantBuffer<40>));
	m_shaders[(size_t)RendererShaderType::BackDistortion]->SetPixelConstantBufferSize(sizeof(PixelConstantBuffer));
	m_shaders[(size_t)RendererShaderType::BackDistortion]->Compile(Shader::RenderType::SpatialLightweight, Distortion::Lightweight::code, Distortion::Lightweight::decl);
	m_shaders[(size_t)RendererShaderType::BackDistortion]->Compile(Shader::RenderType::SpatialDepthFade, Distortion::SoftParticle::code, Distortion::SoftParticle::decl);
	m_shaders[(size_t)RendererShaderType::BackDistortion]->Compile(Shader::RenderType::CanvasItem, Distortion::CanvasItem::code, Distortion::CanvasItem::decl);
    
	auto applyPSAdvancedRendererParameterTexture = [](Shader* shader, int32_t offset) -> void {
        shader->SetTextureSlot(0 + offset, GetValidUniform(shader, "s_sampler_alphaTex"));
        shader->SetTextureSlot(1 + offset, GetValidUniform(shader, "s_sampler_uvDistortionTex"));
        shader->SetTextureSlot(2 + offset, GetValidUniform(shader, "s_sampler_blendTex"));
        shader->SetTextureSlot(3 + offset, GetValidUniform(shader, "s_sampler_blendAlphaTex"));
        shader->SetTextureSlot(4 + offset, GetValidUniform(shader, "s_sampler_blendUVDistortionTex"));
    };

    auto shader_unlit = m_shaders[static_cast<size_t>(EffekseerRenderer::RendererShaderType::Unlit)].get();
    // auto shader_ad_unlit =
    // m_shaders[static_cast<size_t>(EffekseerRenderer::RendererShaderType::AdvancedUnlit)].get();
    for (auto& shader : {/*shader_ad_unlit, */ shader_unlit})
    {
        shader->SetVertexConstantBufferSize(sizeof(EffekseerRenderer::StandardRendererVertexBuffer));
        shader->SetPixelConstantBufferSize(sizeof(EffekseerRenderer::PixelConstantBuffer));

        shader->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shader, "mflipbookParameter"),
                                        sizeof(Effekseer::Matrix44) * 2 + sizeof(float) * 4);
        shader->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shader, "mCamera"), 0);
        shader->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shader, "mCameraProj"),
                                        sizeof(Effekseer::Matrix44));
        shader->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shader, "mUVInversed"),
                                        sizeof(Effekseer::Matrix44) * 2);
        shader->SetTextureSlot(0, GetValidUniform(shader, "sDiffMap"));
        AssignPixelConstantBuffer(shader);
    }

    // applyPSAdvancedRendererParameterTexture(shader_ad_unlit, 1);
    shader_unlit->SetTextureSlot(1, GetValidUniform(shader_unlit, "sNormalMap"));
    // shader_ad_unlit->SetTextureSlot(6, GetValidUniform(shader_ad_unlit, "sNormalMap"));

    auto shader_distortion =
        m_shaders[static_cast<size_t>(EffekseerRenderer::RendererShaderType::BackDistortion)].get();
    // auto shader_ad_distortion =
    // m_shaders[static_cast<size_t>(EffekseerRenderer::RendererShaderType::AdvancedBackDistortion)].get();

    for (auto& shader : {/*shader_ad_distortion, */ shader_distortion})
    {
        shader->SetVertexConstantBufferSize(sizeof(EffekseerRenderer::StandardRendererVertexBuffer));
        shader->SetPixelConstantBufferSize(sizeof(EffekseerRenderer::PixelConstantBufferDistortion));

        shader->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shader, "mCamera"), 0);

        shader->AddVertexConstantLayout(CONSTANT_TYPE_MATRIX44, GetValidUniform(shader, "mCameraProj"),
                                        sizeof(Effekseer::Matrix44));

        shader->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shader, "mUVInversed"),
                                        sizeof(Effekseer::Matrix44) * 2);

        shader->SetTextureSlot(0, GetValidUniform(shader, "sDiffMap"));
        shader->SetTextureSlot(2, GetValidUniform(shader, "sSpecMap"));

        shader->AddVertexConstantLayout(CONSTANT_TYPE_VECTOR4, GetValidUniform(shader, "fFlipbookParameter"),
                                        sizeof(Effekseer::Matrix44) * 2 + sizeof(float) * 4);

        AssignDistortionPixelConstantBuffer(shader);
    }
    // applyPSAdvancedRendererParameterTexture(shader_ad_distortion, 2);
    shader_distortion->SetTextureSlot(1, GetValidUniform(shader_distortion, "sNormalMap"));
    // shader_ad_distortion_->SetTextureSlot(7, GetValidUniform(shader_ad_distortion, "Sampler_sampler_depthTex"));
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

	Effekseer::ModelRef model;

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

	m_renderer->SetModel(model);

	using namespace EffekseerRenderer;

	EndRendering_<
		RendererImplemented,
		Shader,
		Effekseer::Model,
		false,
		1>(
		m_renderer,
		m_shaders[(size_t)RendererShaderType::AdvancedLit].get(),
		m_shaders[(size_t)RendererShaderType::AdvancedUnlit].get(),
		m_shaders[(size_t)RendererShaderType::AdvancedBackDistortion].get(),
		m_shaders[(size_t)RendererShaderType::Lit].get(),
		m_shaders[(size_t)RendererShaderType::Unlit].get(),
		m_shaders[(size_t)RendererShaderType::BackDistortion].get(),
		parameter, userData);

	m_renderer->SetModel(nullptr);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
