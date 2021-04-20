#pragma once
#include "../../EffekseerRendererCommon/EffekseerRenderer.Renderer.h"
#include "EffekseerRendererBGFX.Base.h"

namespace EffekseerRendererBGFX
{
::Effekseer::Backend::GraphicsDeviceRef CreateGraphicsDevice(/*OpenGLDeviceType deviceType, bool isExtensionsEnabled = true*/);

::Effekseer::TextureLoaderRef CreateTextureLoader(
	Effekseer::Backend::GraphicsDeviceRef graphicsDevice,
	::Effekseer::FileInterface* fileInterface = nullptr,
	::Effekseer::ColorSpaceType colorSpaceType = ::Effekseer::ColorSpaceType::Gamma);

::Effekseer::ModelLoaderRef CreateModelLoader(::Effekseer::FileInterface* fileInterface = nullptr/*, OpenGLDeviceType deviceType = OpenGLDeviceType::OpenGL2*/);

::Effekseer::MaterialLoaderRef CreateMaterialLoader(Effekseer::Backend::GraphicsDeviceRef graphicsDevice,
	::Effekseer::FileInterface* fileInterface = nullptr);

Effekseer::Backend::TextureRef CreateTexture(Effekseer::Backend::GraphicsDeviceRef graphicsDevice,
	bgfx::TextureHandle buffer, bool hasMipmap, const std::function<void()>& onDisposed);

struct bgfx_context
{
	bgfx::ProgramHandle program_;
	//bgfx::VertexLayout* vertex_layout_;
    unsigned int vertex_layout_;
	std::unordered_map<std::string, bgfx::UniformHandle> uniforms_;
};

class Renderer;
using RendererRef = ::Effekseer::RefPtr<Renderer>;

class Renderer : public ::EffekseerRenderer::Renderer
{
protected:
	Renderer(){}
	virtual ~Renderer() {}
public:
	static std::vector<bgfx_context> s_bgfx_context_;
	static RendererRef Create(int32_t squareMaxCount/*, OpenGLDeviceType deviceType = OpenGLDeviceType::OpenGL2, bool isExtensionsEnabled = true*/);
	static RendererRef Create(Effekseer::Backend::GraphicsDeviceRef graphicsDevice, int32_t squareMaxCount);
	virtual int32_t GetSquareMaxCount() const = 0;
	virtual void SetSquareMaxCount(int32_t count) = 0;
	virtual void SetBackground(bgfx::TextureHandle background, bool hasMipmap = false) = 0;
	virtual bool IsVertexArrayObjectSupported() const = 0;
};

} // namespace EffekseerRendererBGFX
