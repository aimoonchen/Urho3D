#pragma once
#include "CEGUI/RenderTarget.h"
#include "Renderer.h"
#include "CEGUI/Rectf.h"

namespace Urho3D
{
	class Renderer;
	class Viewport;
}

namespace CEGUI
{
	class Urho3DRenderer;
	class URHO3D_API Urho3DRenderTarget : virtual public RenderTarget
	{
	public:
		//! Constructor
		Urho3DRenderTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs);

		//! Destructor
		virtual ~Urho3DRenderTarget();

		/*!
		\brief
			Set the underlying viewport area directly - bypassing what the
			RenderTarget considers to be it's area - thus allowing the view port
			area used for rendering to be different to the area set for the target.

		\param area
			Rect object describing the area to use in pixels.

		\deprecated
			This function is deprecated and will be removed or changed considerably
			in future releases.
		*/
		void setOgreViewportDimensions(const Rectf& area);

		// implement parts of CEGUI::RenderTarget interface
		virtual void activate();
		virtual void unprojectPoint(const GeometryBuffer& buff, const glm::vec2& p_in, glm::vec2& p_out) const;
		virtual void setArea(const Rectf& area);
		// implementing the virtual function with a covariant return type
		virtual Urho3DRenderer& getOwner();

	protected:
		//! helper that initialises the cached matrix
		void updateMatrix() const;
		//! helper that initialises the viewport
		void updateViewport();
		//! helper to update the actual Ogre viewport dimensions
		void updateOgreViewportDimensions(const Urho3D::RenderSurface* const rt);

		//! Urho3DRenderer object that owns this RenderTarget
		Urho3DRenderer& d_owner;
		//! Urho3D RendererSystem used to affect the rendering process
		Urho3D::Graphics& d_renderSystem;
		//! Urho3D render target that we are effectively wrapping
		Urho3D::RenderSurface* d_renderTarget{ nullptr };

		//! Urho3D viewport used for this target.
		Urho3D::Viewport* d_viewport{ nullptr };
		//! holds set Ogre viewport dimensions
		Rectf d_ogreViewportDimensions{ 0, 0, 0, 0 };

		//! true when d_viewport is up to date and valid.
		//! \version Beginning from Ogre 2.0 this indicates whether the workspace is
		//! up to date
		bool d_viewportValid{ false };
	};
}