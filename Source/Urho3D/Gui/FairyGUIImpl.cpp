#include "FairyGUIImpl.h"
#include "../FairyGUI/UIPackage.h"
#include "../FairyGUI/GComponent.h"
#include "../FairyGUI/GRoot.h"
#include "../Cocos2d/Urho3DContext.h"
#include "../Cocos2d/platform/CCFileUtils.h"
#include "../Cocos2d/2d/CCScene.h"
#include "../Cocos2d/renderer/CCRenderer.h"
#include "../Core/Context.h"
#include "../Resource/ResourceCache.h"
namespace Urho3D {
	void FairyGUIImpl::Initialize(Context* context)
	{
		context_ = context;
		SetUrho3DContext(context);
//  		cocos2d::FileUtils::getInstance()->addSearchPath("C:/GitProjects/Urho3D/build/bin/Data/FairyGUI/Resources");
//  		context->GetSubsystem<ResourceCache>()->AddResourceDir("C:/GitProjects/Urho3D/build/bin/Data/FairyGUI/Resources");
		cocos2d::FileUtils::getInstance()->addSearchPath("D:/Github/Urho3D/build/bin/Data/FairyGUI/Resources");
		context->GetSubsystem<ResourceCache>()->AddResourceDir("D:/Github/Urho3D/build/bin/Data/FairyGUI/Resources");
		cocos_scene_ = cocos2d::Scene::create();
		cocos_renderder_ = new cocos2d::Renderer;
		fairy_root_ = fairygui::GRoot::create(cocos_scene_);
		cocos2d::Director::getInstance()->runWithScene(cocos_scene_);
	}
	void FairyGUIImpl::Update(float timeStep)
	{

	}
	void FairyGUIImpl::Render()
	{
		//cocos_scene_->render(cocos_renderder_, cocos2d::Mat4::IDENTITY, nullptr);
		cocos2d::Director::getInstance()->drawScene();
	}

	void FairyGUIImpl::OnWindowSizeChanged()
	{
		fairy_root_->onWindowSizeChanged();
	}
}