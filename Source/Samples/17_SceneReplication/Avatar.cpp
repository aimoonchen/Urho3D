#include "Avatar.h"

#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/AnimatedModel.h"
#include "Urho3D/Graphics/AnimationController.h"
#include "Urho3D/IO/MemoryBuffer.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Physics/CollisionShape.h"
#include "Urho3D/Physics/PhysicsEvents.h"
#include "Urho3D/Input/Controls.h"

namespace race
{
	Avatar::Avatar(Urho3D::Context* context, Urho3D::Scene* scene)
		: Object(context)
		, scene_{ scene }
	{

	}

	Avatar::~Avatar()
	{

	}
	
	unsigned int Avatar::GetID() const
	{
		return node_ != nullptr ? node_->GetID() : 0;
	}

	Urho3D::AnimatedModel* Avatar::create_model(Urho3D::Node* modelNode, race::RoleId roleId)
	{
		auto* cache = scene_->GetSubsystem<Urho3D::ResourceCache>();
		auto* modelObject = modelNode->CreateComponent<Urho3D::AnimatedModel>();
		modelObject->SetModel(cache->GetResource<Urho3D::Model>(race::g_model_res[roleId].model.c_str()));
		modelObject->SetMaterial(cache->GetResource<Urho3D::Material>(race::g_model_res[roleId].mtl.c_str()));
		if (roleId == race::kBloodelf) {
			auto hairMtl = cache->GetResource<Urho3D::Material>("Models/Wow/Bloodelf/Female/Materials/JoinedMaterial_#6.xml");
			auto eyeMtl = cache->GetResource<Urho3D::Material>("Models/Wow/Bloodelf/Female/Materials/JoinedMaterial_#17.xml");
			modelObject->SetMaterial(1, hairMtl);
			modelObject->SetMaterial(6, hairMtl);
			modelObject->SetMaterial(16, eyeMtl);
			modelObject->SetMaterial(17, eyeMtl);
		}
		else if (roleId == race::kHuman) {
			auto hairMtl = cache->GetResource<Urho3D::Material>("Models/Wow/Human/Female/Materials/JoinedMaterial_#6.xml");
			modelObject->SetMaterial(0, hairMtl);
			modelObject->SetMaterial(5, hairMtl);
			modelObject->SetMaterial(6, hairMtl);
		}
		else if (roleId == race::kOrc) {
			auto hairMtl = cache->GetResource<Urho3D::Material>("Models/Wow/Orc/Male/Materials/characterorcmaleorcmale_hd_41.xml");
			modelObject->SetMaterial(4, hairMtl);
		}
		else if (roleId == race::kTauren) {
			auto hairMtl = cache->GetResource<Urho3D::Material>("Models/Wow/Tauren/Male/Materials/JoinedMaterial_#6.xml");
			auto hornMtl = cache->GetResource<Urho3D::Material>("Models/Wow/Tauren/Male/Materials/charactertaurenmaletaurenmale_hd_16.xml");
			modelObject->SetMaterial(6, hairMtl);
			modelObject->SetMaterial(7, hornMtl);
		}
		modelObject->SetCastShadows(true);

		auto animCtrl = modelNode->CreateComponent<Urho3D::AnimationController>();
		auto& ani = race::g_ani_state[roleId][race::kIdle];
		animCtrl->PlayExclusive(ani.c_str(), 0, true, 0.2f);

		if (roleId != race::kMutant) {
			modelNode->SetScale({ 0.03f,0.03f,0.03f });
		}

		return modelObject;
	}

	bool Avatar::Init(race::RoleId roleId, const Urho3D::String& name, const Urho3D::Vector3& pos)
	{
		role_id_ = roleId;

		auto objectNode = scene_->CreateChild(name);
		objectNode->SetPosition(pos);
		
		auto adjustNode = objectNode->CreateChild("AdjNode");
		if (roleId != race::kMutant) {
			float scale = 0.02f;
			adjustNode->SetRotation(Urho3D::Quaternion(-90, Urho3D::Vector3(0, 1, 0)));
			adjustNode->SetScale(Urho3D::Vector3{ scale, scale, scale });
		} else {
			adjustNode->SetRotation(Urho3D::Quaternion(180, Urho3D::Vector3(0, 1, 0)));
		}
		create_model(adjustNode, roleId);

		// Create rigidbody, and set non-zero mass so that the body becomes dynamic
		auto* body = objectNode->CreateComponent<Urho3D::RigidBody>();
		body->SetMass(1.0f);
		body->SetFriction(1.0f);
		body->SetLinearDamping(0.5f);
		body->SetAngularDamping(0.5f);

		// Set zero angular factor so that physics doesn't turn the character on its own.
		// Instead we will control the character yaw manually
		body->SetAngularFactor(Urho3D::Vector3::ZERO);

		// Set a capsule shape for collision
		auto* shape = objectNode->CreateComponent<Urho3D::CollisionShape>();
		shape->SetCapsule(0.7f, 1.8f, Urho3D::Vector3(0.0f, 0.9f, 0.0f));

		auto* light = objectNode->CreateComponent<Urho3D::Light>();
		light->SetRange(3.0f);
		light->SetColor(Urho3D::Color(0.5f + ((unsigned)Urho3D::Rand() & 1u) * 0.5f, 0.5f + ((unsigned)Urho3D::Rand() & 1u) * 0.5f, 0.5f + ((unsigned)Urho3D::Rand() & 1u) * 0.5f));
		
		SubscribeToEvent(objectNode, Urho3D::E_NODECOLLISION, URHO3D_HANDLER(Avatar, handle_node_collision));
		
		node_ = objectNode;
		return true;
	}

	void Avatar::handle_node_collision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
	{
		// Check collision contacts and see if character is standing on ground (look for a contact that has near vertical normal)
		//using namespace Urho3D::NodeCollision;

		Urho3D::MemoryBuffer contacts(eventData[Urho3D::NodeCollision::P_CONTACTS].GetBuffer());

		while (!contacts.IsEof())
		{
			Urho3D::Vector3 contactPosition = contacts.ReadVector3();
			Urho3D::Vector3 contactNormal = contacts.ReadVector3();
			/*float contactDistance = */contacts.ReadFloat();
			/*float contactImpulse = */contacts.ReadFloat();

			// If contact is below node center and pointing up, assume it's a ground contact
			if (contactPosition.y_ < (node_->GetPosition().y_ + 1.0f))
			{
				float level = contactNormal.y_;
				if (level > 0.75)
					onGround_ = true;
			}
		}
	}

	void Avatar::ApplyContrlToNode(const Urho3D::Controls& ctrl)
	{
		// 	bool onGround_ = true;
		// 	float inAirTimer_ = 0.0f;
		// 	bool okToJump_ = true;
			/// \todo Could cache the components for faster access instead of finding them each frame
		auto* body = node_->GetComponent<Urho3D::RigidBody>();
		auto* animCtrl = node_->GetComponent<Urho3D::AnimationController>(true);

		// Update the in air timer. Reset if grounded
		if (!onGround_)
			inAirTimer_ += 0;// timeStep;
		else
			inAirTimer_ = 0.0f;
		// When character has been in air less than 1/10 second, it's still interpreted as being on ground
		bool softGrounded = inAirTimer_ < INAIR_THRESHOLD_TIME;

		// Update movement & animation
		const Urho3D::Quaternion& rot = node_->GetRotation();
		Urho3D::Vector3 moveDir = Urho3D::Vector3::ZERO;
		const Urho3D::Vector3& velocity = body->GetLinearVelocity();
		// Velocity on the XZ plane
		Urho3D::Vector3 planeVelocity(velocity.x_, 0.0f, velocity.z_);

		if (ctrl.IsDown(CTRL_FORWARD))
			moveDir += Urho3D::Vector3::FORWARD;
		if (ctrl.IsDown(CTRL_BACK))
			moveDir += Urho3D::Vector3::BACK;
		if (ctrl.IsDown(CTRL_LEFT))
			moveDir += Urho3D::Vector3::LEFT;
		if (ctrl.IsDown(CTRL_RIGHT))
			moveDir += Urho3D::Vector3::RIGHT;

		// Normalize move vector so that diagonal strafing is not faster
		if (moveDir.LengthSquared() > 0.0f)
			moveDir.Normalize();

		// If in air, allow control, but slower than when on ground
		body->ApplyImpulse(rot * moveDir * (softGrounded ? MOVE_FORCE : INAIR_MOVE_FORCE));

		if (softGrounded)
		{
			// When on ground, apply a braking force to limit maximum ground velocity
			Urho3D::Vector3 brakeForce = -planeVelocity * BRAKE_FORCE;
			body->ApplyImpulse(brakeForce);

			// Jump. Must release jump control between jumps
			if (ctrl.IsDown(CTRL_JUMP))
			{
				if (okToJump_)
				{
					body->ApplyImpulse(Urho3D::Vector3::UP * JUMP_FORCE);
					okToJump_ = false;
					auto& ani = race::g_ani_state[role_id_][race::kJump];
					if (!ani.empty())
					{
						animCtrl->PlayExclusive(ani.c_str(), 0, false, 0.2f);
					}
					//animCtrl->PlayExclusive("Models/Mutant/Mutant_Jump1.ani", 0, false, 0.2f);
				}
			}
			else
				okToJump_ = true;
		}

		if (!onGround_)
		{
			auto& ani = race::g_ani_state[role_id_][race::kJump];
			if (!ani.empty())
			{
				animCtrl->PlayExclusive(ani.c_str(), 0, false, 0.2f);
			}
			//animCtrl->PlayExclusive("Models/Mutant/Mutant_Jump1.ani", 0, false, 0.2f);
		}
		else
		{
			// Play walk animation if moving on ground, otherwise fade it out
			if (softGrounded && !moveDir.Equals(Urho3D::Vector3::ZERO))
			{
				auto& ani = race::g_ani_state[role_id_][race::kRun];
				if (!ani.empty())
				{
					animCtrl->PlayExclusive(ani.c_str(), 0, true, 0.2f);
				}
			}
			// animCtrl->PlayExclusive("Models/Mutant/Mutant_Run.ani", 0, true, 0.2f);
			else
			{
				auto& ani = race::g_ani_state[role_id_][race::kIdle];
				if (!ani.empty())
				{
					animCtrl->PlayExclusive(ani.c_str(), 0, true, 0.2f);
				}
			}
			//animCtrl->PlayExclusive("Models/Mutant/Mutant_Idle0.ani", 0, true, 0.2f);

			auto& ani = race::g_ani_state[role_id_][race::kRun];
			if (!ani.empty())
			{
				animCtrl->SetSpeed(ani.c_str(), planeVelocity.Length() * 0.3f);
			}
			// Set walk animation speed proportional to velocity
			//animCtrl->SetSpeed("Models/Mutant/Mutant_Run.ani", planeVelocity.Length() * 0.3f);
		}

		// Reset grounded flag for next frame
		onGround_ = false;
	}
}