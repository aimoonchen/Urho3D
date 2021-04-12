#include "FairyGUIImpl.h"
#include <map>
#include <vector>
#include "../FairyGUI/UIPackage.h"
#include "../FairyGUI/GComponent.h"
#include "../FairyGUI/GRoot.h"
#include "../Cocos2d/Urho3DContext.h"
#include "../Cocos2d/platform/CCFileUtils.h"
#include "../Cocos2d/platform/GLViewImpl.h"
#include "../Cocos2d/2d/CCScene.h"
#include "../Cocos2d/renderer/CCRenderer.h"
#include "../Core/Context.h"
#include "../Graphics/Graphics.h"
#include "../Resource/ResourceCache.h"
#include "../Input/InputConstants.h"

namespace Urho3D {
	static cocos2d::Size designResolutionSize = cocos2d::Size(1136, 640);
	static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
	static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
	static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

	void FairyGUIImpl::Initialize(Context* context)
	{
		context_ = context;
		SetUrho3DContext(context);
		cocos2d::FileUtils::getInstance()->addSearchPath("D:/Github/FairyGUI-cocos2dx/Examples/Resources");
		context->GetSubsystem<ResourceCache>()->AddResourceDir("D:/Github/FairyGUI-cocos2dx/Examples/Resources");
//  		cocos2d::FileUtils::getInstance()->addSearchPath("C:/GitProjects/Urho3D/bin/Data/FairyGUI/Resources");
//  		context->GetSubsystem<ResourceCache>()->AddResourceDir("C:/GitProjects/Urho3D/bin/Data/FairyGUI/Resources");
// 		cocos2d::FileUtils::getInstance()->addSearchPath("D:/Github/Urho3D/build/bin/Data/FairyGUI/Resources");
// 		context->GetSubsystem<ResourceCache>()->AddResourceDir("D:/Github/Urho3D/build/bin/Data/FairyGUI/Resources");
		
		cocos_renderder_ = new cocos2d::Renderer;
		view_impl_ = new cocos2d::GLViewImpl();
		auto frameSize = context_->GetSubsystem<Urho3D::Graphics>()->GetSize();
		view_impl_->setFrameSize(frameSize.x_, frameSize.y_);
		// Set the design resolution
		view_impl_->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::SHOW_ALL);
		/*auto frameSize = glview->getFrameSize();
		// if the frame's height is larger than the height of medium size.
		if (frameSize.height > mediumResolutionSize.height)
		{
			director->setContentScaleFactor(MIN(largeResolutionSize.height / designResolutionSize.height, largeResolutionSize.width / designResolutionSize.width));
		}
		// if the frame's height is larger than the height of small size.
		else if (frameSize.height > smallResolutionSize.height)
		{
			director->setContentScaleFactor(MIN(mediumResolutionSize.height / designResolutionSize.height, mediumResolutionSize.width / designResolutionSize.width));
		}
		// if the frame's height is smaller than the height of medium size.
		else
		{
			director->setContentScaleFactor(MIN(smallResolutionSize.height / designResolutionSize.height, smallResolutionSize.width / designResolutionSize.width));
		}*/
		cocos2d::Director::getInstance()->setOpenGLView(view_impl_);
		cocos_scene_ = cocos2d::Scene::create();
		fairy_root_ = fairygui::GRoot::create(cocos_scene_);
		cocos2d::Director::getInstance()->runWithScene(cocos_scene_);
	}
	void FairyGUIImpl::Update(float timeStep)
	{

	}
	void FairyGUIImpl::Render()
    {
        context_->GetSubsystem<Urho3D::Graphics>()->SetUIMode(true);
		cocos2d::Director::getInstance()->drawScene();
        context_->GetSubsystem<Urho3D::Graphics>()->SetUIMode(false);
	}

	void FairyGUIImpl::OnWindowSizeChanged()
	{
		fairy_root_->onWindowSizeChanged();
	}
	
	void FairyGUIImpl::OnKey(unsigned int key, bool isPress)
	{
		view_impl_->onGLFWKeyCallback(key, 0, isPress, 0);
	}

	void FairyGUIImpl::OnMouseButtonDown(const IntVector2& windowCursorPos, MouseButton mouseButtons)
	{
		view_impl_->onGLFWMouseCallBack(mouseButtons, true, 0);
	}

	void FairyGUIImpl::OnMouseButtonUp(const IntVector2& windowCursorPos, MouseButton mouseButtons)
	{
		view_impl_->onGLFWMouseCallBack(mouseButtons, false, 0);
	}

	void FairyGUIImpl::OnMouseMove(float x, float y)
	{
		view_impl_->onGLFWMouseMoveCallBack(x, y);
	}
}