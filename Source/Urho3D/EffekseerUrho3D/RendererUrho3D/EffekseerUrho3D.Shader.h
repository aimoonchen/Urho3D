#pragma once
#include "../../Math/StringHash.h"
//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "../../Effekseer/EffekseerRendererCommon/EffekseerRenderer.ShaderBase.h"
#include "EffekseerUrho3D.RendererImplemented.h"

namespace Urho3D
{
	class Graphics;
	class ShaderVariation;
	class ShaderProgram;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{
enum eConstantType
{
    CONSTANT_TYPE_MATRIX44 = 0,
    CONSTANT_TYPE_VECTOR4 = 100,
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class Shader : public ::EffekseerRenderer::ShaderBase
{
public:
	enum class RenderType : uint8_t
	{
		SpatialLightweight,
		SpatialDepthFade,
		CanvasItem,
		Max
	};

	enum class ParamType : uint8_t
	{
		Int,
		Float,
		Vector2,
		Vector3,
		Vector4,
		Matrix44,
		Color,
		Texture,
	};

	struct ParamDecl
	{
		char name[24];
		ParamType type;
		uint16_t slot;
		uint16_t offset;
	};

	virtual ~Shader();

	static std::unique_ptr<Shader> Create(Urho3D::Graphics* graphics, const char* name,
                                          EffekseerRenderer::RendererShaderType shaderType);

	template <size_t N>
	bool Compile(RenderType renderType, const char* code, const ParamDecl (&paramDecls)[N])
	{
		std::vector<ParamDecl> v(N);
		v.assign(paramDecls, paramDecls + N);
		return Compile(renderType, code, std::move(v));
	}

	bool Compile(RenderType renderType, const char* code, std::vector<ParamDecl>&& paramDecls);

	void SetVertexConstantBufferSize(int32_t size)
	{
		m_constantBuffers[0].resize((size_t)size);
	}
	void SetPixelConstantBufferSize(int32_t size)
	{
		m_constantBuffers[1].resize((size_t)size);
	}
	int32_t GetVertexConstantBufferSize() const
	{
		return (int32_t)m_constantBuffers[0].size();
	}
	int32_t GetPixelConstantBufferSize() const
	{
		return (int32_t)m_constantBuffers[1].size();
	}

	void* GetVertexConstantBuffer()
	{
		return m_constantBuffers[0].data();
	}
	void* GetPixelConstantBuffer()
	{
		return m_constantBuffers[1].data();
	}

	void SetConstantBuffer() override;

	bool HasUniform(Urho3D::StringHash name);
	void SetTextureSlot(int32_t index, Urho3D::StringHash /*bgfx::UniformHandle*/ value);
    /*bgfx::UniformHandle*/ Urho3D::StringHash GetTextureSlot(int32_t index);
    bool GetTextureSlotEnable(int32_t index);
    bool IsValid() const ;
	//void ApplyToMaterial(RenderType renderType, godot::RID material, EffekseerRenderer::RenderStateBase::State& state);

	EffekseerRenderer::RendererShaderType GetShaderType() { return m_shaderType; }

	void AddVertexConstantLayout(eConstantType type, Urho3D::StringHash /*bgfx::UniformHandle*/ id, int32_t offset,
                                 int32_t count = 1);
    void AddPixelConstantLayout(eConstantType type, Urho3D::StringHash /*bgfx::UniformHandle*/ id, int32_t offset,
                                int32_t count = 1);
    void BeginScene();
    void EndScene();

private:
	std::vector<uint8_t> m_constantBuffers[2];

	std::string m_name;
	EffekseerRenderer::RendererShaderType m_shaderType = EffekseerRenderer::RendererShaderType::Unlit;

	struct InternalShader {
		//godot::RID rid[2][2][3][5];
		std::vector<ParamDecl> paramDecls;
	};
	InternalShader m_internals[(size_t)RenderType::Max];

	Shader(Urho3D::Graphics* graphics, const char* name, EffekseerRenderer::RendererShaderType shaderType);

	// Urho3D impl
	Urho3D::Graphics* graphics_{ nullptr };
	Urho3D::ShaderProgram* m_program{ nullptr };
	Urho3D::ShaderVariation* m_vs{ nullptr };
    Urho3D::ShaderVariation* m_fs{ nullptr };
    struct ConstantLayout
    {
        eConstantType Type;
        // bgfx::UniformHandle ID;
        Urho3D::StringHash ID;
        int32_t Offset;
        int32_t Count;
    };
    std::vector<ConstantLayout> m_vertexConstantLayout;
    std::vector<ConstantLayout> m_pixelConstantLayout;
    std::array<Urho3D::StringHash, Effekseer::TextureSlotMax> m_textureSlots;
    std::array<bool, Effekseer::TextureSlotMax> m_textureSlotEnables;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
