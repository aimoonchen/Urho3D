#include "TextureTarget.h"
#include "Texture.h"
#include "CEGUI/PropertyHelper.h"

#include "Graphics/Graphics.h"

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
		d_CEGUITexture = static_cast<Urho3DTexture*>(&d_owner.createTexture(generateTextureName()));
		auto& urho3d_tex = d_CEGUITexture->getUrho3DTexture();
		urho3d_tex->SetFilterMode(Urho3D::FILTER_BILINEAR);
		urho3d_tex->SetAddressMode(Urho3D::COORD_U, Urho3D::ADDRESS_CLAMP);
		urho3d_tex->SetAddressMode(Urho3D::COORD_V, Urho3D::ADDRESS_CLAMP);
		urho3d_tex->SetNumLevels(1);
		// setup area and cause the initial texture to be generated.
		declareRenderSize(Sizef(DEFAULT_SIZE, DEFAULT_SIZE));
	}

	//----------------------------------------------------------------------------//
	Urho3DTextureTarget::~Urho3DTextureTarget()
	{
		d_owner.destroyTexture(*d_CEGUITexture);
	}

	void Urho3DTextureTarget::activate()
	{
		auto& urho3d_tex = d_CEGUITexture->getUrho3DTexture();
		Urho3D::RenderSurface* surface = urho3d_tex->GetRenderSurface();
		d_graphics.SetDepthStencil(surface->GetLinkedDepthStencil());
		d_graphics.SetRenderTarget(0, surface);
		// 		d_graphics.SetViewport(IntRect(0, 0, surface->GetWidth(), surface->GetHeight()));
		// 		d_graphics.Clear(Urho3D::CLEAR_COLOR);
		Urho3DRenderTarget::activate();
// 		d_graphics.SetViewport(Urho3D::IntRect(0, 0, surface->GetWidth(), surface->GetHeight()));
 		d_graphics.Clear(Urho3D::CLEAR_COLOR);
	}

	void Urho3DTextureTarget::deactivate()
	{
		Urho3DRenderTarget::deactivate();

		d_graphics.ResetRenderTargets();
	}
	//----------------------------------------------------------------------------//
	bool Urho3DTextureTarget::isImageryCache() const
	{
		return true;
	}

	//----------------------------------------------------------------------------//
	void Urho3DTextureTarget::clear()
	{
		if (!d_viewportValid)
			updateViewport();
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
		auto& urho3d_tex = d_CEGUITexture->getUrho3DTexture();
		Urho3D::RenderSurface* surface = urho3d_tex->GetRenderSurface();
		d_graphics.SetDepthStencil(surface->GetLinkedDepthStencil());
		d_graphics.SetRenderTarget(0, surface);
		d_graphics.SetViewport(Urho3D::IntRect(0, 0, surface->GetWidth(), surface->GetHeight()));
		d_graphics.Clear(Urho3D::CLEAR_COLOR);
	}

	//----------------------------------------------------------------------------//
	Texture& Urho3DTextureTarget::getTexture() const
	{
		return *d_CEGUITexture;
	}

	//----------------------------------------------------------------------------//
	void Urho3DTextureTarget::declareRenderSize(const Sizef& sz)
	{
		auto& urho3d_tex = d_CEGUITexture->getUrho3DTexture();
		if (urho3d_tex->SetSize(static_cast<int>(sz.d_width),
			static_cast<int>(sz.d_height),
			Urho3D::Graphics::GetRGBAFormat(), Urho3D::TEXTURE_RENDERTARGET)) {
			urho3d_tex->GetRenderSurface()->SetUpdateMode(Urho3D::SURFACE_MANUALUPDATE);
		}

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
 
 		setArea(Rectf(d_area.getPosition(), Sizef(sz)));
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