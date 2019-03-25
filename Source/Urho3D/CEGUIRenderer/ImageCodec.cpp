#include "ImageCodec.h"
#include "Texture.h"

namespace CEGUI
{
	Urho3DImageCodec::Urho3DImageCodec()
		: ImageCodec("Urho3DImageCodec - Integrated ImageCodec using the Urho3D engine.")
	{
	}

	//----------------------------------------------------------------------------//
	void Urho3DImageCodec::setImageFileDataType(const String& type)
	{
		d_dataTypeID = type;
	}

	//----------------------------------------------------------------------------//
	const String& Urho3DImageCodec::getImageFileDataType() const
	{
		return d_dataTypeID;
	}

	//----------------------------------------------------------------------------//
	Texture* Urho3DImageCodec::load(const RawDataContainer& data, Texture* result)
	{
		return nullptr;
// 		using namespace Urho3D;
// 
// 		// wrap the buffer of the RawDataContainer with an Ogre::MemoryDataStream.
// 		DataStreamPtr stream(
// 			OGRE_NEW MemoryDataStream(
// 				const_cast<void*>(static_cast<const void*>(data.getDataPtr())),
// 				data.getSize(), false));
// 
// 		// load the image
// 		Ogre::Image image;
// #if (CEGUI_STRING_CLASS != CEGUI_STRING_CLASS_UTF_32) 
// 		image.load(stream, d_dataTypeID.c_str());
// #else
// 		image.load(stream, String::convertUtf32ToUtf8(d_dataTypeID.getString()).c_str());
// #endif
// 
// 		const PixelFormat ogre_pf = image.getFormat();
// 		const Texture::PixelFormat cegui_pf =
// 			Urho3DTexture::fromOgrePixelFormat(ogre_pf);
// 
// 		// discover the pixel format and number of pixel components
// 		int components;
// 		bool rbswap;
// 		switch (ogre_pf)
// 		{
// 		case PF_R8G8B8:
// 			rbswap = true;
// 			components = 3;
// 			break;
// 
// 		case PF_A8R8G8B8:
// 			rbswap = true;
// 			components = 4;
// 			break;
// 
// 		default:
// 			rbswap = false;
// 			break;
// 		}
// 
// 		// do the old switcharoo on R and B if needed
// 		if (rbswap)
// 		{
// 			std::uint8_t* dat = image.getData();
// 			for (unsigned int j = 0; j < image.getHeight(); ++j)
// 			{
// 				for (unsigned int i = 0; i < image.getWidth(); ++i)
// 				{
// 					std::uint8_t tmp = dat[i * components + 0];
// 					dat[i * components + 0] = dat[i * components + 2];
// 					dat[i * components + 2] = tmp;
// 				}
// 
// 				dat += image.getRowSpan();
// 			}
// 		}
// 
// 		// load the resulting image into the texture
// 		result->loadFromMemory(image.getData(),
// 			Sizef(static_cast<float>(image.getWidth()),
// 				static_cast<float>(image.getHeight())),
// 			cegui_pf);
// 
// 		return result;
	}
}