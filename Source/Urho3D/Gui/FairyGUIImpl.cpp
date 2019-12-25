#include "FairyGUIImpl.h"
#include "../FairyGUI/UIPackage.h"
#include "../FairyGUI/GComponent.h"
#include "../FairyGUI/GRoot.h"
#include "../Cocos2d/Urho3DContext.h"
#include "../Cocos2d/platform/CCFileUtils.h"
#include "../Cocos2d/2d/CCScene.h"
#include "../Cocos2d/renderer/CCRenderer.h"
namespace Urho3D {
	void FairyGUIImpl::Initialize(Context* context)
	{
		cocos_scene_ = cocos2d::Scene::create();
		cocos_renderder_ = new cocos2d::Renderer;
		fairy_root_ = fairygui::GRoot::create(cocos_scene_);
	}
	void FairyGUIImpl::Update(float timeStep)
	{

	}
	void FairyGUIImpl::Render()
	{

	}
}