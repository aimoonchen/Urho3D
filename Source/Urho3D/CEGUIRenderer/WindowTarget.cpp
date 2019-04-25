#include "WindowTarget.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderSurface.h"
namespace CEGUI
{
	Urho3DWindowTarget::Urho3DWindowTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs)
		: Urho3DRenderTarget(owner, rs)
	{
		Rectf init_area(glm::vec2(0.0f, 0.0f),
			Sizef(static_cast<float>(d_graphics.GetWidth()),
				static_cast<float>(d_graphics.GetHeight())));
		RenderTarget::setArea(init_area);
	}

	Urho3DWindowTarget::Urho3DWindowTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs, const Rectf& area)
		: Urho3DRenderTarget(owner, rs)
	{
		RenderTarget::setArea(area);
	}

	Urho3DWindowTarget::~Urho3DWindowTarget()
	{
	}
}