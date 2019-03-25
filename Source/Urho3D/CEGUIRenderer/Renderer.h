#pragma once
#include "CEGUI/Renderer.h"
#include "CEGUI/Sizef.h"
#include "CEGUI/Config.h"

#include "Urho3D/Urho3D.h"
#include "Urho3D/Container/Ptr.h"
#include <vector>
#include <unordered_map>

namespace Urho3D
{
	class RenderSurface;
	class Matrix4;
	class Texture2D;
	class Graphics;
}

namespace CEGUI
{
	class Urho3DGeometryBuffer;
	class Urho3DTexture;
	class Urho3DResourceProvider;
	class Urho3DImageCodec;
	class Urho3DWindowTarget;
	class Urho3DShaderWrapper;

	class URHO3D_API Urho3DRenderer : public Renderer
	{
	public:
		static Urho3DRenderer& bootstrapSystem(Urho3D::Graphics* graphics, const int abi = CEGUI_VERSION_ABI);
		static Urho3DRenderer& bootstrapSystem(Urho3D::Graphics* graphics, Urho3D::RenderSurface& target, const int abi = CEGUI_VERSION_ABI);
		static void destroySystem();

		static Urho3DRenderer& create(Urho3D::Graphics* graphics, const int abi = CEGUI_VERSION_ABI);
		static Urho3DRenderer& create(Urho3D::Graphics* graphics, Urho3D::RenderSurface& target, const int abi = CEGUI_VERSION_ABI);
		static void destroy(Urho3DRenderer& renderer);

		static Urho3DResourceProvider& createUrho3DResourceProvider();
		static void destroyUrho3DResourceProvider(Urho3DResourceProvider& rp);

		static Urho3DImageCodec& createUrho3DImageCodec();
		static void destroyUrho3DImageCodec(Urho3DImageCodec& ic);

		static glm::mat4 urho3DToGlmMatrix(const Urho3D::Matrix4& matrix);
		static Urho3D::Matrix4 glmToUrho3DMatrix(const glm::mat4& matrix);

		void setRenderingEnabled(const bool enabled);
		bool isRenderingEnabled() const;

		void clearVertexBufferPool();

		// implement Renderer interface
		RenderTarget& getDefaultRenderTarget() override;
		RefCounted<RenderMaterial> createRenderMaterial(const DefaultShaderType shaderType) const override;
		GeometryBuffer& createGeometryBufferTextured(CEGUI::RefCounted<RenderMaterial> renderMaterial) override;
		GeometryBuffer& createGeometryBufferColoured(CEGUI::RefCounted<RenderMaterial> renderMaterial) override;
		TextureTarget* createTextureTarget(bool addStencilBuffer) override;
		void destroyTextureTarget(TextureTarget* target) override;
		void destroyAllTextureTargets() override;
		Texture& createTexture(const String& name) override;
		Texture& createTexture(const String& name, const String& filename, const String& resourceGroup) override;
		Texture& createTexture(const String& name, const Sizef& size) override;
		Texture& createTexture(const String& name, Urho3D::SharedPtr<Urho3D::Texture2D>& tex, bool take_ownership = false);
		void destroyTexture(Texture& texture) override;
		void destroyTexture(const String& name) override;
		void destroyAllTextures() override;
		Texture& getTexture(const String& name) const override;
		bool isTextureDefined(const String& name) const override;
		void setDisplaySize(const Sizef& sz) override;
		const Sizef& getDisplaySize() const override;
		unsigned int getMaxTextureSize() const override;
		const String& getIdentifierString() const override;
		bool isTexCoordSystemFlipped() const override;
		//
		void beginRendering() override;
		void endRendering() override;
		void setupRenderingBlendMode(const BlendMode mode, bool force = false);
	protected:
		Urho3DRenderer(Urho3D::Graphics* graphics);
		//! constructor takin the Ogre::RenderTarget to use as the default root.
		Urho3DRenderer(Urho3D::Graphics* graphics, Urho3D::RenderSurface& target);
		//! destructor.
		virtual ~Urho3DRenderer();
		void checkUrho3DInitialised();
		void throwIfNameExists(const String& name) const;
		static void logTextureCreation(const String& name);
		static void logTextureDestruction(const String& name);
		void constructor_impl(/*Urho3D::RenderSurface& target*/);
		void initialiseShaders();
		void cleanupShaders();
		Urho3D::Graphics* d_graphics{ nullptr };
		//! String holding the renderer identification text.
		static String d_rendererID;
		//! What the renderer considers to be the current display size.
		Sizef d_displaySize;
		//! The default RenderTarget
		RenderTarget* d_defaultTarget;
		using TextureTargetList = std::vector<TextureTarget*>;
		using GeometryBufferList = std::vector<Urho3DGeometryBuffer*>;
		using TextureMap = std::unordered_map<String, Urho3DTexture*>;
		//! Container used to track texture targets.
		TextureTargetList d_textureTargets;
		//! Container used to track geometry buffers.
		GeometryBufferList d_geometryBuffers;
		//! Container used to track textures.
		TextureMap d_textures;
		//! What the renderer thinks the max texture size is.
		unsigned int d_maxTextureSize;
		BlendMode d_activeBlendMode;
		Urho3DShaderWrapper* d_texturedShaderWrapper;
		Urho3DShaderWrapper* d_colouredShaderWrapper;
	};
}