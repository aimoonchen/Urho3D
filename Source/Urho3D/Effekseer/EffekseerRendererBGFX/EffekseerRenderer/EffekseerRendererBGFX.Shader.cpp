//#include "bgfx_utils.h"
#include "EffekseerRendererBGFX.Shader.h"
#include "EffekseerRendererBGFX.Renderer.h"
#include "../../../Cocos2d/Urho3DContext.h"
#include "../../../Core/Context.h"
#include "../../../Graphics/Graphics.h"
#include "../../../Graphics/ShaderProgram.h"

namespace EffekseerRendererBGFX {

Shader::Shader(/*bgfx::ProgramHandle*/Urho3D::ShaderProgram* programHandle)
	: m_vertexConstantBuffer(nullptr)
	, m_pixelConstantBuffer(nullptr)
	, m_program{ programHandle }
{
	//m_textureSlots.fill(BGFX_INVALID_HANDLE/*0*/);
	m_textureSlotEnables.fill(false);
	graphics_ = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>();
	//baseInstance_ = GLExt::glGetUniformLocation(m_program, "SPIRV_Cross_BaseInstance");
}

Shader::~Shader()
{
	ES_SAFE_DELETE_ARRAY(m_vertexConstantBuffer);
	ES_SAFE_DELETE_ARRAY(m_pixelConstantBuffer);
}

Shader* Shader::Create(/*bgfx::ProgramHandle*/Urho3D::ShaderProgram* program)
{
	if (program->IsValid()) {
		return new Shader(program);
	} else {
		return nullptr;
	}
}

void Shader::SetUniforms(std::unordered_map<std::string, Urho3D::StringHash/*bgfx::UniformHandle*/>&& uniforms)
{
	uniforms_ = std::move(uniforms);
}

/*bgfx::ProgramHandle*/Urho3D::ShaderProgram* Shader::GetInterface() const
{
	return m_program;
}

void Shader::BeginScene()
{
	//GLExt::glUseProgram(m_program);
}

void Shader::EndScene()
{
	bgfx::submit(0, m_program);
}

void Shader::SetVertexConstantBufferSize(int32_t size)
{
	ES_SAFE_DELETE_ARRAY(m_vertexConstantBuffer);
	m_vertexConstantBuffer = new uint8_t[size];
}

void Shader::SetPixelConstantBufferSize(int32_t size)
{
	ES_SAFE_DELETE_ARRAY(m_pixelConstantBuffer);
	m_pixelConstantBuffer = new uint8_t[size];
}

void Shader::AddVertexConstantLayout(eConstantType type, Urho3D::StringHash/*bgfx::UniformHandle*/ id, int32_t offset, int32_t count)
{
	ConstantLayout l;
	l.Type = type;
	l.ID = id;
	l.Offset = offset;
	l.Count = count;
	m_vertexConstantLayout.push_back(l);
}

void Shader::AddPixelConstantLayout(eConstantType type, Urho3D::StringHash/*bgfx::UniformHandle*/ id, int32_t offset, int32_t count)
{
	ConstantLayout l;
	l.Type = type;
	l.ID = id;
	l.Offset = offset;
	l.Count = count;
	m_pixelConstantLayout.push_back(l);
}

void Shader::SetConstantBuffer()
{
	// baseInstance_
	//if (baseInstance_ >= 0)
	//{
	//	GLExt::glUniform1i(baseInstance_, 0);
	//}

	for (size_t i = 0; i < m_vertexConstantLayout.size(); i++)
	{
		if (m_vertexConstantLayout[i].Type == CONSTANT_TYPE_MATRIX44)
		{
			uint8_t* data = (uint8_t*)m_vertexConstantBuffer;
			data += m_vertexConstantLayout[i].Offset;
			//GLExt::glUniformMatrix4fv(m_vertexConstantLayout[i].ID, m_vertexConstantLayout[i].Count, isTransposeEnabled_ ? GL_TRUE : GL_FALSE, (const GLfloat*)data);
			//::Effekseer::Matrix44 mat;
			//memcpy(mat.Values, data, sizeof(float) * 16);
			//mat.Transpose();
// 			if (bgfx::isValid(m_vertexConstantLayout[i].ID)) {
// 				bgfx::setUniform(m_vertexConstantLayout[i].ID, data, m_vertexConstantLayout[i].Count);
// 			}
			graphics_->SetShaderParameter(m_vertexConstantLayout[i].ID, *(Urho3D::Matrix4*)data);
		}

		else if (m_vertexConstantLayout[i].Type == CONSTANT_TYPE_VECTOR4)
		{
			uint8_t* data = (uint8_t*)m_vertexConstantBuffer;
			data += m_vertexConstantLayout[i].Offset;
			//GLExt::glUniform4fv(m_vertexConstantLayout[i].ID, m_vertexConstantLayout[i].Count, (const GLfloat*)data);
// 			if (bgfx::isValid(m_vertexConstantLayout[i].ID)) {
// 				bgfx::setUniform(m_vertexConstantLayout[i].ID, data, m_vertexConstantLayout[i].Count);
// 			}
			graphics_->SetShaderParameter(m_vertexConstantLayout[i].ID, *(Urho3D::Vector4*)data);
		}
	}

	for (size_t i = 0; i < m_pixelConstantLayout.size(); i++)
	{
		if (m_pixelConstantLayout[i].Type == CONSTANT_TYPE_MATRIX44)
		{
			uint8_t* data = (uint8_t*)m_pixelConstantBuffer;
			data += m_pixelConstantLayout[i].Offset;
			//GLExt::glUniformMatrix4fv(m_pixelConstantLayout[i].ID, m_pixelConstantLayout[i].Count, isTransposeEnabled_ ? GL_TRUE : GL_FALSE, (const GLfloat*)data);
			//::Effekseer::Matrix44 mat;
			//memcpy(mat.Values, data, sizeof(float) * 16);
			//mat.Transpose();
// 			if (bgfx::isValid(m_pixelConstantLayout[i].ID)) {
// 				bgfx::setUniform(m_pixelConstantLayout[i].ID, data, m_pixelConstantLayout[i].Count);
// 			}
			graphics_->SetShaderParameter(m_pixelConstantLayout[i].ID, *(Urho3D::Matrix4*)data);
		}

		else if (m_pixelConstantLayout[i].Type == CONSTANT_TYPE_VECTOR4)
		{
			uint8_t* data = (uint8_t*)m_pixelConstantBuffer;
			data += m_pixelConstantLayout[i].Offset;
			//GLExt::glUniform4fv(m_pixelConstantLayout[i].ID, m_pixelConstantLayout[i].Count, (const GLfloat*)data);
// 			if (bgfx::isValid(m_pixelConstantLayout[i].ID)) {
// 				bgfx::setUniform(m_pixelConstantLayout[i].ID, data, m_pixelConstantLayout[i].Count);
// 			}
			graphics_->SetShaderParameter(m_pixelConstantLayout[i].ID, *(Urho3D::Vector4*)data);
		}
	}

	//GLCheckError();
}

void Shader::SetTextureSlot(int32_t index, Urho3D::StringHash/*bgfx::UniformHandle*/ value)
{
// 	if (bgfx::isValid(value))
// 	{
		m_textureSlots[index] = value;
		m_textureSlotEnables[index] = true;
//	}
}

/*bgfx::UniformHandle*/Urho3D::StringHash Shader::GetTextureSlot(int32_t index)
{
	return m_textureSlots[index];
}

bool Shader::GetTextureSlotEnable(int32_t index)
{
	return m_textureSlotEnables[index];
}

bool Shader::IsValid() const
{
	return m_program->IsValid();
}

} // namespace EffekseerRendererBGFX
