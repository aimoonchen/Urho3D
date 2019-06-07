#include "Player.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include "Urho3D/Graphics/Material.h"
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include "Character.h"
#include "Race.h"
namespace race
{
	struct ModelRes
	{
		std::string model;
		std::string mtl;
	};
	ModelRes g_mr[kMaxRoleId] = {
		{"Models/Mutant/Mutant.mdl", "Models/Mutant/Materials/mutant_M.xml"},
		{"Models/Wow/Bloodelf/Female/Purple.mdl", "Models/Wow/Bloodelf/Female/Materials/JoinedMaterial_#18.xml"},
		{"Models/Wow/Human/Female/Red.mdl", "Models/Wow/Human/Female/Materials/JoinedMaterial_#13.xml"},
		{"Models/Wow/Orc/Male/Green.mdl", "Models/Wow/Orc/Male/Materials/JoinedMaterial_#12.xml"},
		{"Models/Wow/Pandaren/Male/Black.mdl", "Models/Wow/Pandaren/Male/Materials/JoinedMaterial_#9.xml"},
	};

	std::string g_ani_state[kMaxRoleId][kMaxAniState] = {
		{{"Models/Mutant/Mutant_Idle0.ani"},{"Models/Mutant/Mutant_Walk.ani"},{"Models/Mutant/Mutant_Run.ani"},{"Models/Mutant/Mutant_Jump1.ani"}},
		{{"Models/Wow/Bloodelf/Female/Purple_Stand [3].ani"},{"Models/Wow/Bloodelf/Female/Purple_Walk [154].ani"},{"Models/Wow/Bloodelf/Female/Purple_Run [22].ani"},{""}},
		{{"Models/Wow/Human/Female/Red_Stand [0].ani"},{"Models/Wow/Human/Female/Red_Walk [1].ani"},{"Models/Wow/Human/Female/Red_Run [2].ani"},{""}},
		{{"Models/Wow/Orc/Male/Green_Stand [0].ani"},{"Models/Wow/Orc/Male/Green_Walk [1].ani"},{"Models/Wow/Orc/Male/Green_Run [2].ani"},{""}},
		{{"Models/Wow/Pandaren/Male/Black_Stand [1].ani"},{"Models/Wow/Pandaren/Male/Black_Walk [83].ani"},{"Models/Wow/Pandaren/Male/Black_Run [62].ani"},{""}},
	};
	void Player::SetScene(Urho3D::Scene* scene)
	{
		scene_ = scene;
	}

	void Player::SetRoleId(int role_id)
	{
		Urho3D::Vector3 pos{ 0.0f, 1.0f, 0.0f };
		auto* cache = scene_->GetSubsystem<Urho3D::ResourceCache>();
		auto trackPos = track_->GetPos();
		auto ti = room_->GetTrackInfo();
		pos.x_ = trackPos.x_ + 0.5f * ti.width_;
		Urho3D::Node* objectNode  = scene_->CreateChild(nick_name_.c_str());
		objectNode->SetPosition(pos);
		// spin node
		Urho3D::Node* adjustNode = objectNode->CreateChild("AdjNode");
		if (role_id != 0) {
			adjustNode->SetRotation(Urho3D::Quaternion(-90, Urho3D::Vector3(0, 1, 0)));
			adjustNode->SetScale(Urho3D::Vector3{ 0.05f, 0.05f, 0.05f });
		} else {
			adjustNode->SetRotation(Urho3D::Quaternion(180, Urho3D::Vector3(0, 1, 0)));
		}
		// Create the rendering component + animation controller
		auto* object = adjustNode->CreateComponent<Urho3D::AnimatedModel>();
		object->SetModel(cache->GetResource<Urho3D::Model>(g_mr[role_id].model.c_str()));
		object->SetMaterial(cache->GetResource<Urho3D::Material>(g_mr[role_id].mtl.c_str()));
		object->SetCastShadows(true);
		adjustNode->CreateComponent<Urho3D::AnimationController>();
		// Set the head bone for manual control
		//object->GetSkeleton().GetBone("Mutant:Head")->animated_ = false;

		// Create rigidbody, and set non-zero mass so that the body becomes dynamic
		auto* body = objectNode->CreateComponent<Urho3D::RigidBody>();
		body->SetCollisionLayer(1);
		body->SetMass(1.0f);

		// Set zero angular factor so that physics doesn't turn the character on its own.
		// Instead we will control the character yaw manually
		body->SetAngularFactor(Urho3D::Vector3::ZERO);

		// Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
		body->SetCollisionEventMode(Urho3D::COLLISION_ALWAYS);

		// Set a capsule shape for collision
		auto* shape = objectNode->CreateComponent<Urho3D::CollisionShape>();
		shape->SetCapsule(0.7f, 1.8f, Urho3D::Vector3(0.0f, 0.9f, 0.0f));

		// Create the character logic component, which takes care of steering the rigidbody
		// Remember it so that we can set the controls. Use a WeakPtr because the scene hierarchy already owns it
		// and keeps it alive as long as it's not removed from the hierarchy
		character_ = objectNode->CreateComponent<Character>();
		character_->SetRoleId(race::RoleId(role_id));
		entity_ = objectNode;
	}

	void Player::LeaveScene()
	{
		scene_->RemoveChild(entity_);
	}

	void Player::Update(float elaspedTime)
	{

	}
}