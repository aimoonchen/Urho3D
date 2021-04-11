#include "Texture2DUtils.h"
#include "base/ccMacros.h"
#include "base/CCDirector.h"
#include "platform/CCDevice.h"
#include "Urho3DContext.h"
#include "../../Graphics/Texture2D.h"
#include "../../Core/Context.h"
#include "../../Graphics/Graphics.h"
#include "../../Resource/ResourceCache.h"
NS_CC_BEGIN

//static Texture2D::PixelFormat g_defaultAlphaPixelFormat = Texture2D::PixelFormat::DEFAULT;

bool InitWithString(Urho3D::Texture2D* texture, const char *text, const std::string& fontName, float fontSize, const Size& dimensions/* = Size(0, 0)*/, TextHAlignment hAlignment/* =  TextHAlignment::CENTER */, TextVAlignment vAlignment/* =  TextVAlignment::TOP */, bool enableWrap /* = false */, int overflow /* = 0 */)
{
	FontDefinition tempDef;

	tempDef._shadow._shadowEnabled = false;
	tempDef._stroke._strokeEnabled = false;


	tempDef._fontName = fontName;
	tempDef._fontSize = fontSize;
	tempDef._dimensions = dimensions;
	tempDef._alignment = hAlignment;
	tempDef._vertAlignment = vAlignment;
	tempDef._fontFillColor = Color3B::WHITE;
	tempDef._enableWrap = enableWrap;
	tempDef._overflow = overflow;

	return InitWithString(texture, text, tempDef);
}

bool InitWithString(Urho3D::Texture2D* texture, const char *text, const FontDefinition& textDefinition)
{
	if (!text || 0 == strlen(text))
	{
		return false;
	}

#if CC_ENABLE_CACHE_TEXTURE_DATA
	// cache the texture data
	VolatileTextureMgr::addStringTexture(this, text, textDefinition);
#endif

	bool ret = false;
	Device::TextAlign align;

	if (TextVAlignment::TOP == textDefinition._vertAlignment)
	{
		align = (TextHAlignment::CENTER == textDefinition._alignment) ? Device::TextAlign::TOP
			: (TextHAlignment::LEFT == textDefinition._alignment) ? Device::TextAlign::TOP_LEFT : Device::TextAlign::TOP_RIGHT;
	}
	else if (TextVAlignment::CENTER == textDefinition._vertAlignment)
	{
		align = (TextHAlignment::CENTER == textDefinition._alignment) ? Device::TextAlign::CENTER
			: (TextHAlignment::LEFT == textDefinition._alignment) ? Device::TextAlign::LEFT : Device::TextAlign::RIGHT;
	}
	else if (TextVAlignment::BOTTOM == textDefinition._vertAlignment)
	{
		align = (TextHAlignment::CENTER == textDefinition._alignment) ? Device::TextAlign::BOTTOM
			: (TextHAlignment::LEFT == textDefinition._alignment) ? Device::TextAlign::BOTTOM_LEFT : Device::TextAlign::BOTTOM_RIGHT;
	}
	else
	{
		CCASSERT(false, "Not supported alignment format!");
		return false;
	}

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID) && (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
	CCASSERT(textDefinition._stroke._strokeEnabled == false, "Currently stroke only supported on iOS and Android!");
#endif

	//Texture2D::PixelFormat pixelFormat = g_defaultAlphaPixelFormat;
	unsigned char* outTempData = nullptr;
	ssize_t outTempDataLen = 0;

	int imageWidth;
	int imageHeight;
	auto textDef = textDefinition;
	auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();
	textDef._fontSize *= contentScaleFactor;
	textDef._lineSpacing *= contentScaleFactor;
	textDef._dimensions.width *= contentScaleFactor;
	textDef._dimensions.height *= contentScaleFactor;
	textDef._stroke._strokeSize *= contentScaleFactor;
	textDef._shadow._shadowEnabled = false;

	bool hasPremultipliedAlpha;
	Data outData = Device::getTextureDataForText(text, textDef, align, imageWidth, imageHeight, hasPremultipliedAlpha);
	if (outData.isNull())
	{
		return false;
	}

	Size  imageSize = Size((float)imageWidth, (float)imageHeight);
	//pixelFormat = Texture2D::convertDataToFormat(outData.getBytes(), imageWidth*imageHeight * 4, Texture2D::PixelFormat::RGBA8888, pixelFormat, &outTempData, &outTempDataLen);
	outTempData = outData.getBytes();
	outTempDataLen = imageWidth * imageHeight * 4;
	//ret = initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);
	texture->SetNumLevels(1);
	texture->SetSize(imageWidth, imageHeight, Urho3D::Graphics::GetRGBAFormat());
	ret = texture->SetData(0, 0, 0, imageWidth, imageHeight, outTempData);
	if (outTempData != nullptr && outTempData != outData.getBytes()) {
		free(outTempData);
	}
	//_hasPremultipliedAlpha = hasPremultipliedAlpha;

	return ret;
}

CC_DLL Urho3D::Texture2D* GetUrho3DTexture(const std::string& path)
{
	auto cache = GetUrho3DContext()->GetSubsystem<Urho3D::ResourceCache>();
	return cache->GetResource<Urho3D::Texture2D>(path.c_str());
}

NS_CC_END