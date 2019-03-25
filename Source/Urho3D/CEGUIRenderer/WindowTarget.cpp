#include "WindowTarget.h"
#include "Urho3D/Graphics/RenderSurface.h"

namespace CEGUI
{
	Urho3DWindowTarget::Urho3DWindowTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs/*, Urho3D::RenderSurface& target*/)
		: Urho3DRenderTarget(owner, rs)
	{
		initRenderTarget(/*target*/);
	}

	Urho3DWindowTarget::~Urho3DWindowTarget()
	{
	}

	void Urho3DWindowTarget::setUrho3DRenderTarget(Urho3D::RenderSurface& target)
	{
		// cleanup viewport since it's RT dependent.
		delete d_viewport;
		d_viewport = 0;
		d_viewportValid = false;

		initRenderTarget(/*target*/);
	}

	bool Urho3DWindowTarget::isImageryCache() const
	{
		return false;
	}

	void Urho3DWindowTarget::initRenderTarget(/*Urho3D::RenderSurface& target*/)
	{
		//d_renderTarget = &target;

		Rectf init_area(glm::vec2(0.0f, 0.0f),
			Sizef(static_cast<float>(d_renderTarget->GetWidth()),
				static_cast<float>(d_renderTarget->GetHeight())));

		setArea(init_area);
	}
}