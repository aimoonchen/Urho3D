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
#include "../Input/InputConstants.h"
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
	void FairyGUIImpl::OnMouseButtonDown(const IntVector2& windowCursorPos, MouseButton mouseButtons)
	{
		cocos2d::EventMouse event(cocos2d::EventMouse::MouseEventType::MOUSE_DOWN);
		event.setCursorPosition(windowCursorPos.x_, windowCursorPos.y_);
		event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>((mouseButtons == MOUSEB_LEFT) ? 0 : (mouseButtons == MOUSEB_RIGHT) ? 1 : 2));
		cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
	}
	void FairyGUIImpl::OnMouseButtonUp(const IntVector2& windowCursorPos, MouseButton mouseButtons)
	{
		cocos2d::EventMouse event(cocos2d::EventMouse::MouseEventType::MOUSE_UP);
		event.setCursorPosition(windowCursorPos.x_, windowCursorPos.y_);
		event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>((mouseButtons == MOUSEB_LEFT) ? 0 : (mouseButtons == MOUSEB_RIGHT) ? 1 : 2));
		cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
	}
	void FairyGUIImpl::OnMouseMove(float x, float y)
	{

	}
}