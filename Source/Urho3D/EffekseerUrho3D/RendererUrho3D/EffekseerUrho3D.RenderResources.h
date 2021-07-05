#pragma once

#include <stdint.h>
#include <Effekseer.h>
//#include <Resource.hpp>

namespace Urho3D
{
	class Context;
	class Texture2D;
}
namespace EffekseerUrho3D
{
	
class Texture : public Effekseer::Backend::Texture
{
public:
	//godot::RID GetRID() const { return textureRid_; }
    Urho3D::Texture2D* GetUrho3DTexture() const { return urho3d_texture_; }
    bool Texture::Init(Urho3D::Context* context, const Effekseer::Backend::TextureParameter& param);
    bool Texture::Init(Urho3D::Context* context, const Effekseer::Backend::RenderTextureParameter& param);
    bool Texture::Init(Urho3D::Context* context, const Effekseer::Backend::DepthTextureParameter& param);

private:
	friend class TextureLoader;
    bool Texture::InitInternal(Urho3D::Context* context, const Effekseer::Backend::TextureParameter& param);
	Urho3D::Texture2D* urho3d_texture_{ nullptr };
// 	godot::Ref<godot::Resource> godotTexture_;
// 	godot::RID textureRid_;
};

class Model : public Effekseer::Model
{
public:
	Model(const void* data, int32_t size);
	~Model();
	//godot::RID GetRID() const { return meshRid_; }

private:
	friend class ModelLoader;

	//godot::RID meshRid_;
};

}
