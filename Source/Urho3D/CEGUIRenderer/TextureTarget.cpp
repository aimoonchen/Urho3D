#include "TextureTarget.h"
#include "Texture.h"
#include "CEGUI/PropertyHelper.h"

namespace CEGUI
{
	const float Urho3DTextureTarget::DEFAULT_SIZE = 128.0f;
	std::uint32_t Urho3DTextureTarget::s_textureNumber = 0;

	//----------------------------------------------------------------------------//
	Urho3DTextureTarget::Urho3DTextureTarget(Urho3DRenderer& owner,
		Urho3D::Graphics& rs,
		bool addStencilBuffer) :
		Urho3DRenderTarget(owner, rs),
		TextureTarget(addStencilBuffer),
		d_CEGUITexture(0)
	{
		d_CEGUITexture = static_cast<Urho3DTexture*>(
			&d_owner.createTexture(generateTextureName()));

		// setup area and cause the initial texture to be generated.
		declareRenderSize(Sizef(DEFAULT_SIZE, DEFAULT_SIZE));
	}

	//----------------------------------------------------------------------------//
	Urho3DTextureTarget::~Urho3DTextureTarget()
	{
		d_owner.destroyTexture(*d_CEGUITexture);
	}

	//----------------------------------------------------------------------------//
	bool Urho3DTextureTarget::isImageryCache() const
	{
		return true;
	}

	//----------------------------------------------------------------------------//
	void Urho3DTextureTarget::clear()
	{
// 		if (!d_viewportValid)
// 			updateViewport();
// 
// 		Ogre::Viewport* const saved_vp = d_renderSystem._getViewport();
// 
// 		d_renderSystem._setViewport(d_viewport);
// 		d_renderSystem.clearFrameBuffer(Ogre::FBT_COLOUR,
// 			Ogre::ColourValue(0, 0, 0, 0));
// 
// #if OGRE_VERSION < 0x10800
// 		if (saved_vp)
// 			d_renderSystem._setViewport(saved_vp);
// #else
// 		d_renderSystem._setViewport(saved_vp);
// #endif
	}

	//----------------------------------------------------------------------------//
	Texture& Urho3DTextureTarget::getTexture() const
	{
		return *d_CEGUITexture;
	}

	//----------------------------------------------------------------------------//
	void Urho3DTextureTarget::declareRenderSize(const Sizef& sz)
	{
// 		// exit if current size is enough
// 		if ((d_area.getWidth() >= sz.d_width) && (d_area.getHeight() >= sz.d_height))
// 			return;
// 
// 		Ogre::TexturePtr rttTex = Ogre::TextureManager::getSingleton().createManual(Urho3DTexture::getUniqueName(),
// 			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
// 			Ogre::TEX_TYPE_2D,
// 			static_cast<Ogre::uint>(sz.d_width),
// 			static_cast<Ogre::uint>(sz.d_height),
// 			1,
// 			0,
// 			Ogre::PF_A8R8G8B8,
// 			Ogre::TU_RENDERTARGET);
// 
// 		d_renderTarget = rttTex->getBuffer()->getRenderTarget();
// 
// 		const Rectf init_area(
// 			glm::vec2(0.0f, 0.0f),
// 			Sizef(
// 				static_cast<float>(d_renderTarget->getWidth()),
// 				static_cast<float>(d_renderTarget->getHeight())
// 			)
// 		);
// 
// 		setArea(init_area);
// 
// 		// delete viewport and reset ptr so a new one is generated.  This is
// 		// required because we have changed d_renderTarget so need a new VP also.
// 		OGRE_DELETE d_viewport;
// 		d_viewport = 0;
// 
// 		// because Texture takes ownership, the act of setting the new ogre texture
// 		// also ensures any previous ogre texture is released.
// 		d_CEGUITexture->setOgreTexture(rttTex, true);
// 
// 		clear();
	}

	//----------------------------------------------------------------------------//
	String Urho3DTextureTarget::generateTextureName()
	{
		String tmp("_ogre_tt_tex_");
		tmp.append(PropertyHelper<std::uint32_t>::toString(s_textureNumber++));

		return tmp;
	}

}