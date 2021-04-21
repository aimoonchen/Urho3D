//#include "bgfx_utils.h"
#include "EffekseerRendererBGFX.Texture.h"
#include "EffekseerRendererBGFX.Renderer.h"
#include "../../../Cocos2d/Urho3DContext.h"
#include "../../../Core/Context.h"
#include "../../../Graphics/Graphics.h"
#include "../../../Graphics/Texture2D.h"

namespace EffekseerRendererBGFX {
	namespace Backend {
		static /*bgfx::TextureFormat::Enum*/uint32_t GetBGFXTextureFormat(Effekseer::Backend::TextureFormatType EffekseerFormat)
		{
			//bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Unknown;
			unsigned int format = 0;
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
				format = Urho3D::CF_DXT1;
				break;
			case Effekseer::Backend::TextureFormatType::BC2:
				format = Urho3D::CF_DXT3;
				break;
			case Effekseer::Backend::TextureFormatType::BC3:
				format = Urho3D::CF_DXT5;
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
				//bgfx::destroy(buffer_);
				delete buffer_;
			}

			//ES_SAFE_RELEASE(graphicsDevice_);
		}

		bool Texture::InitInternal(const Effekseer::Backend::TextureParameter& param)
		{
			auto format = GetBGFXTextureFormat(param.Format);
			Urho3D::TextureUsage usage = Urho3D::TEXTURE_STATIC;
			//uint64_t textureFlags = BGFX_SAMPLER_U_MIRROR | BGFX_SAMPLER_V_MIRROR | BGFX_SAMPLER_NONE;
			if (type_ == Effekseer::Backend::TextureType::Render)
			{
				//textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT;
				usage = Urho3D::TEXTURE_RENDERTARGET;
			}

// 			buffer_ = bgfx::createTexture2D(
// 				param.Size[0],
// 				param.Size[1],
// 				false,// param.GenerateMipmap,
// 				1,
// 				format,
// 				textureFlags,
// 				bgfx::copy(param.InitialData.data(), param.InitialData.size())
// 				);
            buffer_ = new Urho3D::Texture2D(GetUrho3DContext());
            buffer_->SetSize(param.Size[0], param.Size[1], format, usage);
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
// 			buffer_ = bgfx::createTexture2D(
// 				param.Size[0],
// 				param.Size[1],
// 				false,
// 				1,
// 				format,
// 				BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT_WRITE_ONLY,
// 				nullptr
// 				);
			buffer_ = new Urho3D::Texture2D(GetUrho3DContext());
			buffer_->SetSize(param.Size[0], param.Size[1], format);

			size_ = param.Size;
			format_ = param.Format;
			hasMipmap_ = false;

			type_ = Effekseer::Backend::TextureType::Depth;

			return true;
		}

		bool Texture::Init(Urho3D::Texture2D* buffer, bool hasMipmap, const std::function<void()>& onDisposed)
		{
			if (!buffer->IsValid())
				return false;

			buffer_ = buffer;
			onDisposed_ = onDisposed;
			hasMipmap_ = hasMipmap;

			type_ = Effekseer::Backend::TextureType::Color2D;

			return true;
		}
	}
} // namespace EffekseerRendererBGFX
