#include "ResourceProvider.h"
#include "CEGUI/Exceptions.h"
namespace CEGUI
{
	Urho3DResourceProvider::Urho3DResourceProvider()
	{
		// set deafult resource group for Ogre
		//d_defaultResourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME.c_str();
	}

	//----------------------------------------------------------------------------//
	void Urho3DResourceProvider::loadRawDataContainer(const String& filename, RawDataContainer& output, const String& resourceGroup)
	{
		String final_filename(getFinalFilename(filename, resourceGroup));

// 		unsigned char* mem = new unsigned char[memBuffSize];
// 		memcpy(mem, buf.c_str(), memBuffSize);
// 
// 		output.setData(mem);
// 		output.setSize(memBuffSize);
// 		String orpGroup;
// 		if (resourceGroup.empty())
// 			orpGroup = d_defaultResourceGroup.empty() ?
// 			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME.c_str() :
// 			d_defaultResourceGroup;
// 		else
// 			orpGroup = resourceGroup;
// 
// #if (CEGUI_STRING_CLASS != CEGUI_STRING_CLASS_UTF_32) 
// 		Ogre::DataStreamPtr input = Ogre::ResourceGroupManager::getSingleton().
// 			openResource(filename.c_str(), orpGroup.c_str());
// #else
// 		Ogre::DataStreamPtr input = Ogre::ResourceGroupManager::getSingleton().
// 			openResource(String::convertUtf32ToUtf8(filename.getString()).c_str(), String::convertUtf32ToUtf8(orpGroup.getString()).c_str());
// #endif
// 
// 		if (input.isNull())
// 			throw InvalidRequestException(
// 				"Unable to open resource file '" + filename +
// 				"' in resource group '" + orpGroup + "'.");
// 
// 		Ogre::String buf = input->getAsString();
// 		const size_t memBuffSize = buf.length();
// 
// 		unsigned char* mem = new unsigned char[memBuffSize];
// 		memcpy(mem, buf.c_str(), memBuffSize);
// 
// 		output.setData(mem);
// 		output.setSize(memBuffSize);
	}

	//----------------------------------------------------------------------------//
	void Urho3DResourceProvider::unloadRawDataContainer(RawDataContainer& data)
	{
		if (data.getDataPtr()) {
			delete[] data.getDataPtr();
			data.setData(0);
			data.setSize(0);
		}
	}

	//----------------------------------------------------------------------------//
	size_t Urho3DResourceProvider::getResourceGroupFileNames(std::vector<String>& out_vec, const String& file_pattern, const String& resource_group)
	{
		return 0;
		// get list of files in the group that match the pattern.
// #if (CEGUI_STRING_CLASS != CEGUI_STRING_CLASS_UTF_32) 
// 		Ogre::StringVectorPtr vp =
// 			Ogre::ResourceGroupManager::getSingleton().findResourceNames(
// 			(resource_group.empty() ?
// 				d_defaultResourceGroup.c_str() :
// 				resource_group.c_str()),
// 				file_pattern.c_str());
// #else
// 		Ogre::StringVectorPtr vp =
// 			Ogre::ResourceGroupManager::getSingleton().findResourceNames(
// 			(resource_group.empty() ?
// 				String::convertUtf32ToUtf8(d_defaultResourceGroup.getString()).c_str() :
// 				String::convertUtf32ToUtf8(resource_group.getString()).c_str()),
// 				String::convertUtf32ToUtf8(file_pattern.getString()).c_str());
// #endif
// 
// 		size_t entries = 0;
// 		Ogre::StringVector::iterator i = vp->begin();
// 		for (; i != vp->end(); ++i)
// 		{
// 			out_vec.push_back(i->c_str());
// 			++entries;
// 		}
// 
// 		return entries;
	}

	String Urho3DResourceProvider::getFilePath(const String& filename, const String& resourceGroup)
	{
		return getFinalFilename(filename, resourceGroup);
	}
}