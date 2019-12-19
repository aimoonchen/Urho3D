#pragma once

#include <string>
#include "base/ccTypes.h"
#include "math/CCGeometry.h"
namespace Urho3D
{
	class Texture2D;
}
NS_CC_BEGIN
bool InitWithString(Urho3D::Texture2D* texture, const char *text, const std::string &fontName, float fontSize, const Size& dimensions = Size(0, 0), TextHAlignment hAlignment = TextHAlignment::CENTER, TextVAlignment vAlignment = TextVAlignment::TOP, bool enableWrap = true, int overflow = 0);
bool InitWithString(Urho3D::Texture2D* texture, const char *text, const FontDefinition& textDefinition);
CC_DLL Urho3D::Texture2D* GetUrho3DTexture(const std::string& path);
NS_CC_END