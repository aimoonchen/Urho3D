// #include <VisualServer.hpp>
// #include <Texture.hpp>
#include "../../Graphics/Graphics.h"
#include "../../Graphics/ShaderProgram.h"
#include "EffekseerUrho3D.Shader.h"
#include "../Utils/EffekseerUrho3D.Utils.h"

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{

static const char* ShaderType3D = 
	"shader_type spatial;\n";

static const char* ShaderType2D = 
	"shader_type canvas_item;\n";

static const char* BlendMode[] = {
	"",
	"render_mode blend_mix;\n",
	"render_mode blend_add;\n",
	"render_mode blend_sub;\n",
	"render_mode blend_mul;\n",
};
static const char* CullMode[] = {
	"render_mode cull_back;\n",
	"render_mode cull_front;\n",
	"render_mode cull_disabled;\n",
};
static const char* DepthTestMode[] = {
	"render_mode depth_test_disable;\n",
	"",
};
static const char* DepthWriteMode[] = {
	"render_mode depth_draw_never;\n",
	"render_mode depth_draw_always;\n",
};

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
std::unique_ptr<Shader> Shader::Create(Urho3D::Graphics* graphics, const char* name,
                                       EffekseerRenderer::RendererShaderType shaderType)
{
    return std::unique_ptr<Shader>(new Shader(graphics, name, shaderType));
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Shader::Shader(Urho3D::Graphics* graphics, const char* name, EffekseerRenderer::RendererShaderType shaderType)
{
	m_name = name;
	m_shaderType = shaderType;
	//
    graphics_ = graphics;
    m_vs = graphics_->GetShader(Urho3D::VS, name, "");
	m_fs = graphics_->GetShader(Urho3D::PS, name, "");
	graphics_->SetShaders(m_vs, m_fs);
	m_program = graphics_->GetShaderProgram();
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Shader::~Shader()
{
// 	auto vs = godot::VisualServer::get_singleton();
// 
// #define COUNT_OF(list) (sizeof(list) / sizeof(list[0]))
// 	for (int i = 0; i < (int)RenderType::Max; i++)
// 	{
// 		if ((RenderType)i == RenderType::CanvasItem)
// 		{
// 			for (size_t bm = 0; bm < COUNT_OF(BlendMode); bm++)
// 			{
// 				vs->free_rid(m_internals[i].rid[0][0][0][bm]);
// 			}
// 		}
// 		else
// 		{
// 			for (size_t dwm = 0; dwm < COUNT_OF(DepthWriteMode); dwm++)
// 			{
// 				for (size_t dtm = 0; dtm < COUNT_OF(DepthTestMode); dtm++)
// 				{
// 					for (size_t cm = 0; cm < COUNT_OF(CullMode); cm++)
// 					{
// 						for (size_t bm = 0; bm < COUNT_OF(BlendMode); bm++)
// 						{
// 							vs->free_rid(m_internals[i].rid[dwm][dtm][cm][bm]);
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// #undef COUNT_OF
}

bool Shader::Compile(RenderType renderType, const char* code, std::vector<ParamDecl>&& paramDecls)
{
// 	auto vs = godot::VisualServer::get_singleton();
// 
// 	auto& shader = m_internals[(int)renderType];
// 	shader.paramDecls = std::move(paramDecls);
// 
// 	Urho3D::String baseCode = code;
// 
// #define COUNT_OF(list) (sizeof(list) / sizeof(list[0]))
// 	if (renderType == RenderType::CanvasItem)
// 	{
// 		for (size_t bm = 0; bm < COUNT_OF(BlendMode); bm++)
// 		{
// 			Urho3D::String fullCode;
// 			fullCode += ShaderType2D;
// 			fullCode += BlendMode[bm];
// 			fullCode += baseCode;
// 
// 			shader.rid[0][0][0][bm] = vs->shader_create();
// 			vs->shader_set_code(shader.rid[0][0][0][bm], fullCode);
// 		}
// 	}
// 	else
// 	{
// 		for (size_t dwm = 0; dwm < COUNT_OF(DepthWriteMode); dwm++)
// 		{
// 			for (size_t dtm = 0; dtm < COUNT_OF(DepthTestMode); dtm++)
// 			{
// 				for (size_t cm = 0; cm < COUNT_OF(CullMode); cm++)
// 				{
// 					for (size_t bm = 0; bm < COUNT_OF(BlendMode); bm++)
// 					{
// 						Urho3D::String fullCode;
// 						fullCode += ShaderType3D;
// 						fullCode += DepthWriteMode[dwm];
// 						fullCode += DepthTestMode[dtm];
// 						fullCode += CullMode[cm];
// 						fullCode += BlendMode[bm];
// 						fullCode += baseCode;
// 
// 						shader.rid[dwm][dtm][cm][bm] = vs->shader_create();
// 						vs->shader_set_code(shader.rid[dwm][dtm][cm][bm], fullCode);
// 					}
// 				}
// 			}
// 		}
// 	}
// #undef COUNT_OF
 	return true;
}

void Shader::AddVertexConstantLayout(eConstantType type, Urho3D::StringHash /*bgfx::UniformHandle*/ id, int32_t offset,
                                     int32_t count)
{
    ConstantLayout l;
    l.Type = type;
    l.ID = id;
    l.Offset = offset;
    l.Count = count;
    m_vertexConstantLayout.push_back(l);
}

void Shader::AddPixelConstantLayout(eConstantType type, Urho3D::StringHash /*bgfx::UniformHandle*/ id, int32_t offset,
                                    int32_t count)
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
    for (size_t i = 0; i < m_vertexConstantLayout.size(); i++)
    {
        if (m_vertexConstantLayout[i].Type == CONSTANT_TYPE_MATRIX44)
        {
            uint8_t* data = m_constantBuffers[0].data();
            data += m_vertexConstantLayout[i].Offset;
            // GLExt::glUniformMatrix4fv(m_vertexConstantLayout[i].ID, m_vertexConstantLayout[i].Count,
            // isTransposeEnabled_ ? GL_TRUE : GL_FALSE, (const GLfloat*)data);
            //::Effekseer::Matrix44 mat;
            // memcpy(mat.Values, data, sizeof(float) * 16);
            // mat.Transpose();
            // 			if (bgfx::isValid(m_vertexConstantLayout[i].ID)) {
            // 				bgfx::setUniform(m_vertexConstantLayout[i].ID, data, m_vertexConstantLayout[i].Count);
            // 			}
            auto count = m_vertexConstantLayout[i].Count;
            if (count > 1) {
                graphics_->SetShaderParameter(m_vertexConstantLayout[i].ID, (float*)data, count);
            } else {
                graphics_->SetShaderParameter(m_vertexConstantLayout[i].ID, *(Urho3D::Matrix4*)data);
            }
        }

        else if (m_vertexConstantLayout[i].Type == CONSTANT_TYPE_VECTOR4)
        {
            uint8_t* data = m_constantBuffers[0].data();
            data += m_vertexConstantLayout[i].Offset;
            // GLExt::glUniform4fv(m_vertexConstantLayout[i].ID, m_vertexConstantLayout[i].Count, (const GLfloat*)data);
            // 			if (bgfx::isValid(m_vertexConstantLayout[i].ID)) {
            // 				bgfx::setUniform(m_vertexConstantLayout[i].ID, data, m_vertexConstantLayout[i].Count);
            // 			}
            auto count = m_vertexConstantLayout[i].Count;
            if (count > 1) {
                graphics_->SetShaderParameter(m_vertexConstantLayout[i].ID, (float*)data, count);
            } else {
                graphics_->SetShaderParameter(m_vertexConstantLayout[i].ID, *(Urho3D::Vector4*)data);
            }
        }
    }

    for (size_t i = 0; i < m_pixelConstantLayout.size(); i++)
    {
        if (m_pixelConstantLayout[i].Type == CONSTANT_TYPE_MATRIX44)
        {
            uint8_t* data = m_constantBuffers[1].data();
            data += m_pixelConstantLayout[i].Offset;
            // GLExt::glUniformMatrix4fv(m_pixelConstantLayout[i].ID, m_pixelConstantLayout[i].Count,
            // isTransposeEnabled_ ? GL_TRUE : GL_FALSE, (const GLfloat*)data);
            //::Effekseer::Matrix44 mat;
            // memcpy(mat.Values, data, sizeof(float) * 16);
            // mat.Transpose();
            // 			if (bgfx::isValid(m_pixelConstantLayout[i].ID)) {
            // 				bgfx::setUniform(m_pixelConstantLayout[i].ID, data, m_pixelConstantLayout[i].Count);
            // 			}
            graphics_->SetShaderParameter(m_pixelConstantLayout[i].ID, *(Urho3D::Matrix4*)data);
        }
        else if (m_pixelConstantLayout[i].Type == CONSTANT_TYPE_VECTOR4)
        {
            uint8_t* data = m_constantBuffers[1].data();
            data += m_pixelConstantLayout[i].Offset;
            // GLExt::glUniform4fv(m_pixelConstantLayout[i].ID, m_pixelConstantLayout[i].Count, (const GLfloat*)data);
            // 			if (bgfx::isValid(m_pixelConstantLayout[i].ID)) {
            // 				bgfx::setUniform(m_pixelConstantLayout[i].ID, data, m_pixelConstantLayout[i].Count);
            // 			}
            graphics_->SetShaderParameter(m_pixelConstantLayout[i].ID, *(Urho3D::Vector4*)data);
        }
    }
}

void Shader::BeginScene()
{
    // GLExt::glUseProgram(m_program);
    graphics_->SetShaders(m_vs, m_fs);
}

void Shader::EndScene()
{
    // bgfx::submit(0, m_program);
}

Urho3D::StringHash Shader::GetUniformId(const char* name)
{
    auto uniform_name = Urho3D::StringHash(name);
    if (HasUniform(uniform_name)) {
        return uniform_name;
    } else {
        return "";
    }
}

bool Shader::HasUniform(Urho3D::StringHash name) { return m_program->GetUniform(name) != UINT16_MAX; }

void Shader::SetTextureSlot(int32_t index, Urho3D::StringHash /*bgfx::UniformHandle*/ value)
{
    // 	if (bgfx::isValid(value))
    // 	{
    m_textureSlots[index] = value;
    m_textureSlotEnables[index] = true;
    //	}
}

Urho3D::StringHash Shader::GetTextureSlot(int32_t index) { return m_textureSlots[index]; }

bool Shader::GetTextureSlotEnable(int32_t index) { return m_textureSlotEnables[index]; }

bool Shader::IsValid() const { return m_program->IsValid(); }

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
// void Shader::ApplyToMaterial(RenderType renderType, godot::RID material, EffekseerRenderer::RenderStateBase::State& state)
// {
// 	auto vs = godot::VisualServer::get_singleton();
// 
// 	auto& shader = m_internals[(int)renderType];
// 	const size_t bm = (size_t)state.AlphaBlend;
// 	
// 	if (renderType == RenderType::CanvasItem)
// 	{
// 		vs->material_set_shader(material, shader.rid[0][0][0][bm]);
// 	}
// 	else
// 	{
// 		const size_t cm = (size_t)state.CullingType;
// 		const size_t dtm = (size_t)state.DepthTest;
// 		const size_t dwm = (size_t)state.DepthWrite;
// 		vs->material_set_shader(material, shader.rid[dwm][dtm][cm][bm]);
// 	}
// 
// 	for (size_t i = 0; i < shader.paramDecls.size(); i++)
// 	{
// 		const auto& decl = shader.paramDecls[i];
// 
// 		if (decl.type == ParamType::Int)
// 		{
// 			auto value = *(const int32_t*)&m_constantBuffers[decl.slot][decl.offset];
// 			vs->material_set_param(material, decl.name, value);
// 		}
// 		else if (decl.type == ParamType::Float)
// 		{
// 			auto value = *(const float*)&m_constantBuffers[decl.slot][decl.offset];
// 			vs->material_set_param(material, decl.name, value);
// 		}
// 		else if (decl.type == ParamType::Vector2)
// 		{
// 			auto& vector = *(const Urho3D::Vector2*)&m_constantBuffers[decl.slot][decl.offset];
// 			vs->material_set_param(material, decl.name, vector);
// 		}
// 		else if (decl.type == ParamType::Vector3)
// 		{
// 			auto& vector = *(const Urho3D::Vector3*)&m_constantBuffers[decl.slot][decl.offset];
// 			vs->material_set_param(material, decl.name, vector);
// 		}
// 		else if (decl.type == ParamType::Vector4)
// 		{
// 			auto& vector = *(const Urho3D::Quaternion*)&m_constantBuffers[decl.slot][decl.offset];
// 			//auto& vector = *(const Urho3D::Color*)&m_constantBuffers[decl.slot][decl.offset];
// 			vs->material_set_param(material, decl.name, vector);
// 		}
// 		else if (decl.type == ParamType::Color)
// 		{
// 			auto& vector = *(const Urho3D::Color*)&m_constantBuffers[decl.slot][decl.offset];
// 			vs->material_set_param(material, decl.name, vector);
// 		}
// 		else if (decl.type == ParamType::Matrix44)
// 		{
// 			auto& matrix = *(const Effekseer::Matrix44*)&m_constantBuffers[decl.slot][decl.offset];
// 			vs->material_set_param(material, decl.name, ToGdMatrix(matrix));
// 		}
// 		else if (decl.type == ParamType::Texture)
// 		{
// 			godot::RID texture = Int64ToRID((int64_t)state.TextureIDs[decl.slot]);
// 			if (texture.is_valid())
// 			{
// 				vs->texture_set_flags(texture, godot::Texture::FLAG_MIPMAPS | 
// 					((state.TextureFilterTypes[decl.slot] == Effekseer::TextureFilterType::Linear) ? godot::Texture::FLAG_FILTER : 0) | 
// 					((state.TextureWrapTypes[decl.slot] == Effekseer::TextureWrapType::Repeat) ? godot::Texture::FLAG_REPEAT : 0));
// 				vs->material_set_param(material, decl.name, texture);
// 			}
// 		}
// 	}
// }

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
