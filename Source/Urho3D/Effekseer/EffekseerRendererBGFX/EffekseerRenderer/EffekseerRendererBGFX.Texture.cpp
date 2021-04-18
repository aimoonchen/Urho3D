//#include "bgfx_utils.h"
#include "EffekseerRendererBGFX.Texture.h"
#include "EffekseerRendererBGFX.Renderer.h"

namespace EffekseerRendererBGFX {
	namespace Backend {
		static bgfx::TextureFormat::Enum GetBGFXTextureFormat(Effekseer::Backend::TextureFormatType EffekseerFormat)
		{
			bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Unknown;
			bool isRT = false;
			switch (EffekseerFormat) {
			case Effekseer::Backend::TextureFormatType::R8G8B8A8_UNORM:
				format = bgfx::TextureFormat::RGBA8;
				break;
			case Effekseer::Backend::TextureFormatType::B8G8R8A8_UNORM:
				format = bgfx::TextureFormat::BGRA8;
				break;
			case Effekseer::Backend::TextureFormatType::R8_UNORM:
				format = bgfx::TextureFormat::R8;
				break;
			case Effekseer::Backend::TextureFormatType::R16G16_FLOAT:
				format = bgfx::TextureFormat::RG16F;
				break;
			case Effekseer::Backend::TextureFormatType::R16G16B16A16_FLOAT:
				format = bgfx::TextureFormat::RGBA16F;
				break;
			case Effekseer::Backend::TextureFormatType::R32G32B32A32_FLOAT:
				format = bgfx::TextureFormat::RGBA32F;
				break;
			case Effekseer::Backend::TextureFormatType::BC1:
				format = bgfx::TextureFormat::BC1;
				break;
			case Effekseer::Backend::TextureFormatType::BC2:
				format = bgfx::TextureFormat::BC2;
				break;
			case Effekseer::Backend::TextureFormatType::BC3:
				format = bgfx::TextureFormat::BC3;
				break;
			case Effekseer::Backend::TextureFormatType::D32:
				format = bgfx::TextureFormat::D32;
				break;
			case Effekseer::Backend::TextureFormatType::D24S8:
				format = bgfx::TextureFormat::D24S8;
				break;
			default:
				break;
			}
			return format;
		}
		Texture::Texture(/*GraphicsDevice* graphicsDevice*/)
			//	: graphicsDevice_(graphicsDevice)
		{
			//	ES_SAFE_ADDREF(graphicsDevice_);
		}

		Texture::~Texture()
		{
			if (onDisposed_)
			{
				onDisposed_();
			}
			else
			{
				//if (buffer_ > 0)
				//{
				//	glDeleteTextures(1, &buffer_);
				//	buffer_ = 0;
				//}
				bgfx::destroy(buffer_);
			}

			//ES_SAFE_RELEASE(graphicsDevice_);
		}

		bool Texture::InitInternal(const Effekseer::Backend::TextureParameter& param)
		{
			auto format = GetBGFXTextureFormat(param.Format);
			uint64_t textureFlags = BGFX_SAMPLER_U_MIRROR | BGFX_SAMPLER_V_MIRROR | BGFX_SAMPLER_NONE;
			if (type_ == Effekseer::Backend::TextureType::Render)
			{
				textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT;
			}

			buffer_ = bgfx::createTexture2D(
				param.Size[0],
				param.Size[1],
				false,// param.GenerateMipmap,
				1,
				format,
				textureFlags,
				bgfx::copy(param.InitialData.data(), param.InitialData.size())
				);
			size_ = param.Size;
			format_ = param.Format;
			hasMipmap_ = param.GenerateMipmap;

			return true;
		}

		bool Texture::Init(const Effekseer::Backend::TextureParameter& param)
		{
			type_ = Effekseer::Backend::TextureType::Color2D;
			return InitInternal(param);
		}

		bool Texture::Init(const Effekseer::Backend::RenderTextureParameter& param)
		{
			type_ = Effekseer::Backend::TextureType::Render;
			Effekseer::Backend::TextureParameter paramInternal;
			paramInternal.Size = param.Size;
			paramInternal.Format = param.Format;
			paramInternal.GenerateMipmap = false;
			return Init(paramInternal);
		}

		bool Texture::Init(const Effekseer::Backend::DepthTextureParameter& param)
		{
			auto format = GetBGFXTextureFormat(param.Format);
			buffer_ = bgfx::createTexture2D(
				param.Size[0],
				param.Size[1],
				false,
				1,
				format,
				BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT_WRITE_ONLY,
				nullptr
				);

			size_ = param.Size;
			format_ = param.Format;
			hasMipmap_ = false;

			type_ = Effekseer::Backend::TextureType::Depth;

			return true;
		}

		bool Texture::Init(bgfx::TextureHandle buffer, bool hasMipmap, const std::function<void()>& onDisposed)
		{
			if (!bgfx::isValid(buffer))
				return false;

			buffer_ = buffer;
			onDisposed_ = onDisposed;
			hasMipmap_ = hasMipmap;

			type_ = Effekseer::Backend::TextureType::Color2D;

			return true;
		}
	}
} // namespace EffekseerRendererBGFX
