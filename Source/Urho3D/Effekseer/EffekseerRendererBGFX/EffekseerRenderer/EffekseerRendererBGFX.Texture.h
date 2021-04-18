#pragma once
//#include "Effekseer.h"
#include "EffekseerRendererBGFX.RendererImplemented.h"

#include <string>
#include <vector>

namespace EffekseerRendererBGFX {
	namespace Backend {
		class Texture;
		using TextureRef = Effekseer::RefPtr<Texture>;
		class Texture : public Effekseer::Backend::Texture
		{
		private:
			bgfx::TextureHandle buffer_{ BGFX_INVALID_HANDLE };
			//GraphicsDevice* graphicsDevice_ = nullptr;
			std::function<void()> onDisposed_;
			bool InitInternal(const Effekseer::Backend::TextureParameter& param);
		public:
			Texture(/*GraphicsDevice* graphicsDevice*/);
			~Texture() override;
			bool Init(const Effekseer::Backend::TextureParameter& param);
			bool Init(const Effekseer::Backend::RenderTextureParameter& param);
			bool Init(const Effekseer::Backend::DepthTextureParameter& param);
			bool Init(bgfx::TextureHandle buffer, bool hasMipmap, const std::function<void()>& onDisposed);
			bgfx::TextureHandle GetBuffer() const { return buffer_; }
		};
	}
} // namespace EffekseerRendererBGFX
