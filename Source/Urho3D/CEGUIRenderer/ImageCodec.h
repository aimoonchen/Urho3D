#pragma once
#include "CEGUI/ImageCodec.h"
#include "Urho3D/Urho3D.h"
namespace CEGUI
{
	class URHO3D_API Urho3DImageCodec : public ImageCodec
	{
	public:
		Urho3DImageCodec();
		void setImageFileDataType(const String& type);
		const String& getImageFileDataType() const;
		Texture* load(const RawDataContainer& data, Texture* result);
	protected:
		String d_dataTypeID;
	};
}