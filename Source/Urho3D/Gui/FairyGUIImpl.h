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
	class GLViewImpl;
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
		void OnKey(unsigned int key, bool isPress);
	private:
		fairygui::GRoot* fairy_root_{ nullptr };
		cocos2d::Scene* cocos_scene_{ nullptr };
		cocos2d::Renderer* cocos_renderder_{ nullptr };
		cocos2d::GLViewImpl* view_impl_{ nullptr};
		Context* context_{ nullptr };
		float _scaleX{ 1.0f };
		float _scaleY{ 1.0f };
	};
}