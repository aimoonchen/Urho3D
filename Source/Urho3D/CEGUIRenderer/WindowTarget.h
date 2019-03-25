#pragma once
#include "RenderTarget.h"
#include "Urho3D/Urho3D.h"

namespace CEGUI
{
	class URHO3D_API Urho3DWindowTarget : public Urho3DRenderTarget
	{
	public:
		Urho3DWindowTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs/*, Urho3D::RenderSurface& target*/);
		virtual ~Urho3DWindowTarget();
		void setUrho3DRenderTarget(Urho3D::RenderSurface& target);
		bool isImageryCache() const;
	protected:
		void initRenderTarget(/*Urho3D::RenderSurface& target*/);
	};
}