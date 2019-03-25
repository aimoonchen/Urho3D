#include "ShaderWrapper.h"
#include "CEGUI/ShaderParameterBindings.h"
#include "CEGUI/Exceptions.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Core/Timer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Texture2D.h"
#include "Texture.h"

namespace CEGUI
{
	Urho3DShaderWrapper::Urho3DShaderWrapper(Urho3DRenderer& owner, Urho3D::Graphics& rs,
		Urho3D::ShaderVariation* vs, Urho3D::ShaderVariation* ps)
		: d_owner{ owner }, d_graphics{ rs }, d_vertexShader{ vs }, d_pixelShader{ ps }
	{

	}

	Urho3DShaderWrapper::~Urho3DShaderWrapper()
	{

	}

	void Urho3DShaderWrapper::prepareForRendering(const ShaderParameterBindings* shaderParameterBindings)
	{
		static const auto& context = d_graphics.GetContext();
		d_graphics.SetShaders(d_vertexShader, d_pixelShader);
		const auto& shader_parameter_bindings = shaderParameterBindings->getShaderParameterBindings();
		auto iter = shader_parameter_bindings.begin();
		auto end = shader_parameter_bindings.end();

		while (iter != end) {
			const CEGUI::ShaderParameter* parameter = iter->second;
			if (parameter->getType() != ShaderParamType::Texture) {
				auto found_iterator = d_shaderParameterStates.find(iter->first);
				if (found_iterator != d_shaderParameterStates.end()) {
					ShaderParameter* last_shader_parameter = found_iterator->second;
					if (parameter->equal(last_shader_parameter)) {
						++iter;
						continue;
					} else {
						if (parameter->getType() == last_shader_parameter->getType()) {
							last_shader_parameter->takeOverParameterValue(parameter);
						} else {
							delete found_iterator->second;
							found_iterator->second = parameter->clone();
						}
					}
				} else {
					d_shaderParameterStates[iter->first] = parameter->clone();
				}
			}

			//const GLint& location = d_uniformVariables.find(iter->first)->second;

			CEGUI::ShaderParamType parameter_type = parameter->getType();

			switch (parameter_type) {
			case ShaderParamType::Int: {
				auto parameterInt = static_cast<const CEGUI::ShaderParameterInt*>(parameter);
				//glUniform1i(location, parameterInt->d_parameterValue);
			}
			break;
			case ShaderParamType::Float: {
				auto parameterFloat = static_cast<const CEGUI::ShaderParameterFloat*>(parameter);
				//glUniform1f(location, parameterFloat->d_parameterValue);
			}
			break;
			case ShaderParamType::Matrix4X4: {
				auto parameterMatrix = static_cast<const CEGUI::ShaderParameterMatrix*>(parameter);
				//glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(parameterMatrix->d_parameterValue));
			}
			break;
			case ShaderParamType::Texture: {
				auto parameterTexture = static_cast<const CEGUI::ShaderParameterTexture*>(parameter);
				auto urho3DTexture = static_cast<const CEGUI::Urho3DTexture*>(parameterTexture->d_parameterValue);
				d_graphics.SetTexture(0, urho3DTexture->getUrho3DTexture());
// 				d_glStateChangeWrapper->activeTexture(location);
// 				d_glStateChangeWrapper->bindTexture(GL_TEXTURE_2D, urho3DTexture->getOpenGLTexture());
			}
			break;
			default:
				break;
			}

			++iter;
		}

		float elapsedTime = context->GetSubsystem<Urho3D::Time>()->GetElapsedTime();
		d_graphics.SetShaderParameter(Urho3D::VSP_ELAPSEDTIME, elapsedTime);
		d_graphics.SetShaderParameter(Urho3D::PSP_ELAPSEDTIME, elapsedTime);
	}
}