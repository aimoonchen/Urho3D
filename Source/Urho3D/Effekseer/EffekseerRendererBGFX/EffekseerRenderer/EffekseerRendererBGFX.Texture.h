#pragma once
//#include "Effekseer.h"
#include "EffekseerRendererBGFX.RendererImplemented.h"

#include <string>
#include <vector>

namespace Urho3D
{
	class Texture2D;
}

namespace EffekseerRendererBGFX {
	namespace Backend {
		class Texture;
		using TextureRef = Effekseer::RefPtr<Texture>;
		class Texture : public Effekseer::Backend::Texture
		{
		private:
			//bgfx::TextureHandle buffer_{ BGFX_INVALID_HANDLE };
			Urho3D::Texture2D* buffer_{ nullptr };
			//GraphicsDevice* graphicsDevice_ = nullptr;
			std::function<void()> onDisposed_;
			bool InitInternal(const Effekseer::Backend::TextureParameter& param);
		public:
			Texture(/*GraphicsDevice* graphicsDevice*/);
			~Texture() override;
			bool Init(const Effekseer::Backend::TextureParameter& param);
			bool Init(const Effekseer::Backend::RenderTextureParameter& param);
			bool Init(const Effekseer::Backend::DepthTextureParameter& param);
			bool Init(Urho3D::Texture2D* buffer, bool hasMipmap, const std::function<void()>& onDisposed);
			Urho3D::Texture2D* GetBuffer() const { return buffer_; }
		};
	}
} // namespace EffekseerRendererBGFX
