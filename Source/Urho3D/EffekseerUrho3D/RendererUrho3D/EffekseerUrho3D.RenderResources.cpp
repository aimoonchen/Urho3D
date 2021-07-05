
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
//#include <VisualServer.hpp>
#include "../Utils/EffekseerUrho3D.Utils.h"
#include "EffekseerUrho3D.RenderResources.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/Texture2D.h"
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{

bool Texture::Init(Urho3D::Context* context, const Effekseer::Backend::TextureParameter& param)
{
    type_ = Effekseer::Backend::TextureType::Color2D;
    return InitInternal(context, param);
}

bool Texture::Init(Urho3D::Context* context, const Effekseer::Backend::RenderTextureParameter& param)
{
    type_ = Effekseer::Backend::TextureType::Render;
    Effekseer::Backend::TextureParameter paramInternal;
    paramInternal.Size = param.Size;
    paramInternal.Format = param.Format;
    paramInternal.GenerateMipmap = false;
    return Init(context, paramInternal);
}

bool Texture::Init(Urho3D::Context* context, const Effekseer::Backend::DepthTextureParameter& param)
{
//     auto format = GetBGFXTextureFormat(param.Format);
//     buffer_ =
//         BGFX(create_texture_2d)(param.Size[0], param.Size[1], false, 1, format,
//                                 BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT_WRITE_ONLY, nullptr);
// 
//     size_ = param.Size;
//     format_ = param.Format;
//     hasMipmap_ = false;
// 
//     type_ = Effekseer::Backend::TextureType::Depth;

    return true;
}

static unsigned GetUrho3DTextureFormat(Effekseer::Backend::TextureFormatType EffekseerFormat)
{
    unsigned format = Urho3D::Graphics::GetRGBAFormat();
    bool isRT = false;
    switch (EffekseerFormat) {
    case Effekseer::Backend::TextureFormatType::R8G8B8A8_UNORM:
        format = Urho3D::Graphics::GetRGBAFormat();
        break;
    case Effekseer::Backend::TextureFormatType::B8G8R8A8_UNORM:
        format = Urho3D::Graphics::GetBGRAFormat();
        break;
    case Effekseer::Backend::TextureFormatType::R8_UNORM:
        format = Urho3D::Graphics::GetLuminanceFormat();
        break;
    case Effekseer::Backend::TextureFormatType::R16G16_FLOAT:
        format = Urho3D::Graphics::GetRG16Format();
        break;
    case Effekseer::Backend::TextureFormatType::R16G16B16A16_FLOAT:
        format = Urho3D::Graphics::GetRGBAFloat16Format();
        break;
    case Effekseer::Backend::TextureFormatType::R32G32B32A32_FLOAT:
        format = Urho3D::Graphics::GetRGBAFloat32Format();
        break;
    case Effekseer::Backend::TextureFormatType::BC1:
        format = Urho3D::Graphics::GetCompressedFormat(Urho3D::CF_DXT1);
        break;
    case Effekseer::Backend::TextureFormatType::BC2:
        format = Urho3D::Graphics::GetCompressedFormat(Urho3D::CF_DXT3);
        break;
    case Effekseer::Backend::TextureFormatType::BC3:
        format = Urho3D::Graphics::GetCompressedFormat(Urho3D::CF_DXT5);
        break;
    case Effekseer::Backend::TextureFormatType::D32:
        format = Urho3D::Graphics::GetD32();
        break;
    case Effekseer::Backend::TextureFormatType::D24S8:
        format = Urho3D::Graphics::GetDepthStencilFormat();
        break;
    default:
        break;
    }
    return format;
}

bool Texture::InitInternal(Urho3D::Context* context, const Effekseer::Backend::TextureParameter& param)
{
//     uint64_t textureFlags = BGFX_SAMPLER_U_MIRROR | BGFX_SAMPLER_V_MIRROR | BGFX_SAMPLER_NONE;
//     if (type_ == Effekseer::Backend::TextureType::Render)
//     {
//         textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT;
//     }
// 
//     urho3d_texture_ = BGFX(create_texture_2d)(param.Size[0], param.Size[1],
//                                       false, // param.GenerateMipmap,
//                                       1, format, textureFlags,
//                                       BGFX(copy)(param.InitialData.data(), param.InitialData.size()));
    auto texture = new Urho3D::Texture2D(context);
    texture->SetSize(1, 1, GetUrho3DTextureFormat(param.Format));
    texture->SetNumLevels(1);
    texture->SetData(0, 0, 0, 1, 1, param.InitialData.data());
    urho3d_texture_ = texture; 
    size_ = param.Size;
    format_ = param.Format;
    hasMipmap_ = param.GenerateMipmap;

    return true;
}

Model::Model(const void* data, int32_t size)
	: Effekseer::Model(data, size)
{
// 	int32_t vertexCount = GetVertexCount();
// 	const Vertex* vertexData = GetVertexes();
// 	int32_t faceCount = GetFaceCount();
// 	const Face* faceData = GetFaces();
// 
// 	godot::PoolVector3Array positions; positions.resize(vertexCount);
// 	godot::PoolVector3Array normals; normals.resize(vertexCount);
// 	godot::PoolRealArray tangents; tangents.resize(vertexCount * 4);
// 	godot::PoolColorArray colors; colors.resize(vertexCount);
// 	godot::PoolVector2Array texUVs; texUVs.resize(vertexCount);
// 	godot::PoolIntArray indeces; indeces.resize(faceCount * 3);
// 
// 	for (int32_t i = 0; i < vertexCount; i++)
// 	{
// 		positions.set(i, ToGdVector3(vertexData[i].Position));
// 		normals.set(i, ToGdVector3(vertexData[i].Normal));
// 		tangents.set(i * 4 + 0, vertexData[i].Tangent.X);
// 		tangents.set(i * 4 + 1, vertexData[i].Tangent.Y);
// 		tangents.set(i * 4 + 2, vertexData[i].Tangent.Z);
// 		tangents.set(i * 4 + 3, 1.0f);
// 		colors.set(i, ToGdColor(vertexData[i].VColor));
// 		texUVs.set(i, ToGdVector2(vertexData[i].UV));
// 	}
// 	for (int32_t i = 0; i < faceCount; i++)
// 	{
// 		indeces.set(i * 3 + 0, faceData[i].Indexes[0]);
// 		indeces.set(i * 3 + 1, faceData[i].Indexes[1]);
// 		indeces.set(i * 3 + 2, faceData[i].Indexes[2]);
// 	}
// 
// 	godot::Array arrays;
// 	arrays.resize(godot::VisualServer::ARRAY_MAX);
// 	arrays[godot::VisualServer::ARRAY_VERTEX] = positions;
// 	arrays[godot::VisualServer::ARRAY_NORMAL] = normals;
// 	arrays[godot::VisualServer::ARRAY_TANGENT] = tangents;
// 	arrays[godot::VisualServer::ARRAY_COLOR] = colors;
// 	arrays[godot::VisualServer::ARRAY_TEX_UV] = texUVs;
// 	arrays[godot::VisualServer::ARRAY_INDEX] = indeces;
// 
// 	auto vs = godot::VisualServer::get_singleton();
// 	meshRid_ = vs->mesh_create();
// 	vs->mesh_add_surface_from_arrays(meshRid_, godot::VisualServer::PRIMITIVE_TRIANGLES, arrays);
}

Model::~Model()
{
// 	if (meshRid_.is_valid())
// 	{
// 		auto vs = godot::VisualServer::get_singleton();
// 		vs->free_rid(meshRid_);
// 	}
}

} // namespace EffekseerUrho3D
