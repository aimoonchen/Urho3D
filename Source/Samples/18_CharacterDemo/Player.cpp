#include "Player.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include "Urho3D/Graphics/Material.h"
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include "Character.h"

namespace race
{
	struct ModelRes
	{
		std::string model;
		std::string mtl;
		std::string ani;
	};
	ModelRes g_mr[4] = {
		{"Models/Mutant/Mutant.mdl", "Models/Mutant/Materials/mutant_M.xml", ""},
		{"Models/Kachujin/Kachujin.mdl", "Models/Kachujin/Materials/Kachujin.xml", "Models/Kachujin/Kachujin_Walk.ani"},
		{"Models/NinjaSnowWar/Ninja.mdl", "Materials/NinjaSnowWar/Ninja.xml", "Models/NinjaSnowWar/Ninja_Walk.ani"},
		{"Models/Jack.mdl", "Models/Jack.xml", "Models/Jack_Walk.ani"},
	};

	void Player::SetScene(Urho3D::Scene* scene)
	{
		scene_ = scene;
	}

	void Player::SetRoleId(int role_id)
	{
		static Urho3D::Vector3 test_pos{ 0.0f, 1.0f, 10.0f };
		auto* cache = scene_->GetSubsystem<Urho3D::ResourceCache>();

		Urho3D::Node* objectNode = scene_->CreateChild(nick_name_.c_str());
		objectNode->SetPosition(test_pos);
		test_pos.x_ += 10.0f;
		// spin node
		Urho3D::Node* adjustNode = objectNode->CreateChild("AdjNode");
		adjustNode->SetRotation(Urho3D::Quaternion(180, Urho3D::Vector3(0, 1, 0)));

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
	}

	void Player::Update(float elaspedTime)
	{

	}
}