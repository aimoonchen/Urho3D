#include "GeometryBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Graphics/Graphics.h"

#include "CEGUI/Vertex.h"
#include "CEGUI/RenderEffect.h"
#include "CEGUI/Exceptions.h"
#include "CEGUI/ShaderParameterBindings.h"

#include "Texture.h"
#include "ShaderWrapper.h"

#define FLOATS_PER_TEXTURED     9
#define FLOATS_PER_COLOURED     7

static const unsigned UI_VERTEX_SIZE = 6;

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
		

		d_graphics.ClearParameterSources();
		d_graphics.SetColorWrite(true);

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

		Urho3D::RenderSurface* surface = d_graphics.GetRenderTarget(0);
#ifdef URHO3D_OPENGL
		// Reverse winding if rendering to texture on OpenGL
		if (surface)
			d_graphics.SetCullMode(Urho3D::CULL_CW);
		else
#endif
		//d_graphics.SetCullMode(Urho3D::CULL_CCW);
		d_graphics.SetCullMode(Urho3D::CULL_CW);

		d_graphics.SetDepthTest(Urho3D::CMP_ALWAYS);
		d_graphics.SetDepthWrite(false);
		d_graphics.SetFillMode(Urho3D::FILL_SOLID);
		d_graphics.SetStencilTest(false);
		d_graphics.SetVertexBuffer(d_vertexBuffer);

		auto shaderWrapper = static_cast<Urho3DShaderWrapper*>(const_cast<ShaderWrapper*>(d_renderMaterial->getShaderWrapper()));

		const int pass_count = d_effect ? d_effect->getPassCount() : 1;
		for (int pass = 0; pass < pass_count; ++pass)
		{
			if (d_effect)
				d_effect->performPreRenderFunctions(pass);
			d_renderMaterial->prepareForRendering();
			d_graphics.Draw(Urho3D::TRIANGLE_LIST, d_verticesVBOPosition, d_vertexCount);
		}

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
		auto cegui_vert_size = GeometryBuffer::getVertexAttributeElementCount();
		auto vertcount = array_size / cegui_vert_size;
		const float *psrc = vertex_data;
		std::vector<float> temp_data;
		auto urho3d_vert_size = getVertexAttributeElementCount();
		temp_data.resize(vertcount * urho3d_vert_size);
		float *pdst = &temp_data[0];
		for (size_t i = 0; i < vertcount; i++) {
			pdst[0] = psrc[0];
			pdst[1] = psrc[1];
			pdst[2] = psrc[2];
			//
			((unsigned&)pdst[3]) = Urho3D::Color{ psrc[3], psrc[4], psrc[5], psrc[6]}.ToUInt();
			//
			if (d_vertexAttributes.size() == 3) {
				pdst[4] = psrc[7];
				pdst[5] = psrc[8];
			}
			pdst += urho3d_vert_size;
			psrc += cegui_vert_size;
		}
		GeometryBuffer::appendGeometry(&temp_data[0], temp_data.size());
		d_dataAppended = true;
	}
	
	int Urho3DGeometryBuffer::getVertexAttributeElementCount() const
	{
		int count = 0;

		const unsigned int attribute_count = d_vertexAttributes.size();
		for (unsigned int i = 0; i < attribute_count; ++i)
		{
			switch (d_vertexAttributes.at(i))
			{
			case VertexAttributeType::Position0:
				count += 3;
				break;
			case VertexAttributeType::Colour0:
				count += 1;
				break;
			case VertexAttributeType::TexCoord0:
				count += 2;
				break;
			default:
				break;
			}
		}

		return count;
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

		if (d_vertexBuffer.Null()) {
			d_vertexBuffer = new Urho3D::VertexBuffer(d_graphics.GetContext());
		}

		if (d_vertexData.empty())
			return;

		// Update quad geometry into the vertex buffer
		// Resize the vertex buffer first if too small or much too large
		unsigned numVertices = d_vertexData.size() / UI_VERTEX_SIZE;
		if (d_vertexBuffer->GetVertexCount() < numVertices || d_vertexBuffer->GetVertexCount() > numVertices * 2)
			d_vertexBuffer->SetSize(numVertices, Urho3D::MASK_POSITION | Urho3D::MASK_COLOR | Urho3D::MASK_TEXCOORD1, true);

		d_vertexBuffer->SetData(&d_vertexData[0]);

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