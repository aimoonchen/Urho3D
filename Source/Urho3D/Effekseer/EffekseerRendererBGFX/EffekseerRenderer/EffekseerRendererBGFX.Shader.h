#pragma once
#include "EffekseerRendererBGFX.RendererImplemented.h"

#include "../../EffekseerRendererCommon/EffekseerRenderer.ShaderBase.h"

#include <string>
#include <vector>

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
		bgfx::UniformHandle ID;
		int32_t Offset;
		int32_t Count;
	};

	bgfx::ProgramHandle m_program;

	uint8_t* m_vertexConstantBuffer;
	uint8_t* m_pixelConstantBuffer;

	std::vector<ConstantLayout> m_vertexConstantLayout;
	std::vector<ConstantLayout> m_pixelConstantLayout;

	std::array<bgfx::UniformHandle, Effekseer::TextureSlotMax> m_textureSlots;
	std::array<bool, Effekseer::TextureSlotMax> m_textureSlotEnables;

	bool isTransposeEnabled_ = false;

	Shader(bgfx::ProgramHandle programHandle);
public:
	std::unordered_map<std::string, bgfx::UniformHandle> uniforms_;
	virtual ~Shader();

	static Shader* Create(bgfx::ProgramHandle program);

	bgfx::ProgramHandle GetInterface() const;

	void SetUniforms(std::unordered_map<std::string, bgfx::UniformHandle>&& uniforms);
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

	void AddVertexConstantLayout(eConstantType type, bgfx::UniformHandle id, int32_t offset, int32_t count = 1);
	void AddPixelConstantLayout(eConstantType type, bgfx::UniformHandle id, int32_t offset, int32_t count = 1);

	void SetConstantBuffer() override;

	void SetTextureSlot(int32_t index, bgfx::UniformHandle value);
	bgfx::UniformHandle GetTextureSlot(int32_t index);
	bool GetTextureSlotEnable(int32_t index);

	void SetIsTransposeEnabled(bool isTransposeEnabled)
	{
		isTransposeEnabled_ = isTransposeEnabled;
	}

	bool IsValid() const;
};
} // namespace EffekseerRendererBGFX
