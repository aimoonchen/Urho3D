#pragma once
#include "EffekseerRendererBGFX.RendererImplemented.h"

#include "../../EffekseerRendererCommon/EffekseerRenderer.ShaderBase.h"

#include "../../../Math/StringHash.h"

#include <string>
#include <vector>

namespace Urho3D
{
	class ShaderProgram;
	class ShaderVariation;
}

namespace EffekseerRendererBGFX {

enum eConstantType
{
	CONSTANT_TYPE_MATRIX44 = 0,
	CONSTANT_TYPE_VECTOR4 = 100,
};

struct ShaderCodeView
{
	const char* Data;
	int32_t Length;

	ShaderCodeView()
		: Data(nullptr)
		, Length(0)
	{
	}

	ShaderCodeView(const char* data)
		: Data(data)
		, Length(static_cast<int32_t>(strlen(data)))
	{
	}
};

class Shader : public ::EffekseerRenderer::ShaderBase
{
private:
	struct ConstantLayout
	{
		eConstantType Type;
		//bgfx::UniformHandle ID;
		Urho3D::StringHash ID;
		int32_t Offset;
		int32_t Count;
	};
	Urho3D::Graphics* graphics_{ nullptr };
	//bgfx::ProgramHandle m_program;
	
	Urho3D::ShaderProgram* m_program;
	uint8_t* m_vertexConstantBuffer;
	uint8_t* m_pixelConstantBuffer;

	std::vector<ConstantLayout> m_vertexConstantLayout;
	std::vector<ConstantLayout> m_pixelConstantLayout;

	std::array<Urho3D::StringHash/*bgfx::UniformHandle*/, Effekseer::TextureSlotMax> m_textureSlots;
	std::array<bool, Effekseer::TextureSlotMax> m_textureSlotEnables;

	bool isTransposeEnabled_ = false;
	
	Shader(Urho3D::ShaderProgram* program);
public:
    Urho3D::ShaderVariation* m_vs;
    Urho3D::ShaderVariation* m_fs;
	std::unordered_map<std::string, Urho3D::StringHash/*bgfx::UniformHandle*/> uniforms_;
	virtual ~Shader();

	static Shader* Create(const char* filename);

	bool HasUniform(Urho3D::StringHash name);
	/*bgfx::ProgramHandle*/Urho3D::ShaderProgram* GetInterface() const;

	void SetUniforms(std::unordered_map<std::string, Urho3D::StringHash/*bgfx::UniformHandle*/>&& uniforms);
	void BeginScene();
	void EndScene();

	void SetVertexConstantBufferSize(int32_t size) override;
	void SetPixelConstantBufferSize(int32_t size) override;

	void* GetVertexConstantBuffer() override
	{
		return m_vertexConstantBuffer;
	}
	void* GetPixelConstantBuffer() override
	{
		return m_pixelConstantBuffer;
	}

	void AddVertexConstantLayout(eConstantType type, Urho3D::StringHash/*bgfx::UniformHandle*/ id, int32_t offset, int32_t count = 1);
	void AddPixelConstantLayout(eConstantType type, Urho3D::StringHash/*bgfx::UniformHandle*/ id, int32_t offset, int32_t count = 1);

	void SetConstantBuffer() override;

	void SetTextureSlot(int32_t index, Urho3D::StringHash/*bgfx::UniformHandle*/ value);
	/*bgfx::UniformHandle*/Urho3D::StringHash GetTextureSlot(int32_t index);
	bool GetTextureSlotEnable(int32_t index);

	void SetIsTransposeEnabled(bool isTransposeEnabled)
	{
		isTransposeEnabled_ = isTransposeEnabled;
	}

	bool IsValid() const;
};
} // namespace EffekseerRendererBGFX
