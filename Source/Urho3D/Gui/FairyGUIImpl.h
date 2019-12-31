#pragma once
#include "../Input/InputConstants.h"

namespace fairygui
{
	class GRoot;
}
namespace cocos2d
{
	class Scene;
	class Renderer;
}
namespace Urho3D {
	class Context;
	class IntVector2;
	class FairyGUIImpl
	{
	public:
		void Initialize(Context* context);
		void Update(float timeStep);
		void Render();
		fairygui::GRoot* GetRootWindow() { return fairy_root_; }
		void OnWindowSizeChanged();
		void OnMouseButtonDown(const IntVector2& windowCursorPos, MouseButton mouseButtons);
		void OnMouseButtonUp(const IntVector2& windowCursorPos, MouseButton mouseButtons);
		void OnMouseMove(float x, float y);

		void OnTouchesBegin(int num, intptr_t ids[], float xs[], float ys[]);
		void OnTouchesMove(int num, intptr_t ids[], float xs[], float ys[]);
		void OnTouchesMove(int num, intptr_t ids[], float xs[], float ys[], float fs[], float ms[]);
		void OnTouchesEnd(int num, intptr_t ids[], float xs[], float ys[]);
		void OnTouchesCancel(int num, intptr_t ids[], float xs[], float ys[]);

	private:
		void on_touches_end_or_cancel(cocos2d::EventTouch::EventCode eventCode, int num, intptr_t ids[], float xs[], float ys[]);
	private:
		fairygui::GRoot* fairy_root_{ nullptr };
		cocos2d::Scene* cocos_scene_{ nullptr };
		cocos2d::Renderer* cocos_renderder_{ nullptr };
		Context* context_{ nullptr };
		float _scaleX{ 1.0f };
		float _scaleY{ 1.0f };
	};
}