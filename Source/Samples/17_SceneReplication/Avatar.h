#pragma once
#include "Race.h"
#include "Urho3D/Core/Object.h"

namespace Urho3D
{
	class Vector3;
	class String;
	class Node;
	class Scene;
	class AnimatedModel;
	class Controls;
}
namespace race
{
	struct AavatarConfig 
	{

	};
	class Avatar : public Urho3D::Object
	{
		URHO3D_OBJECT(Avatar, Urho3D::Object);
	public:
		Avatar(Urho3D::Context* context, Urho3D::Scene* scene);
		~Avatar();
		bool Init(race::RoleId roleId, const Urho3D::String& name, const Urho3D::Vector3& pos);
		race::RoleId GetRoleId() const { return role_id_; }
		Urho3D::Node* GetNode() const { return node_; }
		unsigned int GetID() const;
		void ApplyContrlToNode(const Urho3D::Controls& ctrl);
	private:
		Urho3D::AnimatedModel* create_model(Urho3D::Node* modelNode, race::RoleId roleId);
		/// Handle physics collision event.
		void handle_node_collision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	private:
		Urho3D::Scene* scene_{ nullptr };
		Urho3D::Node* node_{ nullptr };
		race::RoleId role_id_;
		/// Grounded flag for movement.
		bool onGround_{ false };
		/// Jump flag.
		bool okToJump_{ true };
		/// In air timer. Due to possible physics inaccuracy, character can be off ground for max. 1/10 second and still be allowed to move.
		float inAirTimer_{ 0.0f };
	};
}