#include "RenderTarget.h"
#include "GeometryBuffer.h"
#include "CEGUI/Exceptions.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CEGUI
{
	Urho3DRenderTarget::Urho3DRenderTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs)
		: d_owner{ owner }, d_renderSystem{ rs }
	{
	}

	Urho3DRenderTarget::~Urho3DRenderTarget()
	{
		delete d_viewport;
	}

	void Urho3DRenderTarget::setOgreViewportDimensions(const Rectf& area)
	{
// 		d_ogreViewportDimensions = area;
// 
// 		if (d_viewport)
// 			updateOgreViewportDimensions(d_viewport->getTarget());
// 
// 		d_viewportValid = false;
	}


	void Urho3DRenderTarget::updateOgreViewportDimensions(const Urho3D::RenderSurface* const rt)
	{
// 		if (rt)
// 		{
// 			if (d_viewport)
// 				d_viewport->setDimensions(
// 					d_ogreViewportDimensions.left() / rt->getWidth(),
// 					d_ogreViewportDimensions.top() / rt->getHeight(),
// 					d_ogreViewportDimensions.getWidth() / rt->getWidth(),
// 					d_ogreViewportDimensions.getHeight() / rt->getHeight());
// 		}
	}


	void Urho3DRenderTarget::activate()
	{
// 		if (!RenderTarget::d_matrixValid)
// 			updateMatrix();
// 
// 		if (!d_viewportValid)
// 			updateViewport();
// 
// 		d_renderSystem._setViewport(d_viewport);
// 
// 		d_owner.setViewProjectionMatrix(RenderTarget::d_matrix);
// #ifdef CEGUI_USE_OGRE_HLMS
// 		d_owner.initialiseRenderStateSettings(d_renderTarget);
// #else
// 		d_owner.initialiseRenderStateSettings();
// #endif
// 
// 		RenderTarget::activate();
	}


	void Urho3DRenderTarget::unprojectPoint(const GeometryBuffer& buff, const glm::vec2& p_in, glm::vec2& p_out) const
	{
// 		if (!RenderTarget::d_matrixValid)
// 			updateMatrix();
// 
// 		const Urho3DGeometryBuffer& gb = static_cast<const Urho3DGeometryBuffer&>(buff);
// 
// 		const Ogre::Real midx = RenderTarget::d_area.getWidth() * 0.5f;
// 		const Ogre::Real midy = RenderTarget::d_area.getHeight() * 0.5f;
// 
// 		// viewport matrix
// 		const Ogre::Matrix4 vpmat(
// 			midx, 0, 0, RenderTarget::d_area.left() + midx,
// 			0, -midy, 0, RenderTarget::d_area.top() + midy,
// 			0, 0, 1, 0,
// 			0, 0, 0, 1
// 		);
// 
// 		// matrices used for projecting and unprojecting points
// 
// 		const Ogre::Matrix4 proj(Urho3DRenderer::glmToOgreMatrix(gb.getModelMatrix() * RenderTarget::d_matrix) * vpmat);
// 		const Ogre::Matrix4 unproj(proj.inverse());
// 
// 		Ogre::Vector3 in;
// 
// 		// unproject the ends of the ray
// 		in.x = midx;
// 		in.y = midy;
// 		in.z = -RenderTarget::d_viewDistance;
// 		const Ogre::Vector3 r1(unproj * in);
// 		in.x = p_in.x;
// 		in.y = p_in.y;
// 		in.z = 0;
// 		// calculate vector of picking ray
// 		const Ogre::Vector3 rv(r1 - unproj * in);
// 
// 		// project points to orientate them with GeometryBuffer plane
// 		in.x = 0.0;
// 		in.y = 0.0;
// 		const Ogre::Vector3 p1(proj * in);
// 		in.x = 1.0;
// 		in.y = 0.0;
// 		const Ogre::Vector3 p2(proj * in);
// 		in.x = 0.0;
// 		in.y = 1.0;
// 		const Ogre::Vector3 p3(proj * in);
// 
// 		// calculate the plane normal
// 		const Ogre::Vector3 pn((p2 - p1).crossProduct(p3 - p1));
// 		// calculate distance from origin
// 		const Ogre::Real plen = pn.length();
// 		const Ogre::Real dist = -(p1.x * (pn.x / plen) +
// 			p1.y * (pn.y / plen) +
// 			p1.z * (pn.z / plen));
// 
// 		// calculate intersection of ray and plane
// 		const Ogre::Real pn_dot_rv = pn.dotProduct(rv);
// 		const Ogre::Real tmp = pn_dot_rv != 0.0 ?
// 			(pn.dotProduct(r1) + dist) / pn_dot_rv :
// 			0.0f;
// 
// 		p_out.x = static_cast<float>(r1.x - rv.x * tmp);
// 		p_out.y = static_cast<float>(r1.y - rv.y * tmp);
	}


	void Urho3DRenderTarget::updateMatrix() const
	{
// 		if (d_owner.usesOpenGL())
// 			RenderTarget::updateMatrix(RenderTarget::createViewProjMatrixForOpenGL());
// 		else if (d_owner.usesDirect3D())
// 			RenderTarget::updateMatrix(RenderTarget::createViewProjMatrixForDirect3D());
// 		else
// 			throw RendererException("An unsupported RenderSystem is being used by Ogre. Please contact the CEGUI team.");
	}


	void Urho3DRenderTarget::updateViewport()
	{
// 		if (!d_viewport)
// 		{
// #ifdef CEGUI_USE_OGRE_COMPOSITOR2
// 
// 			d_viewport = OGRE_NEW Ogre::Viewport(d_renderTarget, 0, 0, 1, 1);
// #else
// 			d_viewport = OGRE_NEW Ogre::Viewport(0, d_renderTarget, 0, 0, 1, 1, 0);
// #endif // CEGUI_USE_OGRE_COMPOSITOR2
// 
// 			updateOgreViewportDimensions(d_renderTarget);
// 		}
// 
// 		d_viewport->_updateDimensions();
// 
// 		d_viewportValid = true;
	}


	Urho3DRenderer& Urho3DRenderTarget::getOwner()
	{
		return d_owner;
	}


	void Urho3DRenderTarget::setArea(const Rectf& area)
	{
		setOgreViewportDimensions(area);

		RenderTarget::setArea(area);
	}
}