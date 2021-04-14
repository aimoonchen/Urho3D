#include "CCGLProgramState.h"
#include "base/CCDirector.h"
#include "../../Core/Context.h"
#include "../../IO/Log.h"
#include "../../Graphics/Graphics.h"
#include "../../Resource/ResourceCache.h"
#include "../../Graphics/Texture2D.h"
#include "Urho3DContext.h"

NS_CC_BEGIN

const char* GLProgram::SHADER_NAME_ETC1AS_POSITION_TEXTURE_COLOR = "#ShaderETC1ASPositionTextureColor";
const char* GLProgram::SHADER_NAME_ETC1AS_POSITION_TEXTURE_COLOR_NO_MVP = "#ShaderETC1ASPositionTextureColor_noMVP";

const char* GLProgram::SHADER_NAME_ETC1AS_POSITION_TEXTURE_GRAY = "#ShaderETC1ASPositionTextureGray";
const char* GLProgram::SHADER_NAME_ETC1AS_POSITION_TEXTURE_GRAY_NO_MVP = "#ShaderETC1ASPositionTextureGray_noMVP";

const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR = "ShaderPositionTextureColor";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP = "ShaderPositionTextureColor_noMVP";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST = "ShaderPositionTextureColorAlphaTest";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV = "ShaderPositionTextureColorAlphaTest_NoMV";
const char* GLProgram::SHADER_NAME_POSITION_COLOR = "ShaderPositionColor";
const char* GLProgram::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE = "ShaderPositionColorTexAsPointsize";
const char* GLProgram::SHADER_NAME_POSITION_COLOR_NO_MVP = "ShaderPositionColor_noMVP";

const char* GLProgram::SHADER_NAME_POSITION_TEXTURE = "ShaderPositionTexture";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_U_COLOR = "ShaderPositionTexture_uColor";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_A8_COLOR = "ShaderPositionTextureA8Color";
const char* GLProgram::SHADER_NAME_POSITION_U_COLOR = "ShaderPosition_uColor";
const char* GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR = "ShaderPositionLengthTextureColor";
const char* GLProgram::SHADER_NAME_POSITION_GRAYSCALE = "ShaderUIGrayScale";
const char* GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL = "ShaderLabelDFNormal";
const char* GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW = "ShaderLabelDFGlow";
const char* GLProgram::SHADER_NAME_LABEL_NORMAL = "ShaderLabelNormal";
const char* GLProgram::SHADER_NAME_LABEL_OUTLINE = "ShaderLabelOutline";

std::unordered_map<std::string, std::shared_ptr<GLProgramState>> GLProgramState::programs_;

GLProgramState::GLProgramState(Urho3D::ShaderVariation* vs, Urho3D::ShaderVariation* fs)
    : vs_{vs}
    , fs_{fs}
{
    graphics_ = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>();
    _director = Director::getInstance();
}

void GLProgramState::setUniformsForBuiltins(const Mat4& modelView)
{
    const auto& matrixP = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    if (graphics_->HasShaderParameter(modelViewName_))
    {
        Mat4 tm = modelView;
        tm.transpose();
        Urho3D::Matrix4 urho3dMV(tm.m);
        graphics_->SetShaderParameter(modelViewName_, urho3dMV);
    }
    if (graphics_->HasShaderParameter(modelViewProjName_))
    {
        Mat4 matrixMVP = matrixP * modelView;
        matrixMVP.transpose();
        Urho3D::Matrix4 urho3dMVP(matrixMVP.m);
        graphics_->SetShaderParameter(modelViewProjName_, urho3dMVP);
    }
}

void GLProgramState::apply(const Mat4& modelView)
{
    graphics_->SetShaders(vs_, fs_);
    setUniformsForBuiltins(modelView);
}

void GLProgramState::apply()
{
    graphics_->SetShaders(vs_, fs_);
}

GLProgramState* GLProgramState::getOrCreateWithGLProgramName(const std::string& glProgramName, Urho3D::Texture2D* texture)
{
    if (texture != nullptr && false/*texture->getAlphaTextureName() != 0*/)
    {
        if (glProgramName == GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR)
        {
            return GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_ETC1AS_POSITION_TEXTURE_COLOR);
        }
        else if (glProgramName == GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP)
        {
            return GLProgramState::getOrCreateWithGLProgramName(
                GLProgram::SHADER_NAME_ETC1AS_POSITION_TEXTURE_COLOR_NO_MVP);
        }
        else if (glProgramName == GLProgram::SHADER_NAME_POSITION_GRAYSCALE)
        {
            return GLProgramState::getOrCreateWithGLProgramName(
                GLProgram::SHADER_NAME_ETC1AS_POSITION_TEXTURE_GRAY_NO_MVP);
        }
    }

    return GLProgramState::getOrCreateWithGLProgramName(glProgramName);
}

GLProgramState* GLProgramState::getOrCreateWithGLProgramName(const std::string& glProgramName)
{
    std::string fileName = "FairyGUI/" + glProgramName + ".sc";
    auto it = programs_.find(fileName);
    if (it != programs_.end()) {
        return it->second.get();
    }
    auto graphics = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>();
    auto vs = graphics->GetShader(Urho3D::VS, fileName.c_str(), "");
    if (!vs) {
        URHO3D_LOGERRORF("CompileShader vs %s Failed.", fileName.c_str());
        return nullptr;
    }
    auto fs = graphics->GetShader(Urho3D::PS, fileName.c_str(), "");
    if (!fs) {
        URHO3D_LOGERRORF("CompileShader fs %s Failed.", fileName.c_str());
        return nullptr;
    }
    programs_[fileName] = std::make_shared<GLProgramState>(vs, fs);
    return programs_[fileName].get();
}


NS_CC_END