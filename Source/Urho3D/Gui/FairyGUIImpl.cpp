#include "FairyGUIImpl.h"
#include <map>
#include <vector>
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

namespace {

	static cocos2d::Touch* g_touches[cocos2d::EventTouch::MAX_TOUCHES] = { nullptr };
	static unsigned int g_indexBitsUsed = 0;
	// System touch pointer ID (It may not be ascending order number) <-> Ascending order number from 0
	static std::map<intptr_t, int> g_touchIdReorderMap;

	static int getUnUsedIndex()
	{
		int i;
		int temp = g_indexBitsUsed;

		for (i = 0; i < cocos2d::EventTouch::MAX_TOUCHES; i++) {
			if (!(temp & 0x00000001)) {
				g_indexBitsUsed |= (1 << i);
				return i;
			}

			temp >>= 1;
		}

		// all bits are used
		return -1;
	}

	static std::vector<cocos2d::Touch*> getAllTouchesVector()
	{
		std::vector<cocos2d::Touch*> ret;
		int i;
		int temp = g_indexBitsUsed;

		for (i = 0; i < cocos2d::EventTouch::MAX_TOUCHES; i++) {
			if (temp & 0x00000001) {
				ret.push_back(g_touches[i]);
			}
			temp >>= 1;
		}
		return ret;
	}

	static void removeUsedIndexBit(int index)
	{
		if (index < 0 || index >= cocos2d::EventTouch::MAX_TOUCHES)
		{
			return;
		}

		unsigned int temp = 1 << index;
		temp = ~temp;
		g_indexBitsUsed &= temp;
	}

}

namespace Urho3D {
	void FairyGUIImpl::Initialize(Context* context)
	{
		context_ = context;
		SetUrho3DContext(context);
 		cocos2d::FileUtils::getInstance()->addSearchPath("C:/GitProjects/Urho3D/build/bin/Data/FairyGUI/Resources");
 		context->GetSubsystem<ResourceCache>()->AddResourceDir("C:/GitProjects/Urho3D/build/bin/Data/FairyGUI/Resources");
// 		cocos2d::FileUtils::getInstance()->addSearchPath("D:/Github/Urho3D/build/bin/Data/FairyGUI/Resources");
// 		context->GetSubsystem<ResourceCache>()->AddResourceDir("D:/Github/Urho3D/build/bin/Data/FairyGUI/Resources");
		cocos_scene_ = cocos2d::Scene::create();
		cocos_renderder_ = new cocos2d::Renderer;
		fairy_root_ = fairygui::GRoot::create(cocos_scene_);
		cocos2d::Director::getInstance()->setOpenGLView(nullptr);
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
		event.setCursorPosition(static_cast<float>(windowCursorPos.x_), static_cast<float>(windowCursorPos.y_));
		event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>((mouseButtons == MOUSEB_LEFT) ? 0 : (mouseButtons == MOUSEB_RIGHT) ? 1 : 2));
		cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
	}

	void FairyGUIImpl::OnMouseButtonUp(const IntVector2& windowCursorPos, MouseButton mouseButtons)
	{
		cocos2d::EventMouse event(cocos2d::EventMouse::MouseEventType::MOUSE_UP);
		event.setCursorPosition(static_cast<float>(windowCursorPos.x_), static_cast<float>(windowCursorPos.y_));
		event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>((mouseButtons == MOUSEB_LEFT) ? 0 : (mouseButtons == MOUSEB_RIGHT) ? 1 : 2));
		cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
	}

	void FairyGUIImpl::OnMouseMove(float x, float y)
	{

	}

	void FairyGUIImpl::OnTouchesBegin(int num, intptr_t ids[], float xs[], float ys[])
	{
		intptr_t id = 0;
		float x = 0.0f;
		float y = 0.0f;
		int unusedIndex = 0;
		cocos2d::EventTouch touchEvent;

		for (int i = 0; i < num; ++i)
		{
			id = ids[i];
			x = xs[i];
			y = ys[i];

			auto iter = g_touchIdReorderMap.find(id);

			// it is a new touch
			if (iter == g_touchIdReorderMap.end())
			{
				unusedIndex = getUnUsedIndex();

				// The touches is more than MAX_TOUCHES ?
				if (unusedIndex == -1) {
					CCLOG("The touches is more than MAX_TOUCHES, unusedIndex = %d", unusedIndex);
					continue;
				}

				cocos2d::Touch* touch = g_touches[unusedIndex] = new (std::nothrow) cocos2d::Touch();
				touch->setTouchInfo(unusedIndex, (x - _viewPortRect.origin.x) / _scaleX,
					(y - _viewPortRect.origin.y) / _scaleY);

				CCLOGINFO("x = %f y = %f", touch->getLocationInView().x, touch->getLocationInView().y);

				g_touchIdReorderMap.emplace(id, unusedIndex);
				touchEvent._touches.push_back(touch);
			}
		}

		if (touchEvent._touches.size() == 0)
		{
			CCLOG("touchesBegan: size = 0");
			return;
		}

		touchEvent._eventCode = cocos2d::EventTouch::EventCode::BEGAN;
		auto dispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
		dispatcher->dispatchEvent(&touchEvent);
	}

	void FairyGUIImpl::OnTouchesMove(int num, intptr_t ids[], float xs[], float ys[])
	{
		OnTouchesMove(num, ids, xs, ys, nullptr, nullptr);
	}

	void FairyGUIImpl::OnTouchesMove(int num, intptr_t ids[], float xs[], float ys[], float fs[], float ms[])
	{
		intptr_t id = 0;
		float x = 0.0f;
		float y = 0.0f;
		float force = 0.0f;
		float maxForce = 0.0f;
		cocos2d::EventTouch touchEvent;

		for (int i = 0; i < num; ++i)
		{
			id = ids[i];
			x = xs[i];
			y = ys[i];
			force = fs ? fs[i] : 0.0f;
			maxForce = ms ? ms[i] : 0.0f;

			auto iter = g_touchIdReorderMap.find(id);
			if (iter == g_touchIdReorderMap.end())
			{
				CCLOG("if the index doesn't exist, it is an error");
				continue;
			}

			CCLOGINFO("Moving touches with id: %d, x=%f, y=%f, force=%f, maxFource=%f", (int)id, x, y, force, maxForce);
			cocos2d::Touch* touch = g_touches[iter->second];
			if (touch)
			{
				touch->setTouchInfo(iter->second, (x - _viewPortRect.origin.x) / _scaleX,
					(y - _viewPortRect.origin.y) / _scaleY, force, maxForce);

				touchEvent._touches.push_back(touch);
			}
			else
			{
				// It is error, should return.
				CCLOG("Moving touches with id: %ld error", (long int)id);
				return;
			}
		}

		if (touchEvent._touches.size() == 0)
		{
			CCLOG("touchesMoved: size = 0");
			return;
		}

		touchEvent._eventCode = cocos2d::EventTouch::EventCode::MOVED;
		auto dispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
		dispatcher->dispatchEvent(&touchEvent);
	}

	void FairyGUIImpl::on_touches_end_or_cancel(cocos2d::EventTouch::EventCode eventCode, int num, intptr_t ids[], float xs[], float ys[])
	{
		intptr_t id = 0;
		float x = 0.0f;
		float y = 0.0f;
		cocos2d::EventTouch touchEvent;

		for (int i = 0; i < num; ++i)
		{
			id = ids[i];
			x = xs[i];
			y = ys[i];

			auto iter = g_touchIdReorderMap.find(id);
			if (iter == g_touchIdReorderMap.end())
			{
				CCLOG("if the index doesn't exist, it is an error");
				continue;
			}

			/* Add to the set to send to the director */
			cocos2d::Touch* touch = g_touches[iter->second];
			if (touch)
			{
				CCLOGINFO("Ending touches with id: %d, x=%f, y=%f", (int)id, x, y);
				touch->setTouchInfo(iter->second, (x - _viewPortRect.origin.x) / _scaleX,
					(y - _viewPortRect.origin.y) / _scaleY);

				touchEvent._touches.push_back(touch);

				g_touches[iter->second] = nullptr;
				removeUsedIndexBit(iter->second);

				g_touchIdReorderMap.erase(id);
			}
			else
			{
				CCLOG("Ending touches with id: %ld error", static_cast<long>(id));
				return;
			}

		}

		if (touchEvent._touches.size() == 0)
		{
			CCLOG("touchesEnded or touchesCancel: size = 0");
			return;
		}

		touchEvent._eventCode = eventCode;
		auto dispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
		dispatcher->dispatchEvent(&touchEvent);

		for (auto& touch : touchEvent._touches)
		{
			// release the touch object.
			touch->release();
		}
	}

	void FairyGUIImpl::OnTouchesEnd(int num, intptr_t ids[], float xs[], float ys[])
	{
		on_touches_end_or_cancel(cocos2d::EventTouch::EventCode::ENDED, num, ids, xs, ys);
	}

	void FairyGUIImpl::OnTouchesCancel(int num, intptr_t ids[], float xs[], float ys[])
	{
		on_touches_end_or_cancel(cocos2d::EventTouch::EventCode::CANCELLED, num, ids, xs, ys);
	}
}