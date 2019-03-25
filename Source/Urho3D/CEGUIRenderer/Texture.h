#pragma once
#include "CEGUI/Texture.h"
#include "Renderer.h"
#include "Urho3D/Urho3D.h"
#include "Urho3D/Container/Ptr.h"

namespace Urho3D
{
	class Texture2D;
}

namespace CEGUI
{
	class URHO3D_API Urho3DTexture : public Texture
	{
	public:
		//! Set the underlying Ogre texture.
		void setUrho3DTexture(const Urho3D::SharedPtr<Urho3D::Texture2D>& texture, bool take_ownership = false);
		//! Return Ogre::TexturePtr for the underlying Ogre texture.
		Urho3D::SharedPtr<Urho3D::Texture2D> getUrho3DTexture() const;

		//! return a Ogre::string containing a unique name.
		static Urho3D::String getUniqueName();

		// implement CEGUI::Texture interface
		const String& getName() const;
		const Sizef& getSize() const;
		const Sizef& getOriginalDataSize() const;
		const glm::vec2& getTexelScaling() const;
		void loadFromFile(const String& filename, const String& resourceGroup);
		void loadFromMemory(const void* buffer, const Sizef& buffer_size,
			PixelFormat pixel_format);
		void blitFromMemory(const void* sourceData, const Rectf& area);
		void blitToMemory(void* targetData);
		bool isPixelFormatSupported(const PixelFormat fmt) const;

		//! convert Ogre::PixelFormat to equivalent CEGUI::Texture::PixelFormat
		static Texture::PixelFormat fromUrho3DPixelFormat(const unsigned fmt);
		//! convert CEGUI::Texture::PixelFormat to equivalent Ogre::PixelFormat
		static unsigned toUrho3DPixelFormat(const Texture::PixelFormat fmt);

	protected:
		// we all need a little help from out friends ;)
		friend Texture& Urho3DRenderer::createTexture(const String&);
		friend Texture& Urho3DRenderer::createTexture(const String&, const String&, const String&);
		friend Texture& Urho3DRenderer::createTexture(const String&, const Sizef&);
		friend Texture& Urho3DRenderer::createTexture(const String&, Urho3D::SharedPtr<Urho3D::Texture2D>&, bool);
		friend void Urho3DRenderer::destroyTexture(Texture&);
		friend void Urho3DRenderer::destroyTexture(const String&);

		//! standard constructor
		Urho3DTexture(const String& name);
		//! construct texture via an image file.
		Urho3DTexture(const String& name, const String& filename,
			const String& resourceGroup);
		//! construct texture with a specified initial size.
		Urho3DTexture(const String& name, const Sizef& sz);
		//! construct texture from existing Ogre texture.
		Urho3DTexture(const String& name, Urho3D::SharedPtr<Urho3D::Texture2D>& tex, bool take_ownership);


		//! destructor.
		virtual ~Urho3DTexture();
		//! construct an empty texture
		void createEmptyUrho3DTexture(PixelFormat pixel_format);
		//! release the underlying Urho3D texture.
		void freeUrho3DTexture();
		//! updates cached scale value used to map pixels to texture co-ords.
		void updateCachedScaleValues();

		//! Counter used to provide unique texture names.
		static std::uint32_t d_textureNumber;
		//!< The underlying Ogre texture.
		Urho3D::SharedPtr<Urho3D::Texture2D> d_texture;
		//! specifies whether d_texture was created externally (not owned by us).
		bool d_isLinked;
		//! Size of the texture.
		Sizef d_size;
		//! original pixel of size data loaded into texture
		Sizef d_dataSize;
		//! cached pixel to texel mapping scale values.
		glm::vec2 d_texelScaling;
		//! Name this texture was created with.
		const String d_name;
	};

}