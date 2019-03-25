#pragma once
#include <string>
#include <map>
#include "Renderer.h"
#include "CEGUI/ShaderWrapper.h"

#include "glm/glm.hpp"

#include "Urho3D/Urho3D.h"
#include "Urho3D/Container/Ptr.h"

namespace Urho3D
{
	class Graphics;
	class ShaderVariation;
}

namespace CEGUI
{
	class ShaderParameter;
	class Urho3DRenderer;

	class URHO3D_API Urho3DShaderWrapper : public ShaderWrapper
	{
	public:
		Urho3DShaderWrapper(Urho3DRenderer& owner, Urho3D::Graphics& rs, Urho3D::ShaderVariation* vs, Urho3D::ShaderVariation* ps);
		~Urho3DShaderWrapper();
		void prepareForRendering(const ShaderParameterBindings* shaderParameterBindings) override;
	private:
		Urho3DRenderer& d_owner;
		Urho3D::Graphics& d_graphics;
		Urho3D::ShaderVariation* d_vertexShader{ nullptr };
		Urho3D::ShaderVariation* d_pixelShader{ nullptr };
		std::map<std::string, ShaderParameter*> d_shaderParameterStates;
	};
}