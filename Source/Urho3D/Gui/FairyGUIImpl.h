#pragma once
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
	class FairyGUIImpl
	{
	public:
		void Initialize(Context* context);
		void Update(float timeStep);
		void Render();
	private:
		fairygui::GRoot* fairy_root_{ nullptr };
		cocos2d::Scene* cocos_scene_{ nullptr };
		cocos2d::Renderer* cocos_renderder_{ nullptr };
	};
}