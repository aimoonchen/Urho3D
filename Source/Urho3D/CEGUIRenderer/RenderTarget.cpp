#include "RenderTarget.h"
#include "GeometryBuffer.h"
#include "CEGUI/Exceptions.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/Graphics.h"

namespace CEGUI
{
	Urho3DRenderTarget::Urho3DRenderTarget(Urho3DRenderer& owner, Urho3D::Graphics& rs)
		: d_owner{ owner }, d_graphics{ rs }
	{
	}

	Urho3DRenderTarget::~Urho3DRenderTarget()
	{

	}

// 	void Urho3DRenderTarget::setUrho3DViewportDimensions(const Rectf& area)
// 	{
// 		d_urho3DViewportDimensions = area;
// 
// 		if (true/*d_viewport*/)
// 			updateUrho3DViewportDimensions(nullptr/*d_viewport->getTarget()*/);
// 
// 		d_viewportValid = false;
// 	}
// 
// 
// 	void Urho3DRenderTarget::updateUrho3DViewportDimensions(const Urho3D::RenderSurface* const rt)
// 	{
// // 		if (rt)
// // 		{
// // 			if (d_viewport)
// // 				d_viewport->setDimensions(
// // 					d_ogreViewportDimensions.left() / rt->getWidth(),
// // 					d_ogreViewportDimensions.top() / rt->getHeight(),
// // 					d_ogreViewportDimensions.getWidth() / rt->getWidth(),
// // 					d_ogreViewportDimensions.getHeight() / rt->getHeight());
// // 		}
// 
// // 		d_graphics.SetViewport({static_cast<int>(d_urho3DViewportDimensions.left() / 1280.0f),
// // 			static_cast<int>(d_urho3DViewportDimensions.top() / 720.0f),
// // 			static_cast<int>(d_urho3DViewportDimensions.right() / 1280.0f),
// // 			static_cast<int>(d_urho3DViewportDimensions.bottom() / 720.0f)});
// 
// 		d_graphics.SetViewport({ static_cast<int>(RenderTarget::d_area.left()),
// 			   static_cast<int>(RenderTarget::d_area.top()),
// 			   static_cast<int>(RenderTarget::d_area.getWidth()),
// 			   static_cast<int>(RenderTarget::d_area.getHeight()) });
// 	}


	void Urho3DRenderTarget::activate()
	{
		d_graphics.SetViewport({ static_cast<int>(RenderTarget::d_area.left()),
								static_cast<int>(RenderTarget::d_area.top()),
								static_cast<int>(RenderTarget::d_area.getWidth()),
								static_cast<int>(RenderTarget::d_area.getHeight()) });

		if (!RenderTarget::d_matrixValid)
			updateMatrix();

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
		d_owner.setViewProjectionMatrix(RenderTarget::d_matrix);

 		RenderTarget::activate();
	}


	void Urho3DRenderTarget::unprojectPoint(const GeometryBuffer& buff, const glm::vec2& p_in, glm::vec2& p_out) const
	{
		if (!RenderTarget::d_matrixValid)
			updateMatrix();

		const Urho3DGeometryBuffer& gb = static_cast<const Urho3DGeometryBuffer&>(buff);

		const int vp[4] = {
			static_cast<int>(RenderTarget::d_area.left()),
			static_cast<int>(RenderTarget::d_area.top()),
			static_cast<int>(RenderTarget::d_area.getWidth()),
			static_cast<int>(RenderTarget::d_area.getHeight())
		};

		float in_x, in_y = 0.0f, in_z = 0.0f;

		glm::ivec4 viewPort = glm::ivec4(vp[0], vp[1], vp[2], vp[3]);
		const glm::mat4& projMatrix = RenderTarget::d_matrix;
		const glm::mat4& modelMatrix = gb.getModelMatrix();

		// unproject the ends of the ray
		glm::vec3 unprojected1;
		glm::vec3 unprojected2;
		in_x = vp[2] * 0.5f;
		in_y = vp[3] * 0.5f;
		in_z = -RenderTarget::d_viewDistance;
		unprojected1 = glm::unProject(glm::vec3(in_x, in_y, in_z), modelMatrix, projMatrix, viewPort);
		in_x = p_in.x;
		in_y = vp[3] - p_in.y;
		in_z = 0.0;
		unprojected2 = glm::unProject(glm::vec3(in_x, in_y, in_z), modelMatrix, projMatrix, viewPort);

		// project points to orientate them with GeometryBuffer plane
		glm::vec3 projected1;
		glm::vec3 projected2;
		glm::vec3 projected3;
		in_x = 0.0;
		in_y = 0.0;
		projected1 = glm::project(glm::vec3(in_x, in_y, in_z), modelMatrix, projMatrix, viewPort);
		in_x = 1.0;
		in_y = 0.0;
		projected2 = glm::project(glm::vec3(in_x, in_y, in_z), modelMatrix, projMatrix, viewPort);
		in_x = 0.0;
		in_y = 1.0;
		projected3 = glm::project(glm::vec3(in_x, in_y, in_z), modelMatrix, projMatrix, viewPort);

		// calculate vectors for generating the plane
		const glm::vec3 pv1 = projected2 - projected1;
		const glm::vec3 pv2 = projected3 - projected1;
		// given the vectors, calculate the plane normal
		const glm::vec3 planeNormal = glm::cross(pv1, pv2);
		// calculate plane
		const glm::vec3 planeNormalNormalized = glm::normalize(planeNormal);
		const double pl_d = -glm::dot(projected1, planeNormalNormalized);
		// calculate vector of picking ray
		const glm::vec3 rv = unprojected1 - unprojected2;
		// calculate intersection of ray and plane
		const double pn_dot_r1 = glm::dot(unprojected1, planeNormal);
		const double pn_dot_rv = glm::dot(rv, planeNormal);
		const double tmp1 = pn_dot_rv != 0.0 ? (pn_dot_r1 + pl_d) / pn_dot_rv : 0.0;
		const double is_x = unprojected1.x - rv.x * tmp1;
		const double is_y = unprojected1.y - rv.y * tmp1;

		p_out.x = static_cast<float>(is_x);
		p_out.y = static_cast<float>(is_y);
	}
	Urho3DRenderer& Urho3DRenderTarget::getOwner()
	{
		return d_owner;
	}

	void Urho3DRenderTarget::updateMatrix() const
	{
// 		if (d_owner.usesOpenGL())
// 			RenderTarget::updateMatrix(RenderTarget::createViewProjMatrixForOpenGL());
// 		else if (d_owner.usesDirect3D())
// 			RenderTarget::updateMatrix(RenderTarget::createViewProjMatrixForDirect3D());
// 		else
// 			throw RendererException("An unsupported RenderSystem is being used by Ogre. Please contact the CEGUI team.");


		//
		Urho3D::RenderSurface* surface = d_graphics.GetRenderTarget(0);
		Urho3D::IntVector2 viewSize = d_graphics.GetViewport().Size();
		Urho3D::Vector2 invScreenSize(1.0f / (float)viewSize.x_, 1.0f / (float)viewSize.y_);
		Urho3D::Vector2 scale(2.0f * invScreenSize.x_, -2.0f * invScreenSize.y_);
		Urho3D::Vector2 offset(-1.0f, 1.0f);
		float uiScale_ = 1.0f;
		if (surface) {
#ifdef URHO3D_OPENGL
			// On OpenGL, flip the projection if rendering to a texture so that the texture can be addressed in the
			// same way as a render texture produced on Direct3D.
			offset.y_ = -offset.y_;
			scale.y_ = -scale.y_;
#endif
		}

// 		Urho3D::Matrix4 projection(Urho3D::Matrix4::IDENTITY);
// 		projection.m00_ = scale.x_ * uiScale_;
// 		projection.m03_ = offset.x_;
// 		projection.m11_ = scale.y_ * uiScale_;
// 		projection.m13_ = offset.y_;
// 		projection.m22_ = 1.0f;
// 		projection.m23_ = 0.0f;
// 		projection.m33_ = 1.0f;

// 		glm::mat4 adjustMat{
// 			glm::vec4{scale.x_ * uiScale_, 0.0f, 0.0f, offset.x_},
// 			glm::vec4{0.0f, scale.y_ * uiScale_, 0.0f, offset.y_},
// 			glm::vec4{0.0f, 0.0f, 1.0f, 0.0f},
// 			glm::vec4{0.0f, 0.0f, 0.0f, 1.0f} };
// 
// 		auto vpm = RenderTarget::createViewProjMatrixForOpenGL();
		
		RenderTarget::updateMatrix(/*adjustMat * vpm*/RenderTarget::createViewProjMatrixForOpenGL());
	}


// 	void Urho3DRenderTarget::updateViewport()
// 	{
// // 		if (!d_viewport)
// // 		{
// // #ifdef CEGUI_USE_OGRE_COMPOSITOR2
// // 
// // 			d_viewport = OGRE_NEW Ogre::Viewport(d_renderTarget, 0, 0, 1, 1);
// // #else
// // 			d_viewport = OGRE_NEW Ogre::Viewport(0, d_renderTarget, 0, 0, 1, 1, 0);
// // #endif // CEGUI_USE_OGRE_COMPOSITOR2
// // 
//  			updateUrho3DViewportDimensions(d_renderTarget);
// // 		}
// // 
// // 		d_viewport->_updateDimensions();
// // 
//  		d_viewportValid = true;
// 	}
// 
// 
// 	Urho3DRenderer& Urho3DRenderTarget::getOwner()
// 	{
// 		return d_owner;
// 	}
// 
// 
// 	void Urho3DRenderTarget::setArea(const Rectf& area)
// 	{
// 		setUrho3DViewportDimensions(area);
// 
// 		RenderTarget::setArea(area);
// 	}


}