#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "base/ccTypes.h"
#include "../../Math/StringHash.h"

namespace Urho3D
{
    class Texture2D;
    class ShaderVariation;
    class Graphics;
}

NS_CC_BEGIN
class Director;
class GLProgram
{
    friend class GLProgramState;
public:
    /**
@name Built Shader types
@{
*/
/** ETC1 ALPHA supports for 2d */
    static const char* SHADER_NAME_ETC1AS_POSITION_TEXTURE_COLOR;
    static const char* SHADER_NAME_ETC1AS_POSITION_TEXTURE_COLOR_NO_MVP;

    static const char* SHADER_NAME_ETC1AS_POSITION_TEXTURE_GRAY;
    static const char* SHADER_NAME_ETC1AS_POSITION_TEXTURE_GRAY_NO_MVP;

    /**Built in shader for 2d. Support Position, Texture and Color vertex attribute.*/
    static const char* SHADER_NAME_POSITION_TEXTURE_COLOR;
    /**Built in shader for 2d. Support Position, Texture and Color vertex attribute, but without multiply vertex by MVP matrix.*/
    static const char* SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP;
    /**Built in shader for 2d. Support Position, Texture vertex attribute, but include alpha test.*/
    static const char* SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST;
    /**Built in shader for 2d. Support Position, Texture and Color vertex attribute, include alpha test and without multiply vertex by MVP matrix.*/
    static const char* SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV;
    /**Built in shader for 2d. Support Position, Color vertex attribute.*/
    static const char* SHADER_NAME_POSITION_COLOR;
    /**Built in shader for 2d. Support Position, Color, Texture vertex attribute. texture coordinate will used as point size.*/
    static const char* SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE;
    /**Built in shader for 2d. Support Position, Color vertex attribute, without multiply vertex by MVP matrix.*/
    static const char* SHADER_NAME_POSITION_COLOR_NO_MVP;
    /**Built in shader for 2d. Support Position, Texture vertex attribute.*/
    static const char* SHADER_NAME_POSITION_TEXTURE;
    /**Built in shader for 2d. Support Position, Texture vertex attribute. with a specified uniform as color*/
    static const char* SHADER_NAME_POSITION_TEXTURE_U_COLOR;
    /**Built in shader for 2d. Support Position, Texture and Color vertex attribute. but alpha will be the multiplication of color attribute and texture.*/
    static const char* SHADER_NAME_POSITION_TEXTURE_A8_COLOR;
    /**Built in shader for 2d. Support Position, with color specified by a uniform.*/
    static const char* SHADER_NAME_POSITION_U_COLOR;
    /**Built in shader for draw a sector with 90 degrees with center at bottom left point.*/
    static const char* SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR;

    /**Built in shader for ui effects */
    static const char* SHADER_NAME_POSITION_GRAYSCALE;
    /** @{
        Built in shader for label and label with effects.
    */
    static const char* SHADER_NAME_LABEL_NORMAL;
    static const char* SHADER_NAME_LABEL_OUTLINE;
    static const char* SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL;
    static const char* SHADER_NAME_LABEL_DISTANCEFIELD_GLOW;
};

class GLProgramState
{
public:
    GLProgramState(Urho3D::ShaderVariation* vs, Urho3D::ShaderVariation* fs);
    ~GLProgramState() = default;
    static GLProgramState* getOrCreateWithGLProgramName(const std::string& glProgramName);
    static GLProgramState* getOrCreateWithGLProgramName(const std::string& glProgramName, Urho3D::Texture2D* texture);
    void apply(const Mat4& modelView);
    void apply();
    void setUniformsForBuiltins(const Mat4& modelView);
private:
    Urho3D::ShaderVariation* vs_{ nullptr };
    Urho3D::ShaderVariation* fs_{ nullptr };
    Urho3D::Graphics* graphics_{ nullptr };
    Director* _director{ nullptr };
    Urho3D::StringHash modelViewName_{ "CC_MVMatrix" };
    Urho3D::StringHash projMatName_{ "CC_PMatrix" };
    Urho3D::StringHash modelViewProjName_{ "CC_MVPMatrix" };
    static std::unordered_map<std::string, std::shared_ptr<GLProgramState>> programs_;
};

NS_CC_END