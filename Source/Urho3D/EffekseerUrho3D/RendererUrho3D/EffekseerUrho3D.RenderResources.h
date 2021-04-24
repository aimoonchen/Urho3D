#pragma once

#include <stdint.h>
#include <Effekseer.h>
//#include <Resource.hpp>

namespace Urho3D
{
	class Texture2D;
}
namespace EffekseerUrho3D
{
	
class Texture : public Effekseer::Backend::Texture
{
public:
	//godot::RID GetRID() const { return textureRid_; }
    Urho3D::Texture2D* GetUrho3DTexture() const { return urho3d_texture_; }

private:
	friend class TextureLoader;

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
