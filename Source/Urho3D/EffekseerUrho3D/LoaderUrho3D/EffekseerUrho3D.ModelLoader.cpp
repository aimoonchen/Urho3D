//#include <ResourceLoader.hpp>
#include "EffekseerUrho3D.ModelLoader.h"
#include "../RendererUrho3D/EffekseerUrho3D.RenderResources.h"
#include "../Utils/EffekseerUrho3D.Utils.h"
#include "../EffekseerResource.h"
#include "../../Core/Context.h"
#include "../../Resource/ResourceCache.h"
#include "../../Cocos2d/Urho3DContext.h"

namespace EffekseerUrho3D
{

Effekseer::ModelRef ModelLoader::Load(const char16_t* path)
{
	static auto* cache = GetUrho3DContext()->GetSubsystem<Urho3D::ResourceCache>();
	Urho3D::String urho3dPath = ToGdString(path);
	auto urhoFile = cache->GetFile(urho3dPath);
	auto dataSize = urhoFile->GetSize();
	auto data = std::make_unique<char[]>(dataSize);
    if (urhoFile->Read(data.get(), dataSize) != dataSize)
        return nullptr;
	// Load by Godot
// 	auto loader = godot::ResourceLoader::get_singleton();
// 	auto resource = loader->load(ToGdString(path), "");
// 	if (!resource.is_valid())
// 	{
// 		return nullptr;
// 	}
// 
// 	auto efkres = godot::as<godot::EffekseerResource>(resource.ptr());
// 	auto& data = efkres->get_data_ref();

 	return Load(data.get(), dataSize);
}

Effekseer::ModelRef ModelLoader::Load(const void* data, int32_t size)
{
	return Effekseer::MakeRefPtr<Model>(data, size);
}

void ModelLoader::Unload(Effekseer::ModelRef data)
{
}

} // namespace EffekseerUrho3D

