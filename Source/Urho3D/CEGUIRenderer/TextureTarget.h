#pragma once
#include "CEGUI/TextureTarget.h"
#include "RenderTarget.h"
#include "Urho3D/Urho3D.h"

namespace Urho3D
{
	class Renderer;
}

namespace CEGUI
{
	class Urho3DRenderer;
	class Urho3DTexture;
	class URHO3D_API Urho3DTextureTarget : public Urho3DRenderTarget, public TextureTarget
	{
	public:
		//! Constructor.
		Urho3DTextureTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs, bool addStencilBuffer);
		//! Destructor.
		virtual ~Urho3DTextureTarget();

		// implementation of RenderTarget interface
		bool isImageryCache() const;
		// implement CEGUI::TextureTarget interface.
		void clear();
		Texture& getTexture() const;
		void declareRenderSize(const Sizef& sz);

	protected:
		//! default / initial size for the underlying texture.
		static const float DEFAULT_SIZE;
		//! static data used for creating texture names
		static std::uint32_t s_textureNumber;
		//! helper to generate unique texture names
		static String generateTextureName();
		//! This wraps d_texture so it can be used by the core CEGUI lib.
		Urho3DTexture* d_CEGUITexture;
	};
}