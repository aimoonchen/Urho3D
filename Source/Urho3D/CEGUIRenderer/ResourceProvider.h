#pragma once
#include "CEGUI/DefaultResourceProvider.h"

#include "Urho3D/Urho3D.h"

namespace CEGUI
{
	class URHO3D_API Urho3DResourceProvider : public DefaultResourceProvider
	{
	public:
		Urho3DResourceProvider();

		void loadRawDataContainer(const String& filename, RawDataContainer& output, const String& resourceGroup);
		void unloadRawDataContainer(RawDataContainer& output);
		size_t getResourceGroupFileNames(std::vector<String>& out_vec, const String& file_pattern, const String& resource_group);

		String getFilePath(const String& filename, const String& resourceGroup);
	};
}