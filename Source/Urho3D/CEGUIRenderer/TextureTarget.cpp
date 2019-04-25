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
		//d_graphics.SetDepthStencil(surface->GetLinkedDepthStencil());
		d_graphics.SetRenderTarget(0, surface);
		// 		d_graphics.SetViewport(IntRect(0, 0, surface->GetWidth(), surface->GetHeight()));
		// 		d_graphics.Clear(Urho3D::CLEAR_COLOR);
		Urho3DRenderTarget::activate();
	}

	void Urho3DTextureTarget::deactivate()
	{
		Urho3DRenderTarget::deactivate();

		d_graphics.ResetRenderTargets();
	}
	//----------------------------------------------------------------------------//
// 	bool Urho3DTextureTarget::isImageryCache() const
// 	{
// 		return true;
// 	}

	//----------------------------------------------------------------------------//
	void Urho3DTextureTarget::clear()
	{
		const Sizef sz(d_area.getSize());
		// Some drivers crash when clearing a 0x0 RTT. This is a workaround for
		// those cases.
		if (sz.d_width < 1.0f || sz.d_height < 1.0f)
			return;

		auto& urho3d_tex = d_CEGUITexture->getUrho3DTexture();
		Urho3D::RenderSurface* surface = urho3d_tex->GetRenderSurface();
		//d_graphics.SetDepthStencil(surface->GetLinkedDepthStencil());
		d_graphics.SetRenderTarget(0, surface);
		//d_graphics.SetViewport(Urho3D::IntRect(0, 0, surface->GetWidth(), surface->GetHeight()));
		auto old_test = d_graphics.GetScissorTest();
		auto old_rect = d_graphics.GetScissorRect();
		d_graphics.SetScissorTest(false);
		d_graphics.Clear(Urho3D::CLEAR_COLOR);
		d_graphics.SetScissorTest(old_test, old_rect);
	}

	//----------------------------------------------------------------------------//
	Texture& Urho3DTextureTarget::getTexture() const
	{
		return *d_CEGUITexture;
	}

	//----------------------------------------------------------------------------//
	void Urho3DTextureTarget::declareRenderSize(const Sizef& sz)
	{
		setArea(Rectf(d_area.getPosition(), Sizef(sz)));

		Sizef adjust_sz(d_area.getSize());
		if (sz.d_width < 1.0f || sz.d_height < 1.0f)
		{
			adjust_sz.d_width = 1.0f;
			adjust_sz.d_height = 1.0f;
		}

		auto& urho3d_tex = d_CEGUITexture->getUrho3DTexture();
		if (urho3d_tex->SetSize(static_cast<int>(adjust_sz.d_width),
			static_cast<int>(adjust_sz.d_height),
			Urho3D::Graphics::GetRGBAFormat(), Urho3D::TEXTURE_RENDERTARGET)) {
			urho3d_tex->GetRenderSurface()->SetUpdateMode(Urho3D::SURFACE_MANUALUPDATE);
		}

		d_CEGUITexture->updateSize();

		if (d_usesStencil)
		{
			;
		}
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