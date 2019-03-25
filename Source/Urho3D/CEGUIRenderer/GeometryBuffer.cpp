#include "GeometryBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Urho3D/Graphics/Graphics.h"

#include "CEGUI/Vertex.h"
#include "CEGUI/RenderEffect.h"
#include "CEGUI/Exceptions.h"
#include "CEGUI/ShaderParameterBindings.h"

#include "Texture.h"
#include "ShaderWrapper.h"

#define FLOATS_PER_TEXTURED     9
#define FLOATS_PER_COLOURED     7

namespace CEGUI
{
	Urho3DGeometryBuffer::Urho3DGeometryBuffer(Urho3DRenderer& owner, Urho3D::Graphics& rs, CEGUI::RefCounted<RenderMaterial> renderMaterial)
		: GeometryBuffer(renderMaterial), d_owner(owner), d_graphics(rs), d_matrix(1.0)
	{
		initialiseVertexBuffers();
	}

	//----------------------------------------------------------------------------//
	Urho3DGeometryBuffer::~Urho3DGeometryBuffer()
	{
		deinitialiseOpenGLBuffers();
		//cleanUpVertexAttributes();
	}

	void Urho3DGeometryBuffer::draw() const
	{
		if (d_vertexData.empty())
			return;

		if (d_dataAppended)
			syncVertexData();

		if (d_vertexBuffer.Null())
			return;
		
//		CEGUI::Rectf viewPort = d_owner.getActiveViewPort();
		if (d_clippingActive) {
// 			d_glStateChanger->scissor(static_cast<GLint>(d_preparedClippingRegion.left()),
// 				static_cast<GLint>(viewPort.getHeight() - d_preparedClippingRegion.bottom()),
// 				static_cast<GLint>(d_preparedClippingRegion.getWidth()),
// 				static_cast<GLint>(d_preparedClippingRegion.getHeight()));
// 
// 			d_glStateChanger->enable(GL_SCISSOR_TEST);

			Urho3D::IntRect scissor{static_cast<int>(d_preparedClippingRegion.left()),
				static_cast<int>(d_preparedClippingRegion.top()),
				static_cast<int>(d_preparedClippingRegion.right()),
				static_cast<int>(d_preparedClippingRegion.bottom()) };
			d_graphics.SetScissorTest(true, scissor);
		} else {
			//d_glStateChanger->disable(GL_SCISSOR_TEST);
			d_graphics.SetScissorTest(false);
		}
			

		updateMatrix();

		CEGUI::ShaderParameterBindings* shaderParameterBindings = (*d_renderMaterial).getShaderParamBindings();

		// Set the ModelViewProjection matrix in the bindings
		shaderParameterBindings->setParameter("modelViewProjMatrix", d_matrix);
		shaderParameterBindings->setParameter("alphaFactor", d_alpha);

		d_owner.setupRenderingBlendMode(d_blendMode);

		auto shaderWrapper = static_cast<Urho3DShaderWrapper*>(const_cast<ShaderWrapper*>(d_renderMaterial->getShaderWrapper()));

		const int pass_count = d_effect ? d_effect->getPassCount() : 1;
		for (int pass = 0; pass < pass_count; ++pass)
		{
			// set up RenderEffect
			if (d_effect)
				d_effect->performPreRenderFunctions(pass);

			//Prepare for the rendering process according to the used render material

			// Starting with Ogre 2.1 this also activates the PSO
			// which contains all the blend settings etc
			// This is required because the material cannot set shader parameters before
			// the PSO is bound

			//shaderWrapper->setRenderOperation(d_renderOp);
			d_renderMaterial->prepareForRendering();

			// draw the geometry
			//d_renderSystem._render(d_renderOp);
		}

		// clean up RenderEffect
		if (d_effect)
			d_effect->performPostRenderFunctions();

		// Disable clipping after rendering
// 		if (d_clippingActive)
// 		{
// #ifdef CEGUI_USE_OGRE_HLMS
// 			currentViewport->setScissors(previousClipRect.left(), previousClipRect.top(),
// 				previousClipRect.right(), previousClipRect.bottom());
// 			// Restore viewport? d_renderSystem._setViewport(previousViewport);
// #else
// 			d_renderSystem.setScissorTest(false);
// #endif //CEGUI_USE_OGRE_HLMS
// 		}

		updateRenderTargetData(d_owner.getActiveRenderTarget());

	}

	void Urho3DGeometryBuffer::appendGeometry(const float* vertex_data, std::size_t array_size)
	{
		GeometryBuffer::appendGeometry(vertex_data, array_size);
	}

	void Urho3DGeometryBuffer::reset()
	{

	}

	void Urho3DGeometryBuffer::finaliseVertexAttributes(MANUALOBJECT_TYPE type)
	{

	}
	
	void Urho3DGeometryBuffer::initialiseVertexBuffers()
	{
	}

	void Urho3DGeometryBuffer::deinitialiseOpenGLBuffers()
	{

	}
	
	void Urho3DGeometryBuffer::syncVertexData() const
	{
		if (!d_dataAppended)
			return;

		d_dataAppended = false;
	}

	void Urho3DGeometryBuffer::updateMatrix() const
	{
		if (!d_matrixValid || !isRenderTargetDataValid(d_owner.getActiveRenderTarget()))
		{
			// Apply the view projection matrix to the model matrix and save the result as cached matrix
			d_matrix = d_owner.getViewProjectionMatrix() * getModelMatrix();

			d_matrixValid = true;
		}
	}
}