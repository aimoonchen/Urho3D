//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// fairygui
#include "UIPackage.h"
#include "GComponent.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

#include "SceneReplication.h"

#include <Urho3D/DebugNew.h>
#include "Cocos2d/cocos/Urho3DContext.h"
#include "Cocos2d/cocos/platform/CCFileUtils.h"
#include "Cocos2d/cocos/2d/CCScene.h"
#include "Cocos2d/cocos/renderer/CCRenderer.h"
#include "Race.h"
//#include "Character.h"
#include "RaceRoom.h"
#include "Avatar.h"
#include "GRoot.h"

// UDP port we will use
static const unsigned short SERVER_PORT = 2345;
// Identifier for our custom remote event we use to tell the client which object they control
static const StringHash E_CLIENTOBJECTID("ClientObjectID");
// Identifier for the node ID parameter in the event data
static const StringHash P_ID("ID");

URHO3D_DEFINE_APPLICATION_MAIN(SceneReplication)

SceneReplication::SceneReplication(Context* context) :
    Sample(context)
{
	//Character::RegisterObject(context);
}

void SceneReplication::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to necessary events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
	Sample::InitMouseMode(MM_ABSOLUTE/*MM_RELATIVE*/);
	auto* input = GetSubsystem<Input>();
	input->SetMouseVisible(true);
}

void SceneReplication::CreateScene()
{
    scene_ = new Scene(context_);

    auto* cache = GetSubsystem<ResourceCache>();

    // Create octree and physics world with default settings. Create them as local so that they are not needlessly replicated
    // when a client connects
    scene_->CreateComponent<Octree>(LOCAL);
    scene_->CreateComponent<PhysicsWorld>(LOCAL);

    // All static scene content and the camera are also created as local, so that they are unaffected by scene replication and are
    // not removed from the client upon connection. Create a Zone component first for ambient lighting & fog control.
    Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    // Create a directional light without shadows
    Node* lightNode = scene_->CreateChild("DirectionalLight", LOCAL);
    lightNode->SetDirection(Vector3(0.5f, -1.0f, 0.5f));
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetColor(Color(0.2f, 0.2f, 0.2f));
    light->SetSpecularIntensity(1.0f);

    // Create a "floor" consisting of several tiles. Make the tiles physical but leave small cracks between them
    for (int y = -20; y <= 20; ++y)
    {
        for (int x = -20; x <= 20; ++x)
        {
            Node* floorNode = scene_->CreateChild("FloorTile", LOCAL);
            floorNode->SetPosition(Vector3(x * 20.2f, -0.5f, y * 20.2f));
            floorNode->SetScale(Vector3(20.0f, 1.0f, 20.0f));
            auto* floorObject = floorNode->CreateComponent<StaticModel>();
            floorObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
            floorObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

            auto* body = floorNode->CreateComponent<RigidBody>();
            body->SetFriction(1.0f);
            auto* shape = floorNode->CreateComponent<CollisionShape>();
            shape->SetBox(Vector3::ONE);
        }
    }

    // Create the camera. Limit far clip distance to match the fog
    // The camera needs to be created into a local node so that each client can retain its own camera, that is unaffected by
    // network messages. Furthermore, because the client removes all replicated scene nodes when connecting to a server scene,
    // the screen would become blank if the camera node was replicated (as only the locally created camera is assigned to a
    // viewport in SetupViewports() below)
    cameraNode_ = scene_->CreateChild("Camera", LOCAL);
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
}

void SceneReplication::CreateUI()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* ui = GetSubsystem<UI>();
    UIElement* root = ui->GetRoot();
    auto* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Set style to the UI root so that elements will inherit it
    root->SetDefaultStyle(uiStyle);

    // Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor will
    // control the camera, and when visible, it can interact with the login UI
    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto(uiStyle);
    ui->SetCursor(cursor);
    // Set starting position of the cursor at the rendering window center
    auto* graphics = GetSubsystem<Graphics>();
    cursor->SetPosition(graphics->GetWidth() / 2, graphics->GetHeight() / 2);

    // Construct the instructions text element
    instructionsText_ = ui->GetRoot()->CreateChild<Text>();
    instructionsText_->SetText(
        "Use WASD keys to move and RMB to rotate view"
    );
    instructionsText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    // Position the text relative to the screen center
    instructionsText_->SetHorizontalAlignment(HA_CENTER);
    instructionsText_->SetVerticalAlignment(VA_CENTER);
    instructionsText_->SetPosition(0, graphics->GetHeight() / 4);
    // Hide until connected
    instructionsText_->SetVisible(false);

    buttonContainer_ = root->CreateChild<UIElement>();
    buttonContainer_->SetFixedSize(500, 20);
    buttonContainer_->SetPosition(20, 20);
    buttonContainer_->SetLayoutMode(LM_HORIZONTAL);

    textEdit_ = buttonContainer_->CreateChild<LineEdit>();
    textEdit_->SetStyleAuto();

    connectButton_ = CreateButton("Connect", 90);
    disconnectButton_ = CreateButton("Disconnect", 100);
    startServerButton_ = CreateButton("Start Server", 110);

    UpdateButtons();

	InitFairyGUI();
}
void SceneReplication::InitFairyGUI()
{
	SetUrho3DContext(GetContext());
    ui_scene_ = cocos2d::Scene::create();
    ui_renderder_ = new cocos2d::Renderer;
	groot_ = fairygui::GRoot::create(ui_scene_);
	cocos2d::FileUtils::getInstance()->addSearchPath("D:/Github/Urho3D/Build/bin/Data/FairyGUI/Resources");
    GetSubsystem<ResourceCache>()->AddResourceDir("D:/Github/Urho3D/Build/bin/Data/FairyGUI/Resources");
	fairygui::UIPackage::addPackage("UI/MainMenu");
	auto _view = fairygui::UIPackage::createObject("MainMenu", "Main")->as<fairygui::GComponent>();
    groot_->addChild(_view);
	_view->getChild("n1")->addClickListener([this](fairygui::EventContext*)
	{
// 		TransitionFade* scene = TransitionFade::create(0.5f, BasicsScene::create());
// 		Director::getInstance()->replaceScene(scene);
	});
    
}
void SceneReplication::SetupViewport()
{
    auto* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void SceneReplication::SubscribeToEvents()
{
    // Subscribe to fixed timestep physics updates for setting or applying controls
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(SceneReplication, HandlePhysicsPreStep));

    // Subscribe HandlePostUpdate() method for processing update events. Subscribe to PostUpdate instead
    // of the usual Update so that physics simulation has already proceeded for the frame, and can
    // accurately follow the object with the camera
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SceneReplication, HandlePostUpdate));

    // Subscribe to button actions
    SubscribeToEvent(connectButton_, E_RELEASED, URHO3D_HANDLER(SceneReplication, HandleConnect));
    SubscribeToEvent(disconnectButton_, E_RELEASED, URHO3D_HANDLER(SceneReplication, HandleDisconnect));
    SubscribeToEvent(startServerButton_, E_RELEASED, URHO3D_HANDLER(SceneReplication, HandleStartServer));

    // Subscribe to network events
    SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(SceneReplication, HandleConnectionStatus));
    SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(SceneReplication, HandleConnectionStatus));
    SubscribeToEvent(E_CONNECTFAILED, URHO3D_HANDLER(SceneReplication, HandleConnectionStatus));
    SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(SceneReplication, HandleClientConnected));
    SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(SceneReplication, HandleClientDisconnected));
    // This is a custom event, sent from the server to the client. It tells the node ID of the object the client should control
    SubscribeToEvent(E_CLIENTOBJECTID, URHO3D_HANDLER(SceneReplication, HandleClientObjectID));
    // Events sent between client & server (remote events) must be explicitly registered or else they are not allowed to be received
    GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTID);
}

Button* SceneReplication::CreateButton(const String& text, int width)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

    auto* button = buttonContainer_->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}

void SceneReplication::UpdateButtons()
{
    auto* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();
    bool serverRunning = network->IsServerRunning();

    // Show and hide buttons so that eg. Connect and Disconnect are never shown at the same time
    connectButton_->SetVisible(!serverConnection && !serverRunning);
    disconnectButton_->SetVisible(serverConnection || serverRunning);
    startServerButton_->SetVisible(!serverConnection && !serverRunning);
    textEdit_->SetVisible(!serverConnection && !serverRunning);
}
int GetRoleId()
{
	static constexpr int max_role_id = 5;
	static int current_id = 0;
	return current_id++ % max_role_id;
}

AnimatedModel* SceneReplication::CreateCharactor(Node* modelNode, race::RoleId roleId)
{
	auto* cache = GetSubsystem<ResourceCache>();
	auto* modelObject = modelNode->CreateComponent<AnimatedModel>();
	modelObject->SetModel(cache->GetResource<Model>(race::g_model_res[roleId].model.c_str()));
	modelObject->SetMaterial(cache->GetResource<Material>(race::g_model_res[roleId].mtl.c_str()));
	if (roleId == race::kBloodelf) {
		auto hairMtl = cache->GetResource<Material>("Models/Wow/Bloodelf/Female/Materials/JoinedMaterial_#6.xml");
		auto eyeMtl = cache->GetResource<Material>("Models/Wow/Bloodelf/Female/Materials/JoinedMaterial_#17.xml");
		modelObject->SetMaterial(1, hairMtl);
		modelObject->SetMaterial(6, hairMtl);
		modelObject->SetMaterial(16, eyeMtl);
		modelObject->SetMaterial(17, eyeMtl);
	}
	else if (roleId == race::kHuman) {
		auto hairMtl = cache->GetResource<Material>("Models/Wow/Human/Female/Materials/JoinedMaterial_#6.xml");
		modelObject->SetMaterial(0, hairMtl);
		modelObject->SetMaterial(5, hairMtl);
		modelObject->SetMaterial(6, hairMtl);
	}
	else if (roleId == race::kOrc) {
		auto hairMtl = cache->GetResource<Material>("Models/Wow/Orc/Male/Materials/characterorcmaleorcmale_hd_41.xml");
		modelObject->SetMaterial(4, hairMtl);
	}
	else if (roleId == race::kTauren) {
		auto hairMtl = cache->GetResource<Material>("Models/Wow/Tauren/Male/Materials/JoinedMaterial_#6.xml");
		auto hornMtl = cache->GetResource<Material>("Models/Wow/Tauren/Male/Materials/charactertaurenmaletaurenmale_hd_16.xml");
		modelObject->SetMaterial(6, hairMtl);
		modelObject->SetMaterial(7, hornMtl);
	}

	if (roleId != race::kMutant) {
		modelNode->SetScale({ 0.03f,0.03f,0.03f });
	}
	
	modelObject->SetCastShadows(true);
	return modelObject;
}

server::Player* SceneReplication::CreatePlayer(Connection* con)
{
	int room_id = 0;
	auto room = server::RaceRoomManager::GetInstancePtr()->FindRoom(room_id);
	if (!room)
	{
		server::TrackInfo ti;
		room = server::RaceRoomManager::GetInstancePtr()->CreateRoom(ti);
	}
	Urho3D::Vector3 pos{ 0.0f, 1.0f, 0.0f };
	int role_id = GetRoleId();
	auto track_id = room->GetFreeTack();
	String name;
	server::Player* new_player = nullptr;
	if (track_id != -1) {
		name.AppendWithFormat("Player_%d", current_player_id_);
		players_.push_back(std::make_unique<server::Player>(current_player_id_++));
		new_player = players_.back().get();
		new_player->SetConnection(con);
		room->AddPlayer(new_player);
		
		auto trackPos = new_player->GetTrack()->GetPos();
		auto ti = room->GetTrackInfo();
		pos.x_ = trackPos.x_ + 0.5f * ti.width_;
		pos.z_ = 30.0f;
		auto avatar = std::make_unique<race::Avatar>(context_, scene_);
		avatar->Init(race::RoleId(role_id), name, pos);
		new_player->SetAvatar(std::move(avatar));
	}
	return new_player;
	/*
	auto* cache = GetSubsystem<ResourceCache>();
	
	Node* objectNode = scene_->CreateChild(name);

	objectNode->SetPosition(pos);

	//objectNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 5.0f, Random(40.0f) - 20.0f));
	// spin node
	Urho3D::Node* adjustNode = objectNode->CreateChild("AdjNode");
	if (role_id != race::kMutant) {
		float scale = 0.02f;
		adjustNode->SetRotation(Urho3D::Quaternion(-90, Urho3D::Vector3(0, 1, 0)));
		adjustNode->SetScale(Urho3D::Vector3{ scale, scale, scale });
	} else {
		adjustNode->SetRotation(Urho3D::Quaternion(180, Urho3D::Vector3(0, 1, 0)));
	}
	// Create the rendering component + animation controller
// 	auto* object = adjustNode->CreateComponent<Urho3D::AnimatedModel>();
// 	object->SetModel(cache->GetResource<Urho3D::Model>(race::g_model_res[role_id].model.c_str()));
// 	object->SetMaterial(cache->GetResource<Urho3D::Material>(race::g_model_res[role_id].mtl.c_str()));
// 	object->SetCastShadows(true);
	CreateCharactor(adjustNode, (race::RoleId)role_id);
	auto animCtrl = adjustNode->CreateComponent<Urho3D::AnimationController>();
	auto& ani = race::g_ani_state[role_id][race::kIdle];
	animCtrl->PlayExclusive(ani.c_str(), 0, true, 0.2f);
	// Set the head bone for manual control
	//object->GetSkeleton().GetBone("Mutant:Head")->animated_ = false;

	// Create rigidbody, and set non-zero mass so that the body becomes dynamic
	auto* body = objectNode->CreateComponent<Urho3D::RigidBody>();
	body->SetMass(1.0f);
	body->SetFriction(1.0f);
	body->SetLinearDamping(0.5f);
	body->SetAngularDamping(0.5f);

	// Set zero angular factor so that physics doesn't turn the character on its own.
	// Instead we will control the character yaw manually
	body->SetAngularFactor(Urho3D::Vector3::ZERO);

	// Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
	//body->SetCollisionEventMode(Urho3D::COLLISION_ALWAYS);

	// Set a capsule shape for collision
	auto* shape = objectNode->CreateComponent<Urho3D::CollisionShape>();
	shape->SetCapsule(0.7f, 1.8f, Urho3D::Vector3(0.0f, 0.9f, 0.0f));
	auto* light = objectNode->CreateComponent<Light>();
	light->SetRange(3.0f);
	light->SetColor(Color(0.5f + ((unsigned)Rand() & 1u) * 0.5f, 0.5f + ((unsigned)Rand() & 1u) * 0.5f, 0.5f + ((unsigned)Rand() & 1u) * 0.5f));
	//auto character = objectNode->CreateComponent<Character>();
	//character->SetRoleId(race::RoleId(role_id));
	return objectNode;
	*/
	
//     // Create the scene node & visual representation. This will be a replicated object
//     Node* ballNode = scene_->CreateChild("Ball");
//     ballNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 5.0f, Random(40.0f) - 20.0f));
//     ballNode->SetScale(0.5f);
//     auto* ballObject = ballNode->CreateComponent<StaticModel>();
//     ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
//     ballObject->SetMaterial(cache->GetResource<Material>("Materials/StoneSmall.xml"));
//     // Create the physics components
//     auto* body = ballNode->CreateComponent<RigidBody>();
//     body->SetMass(1.0f);
//     body->SetFriction(1.0f);
//     // In addition to friction, use motion damping so that the ball can not accelerate limitlessly
//     body->SetLinearDamping(0.5f);
//     body->SetAngularDamping(0.5f);
//     auto* shape = ballNode->CreateComponent<CollisionShape>();
//     shape->SetSphere(1.0f);
// 
//     // Create a random colored point light at the ball so that can see better where is going
//     auto* light = ballNode->CreateComponent<Light>();
//     light->SetRange(3.0f);
//     light->SetColor(Color(0.5f + ((unsigned)Rand() & 1u) * 0.5f, 0.5f + ((unsigned)Rand() & 1u) * 0.5f, 0.5f + ((unsigned)Rand() & 1u) * 0.5f));
// 
//     return ballNode;
}

void SceneReplication::MoveCamera()
{
    // Right mouse button controls mouse cursor visibility: hide when pressed
    auto* ui = GetSubsystem<UI>();
    auto* input = GetSubsystem<Input>();
    ui->GetCursor()->SetVisible(!input->GetMouseButtonDown(MOUSEB_RIGHT));

    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch and only move the camera
    // when the cursor is hidden
    if (!ui->GetCursor()->IsVisible())
    {
        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, 1.0f, 90.0f);
    }

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    // Only move the camera / show instructions if we have a controllable object
    bool showInstructions = false;
    if (clientObjectID_)
    {
        Node* ballNode = scene_->GetNode(clientObjectID_);
        if (ballNode)
        {
            const float CAMERA_DISTANCE = 5.0f;

            // Move camera some distance away from the ball
            cameraNode_->SetPosition(ballNode->GetPosition() + cameraNode_->GetRotation() * Vector3::BACK * CAMERA_DISTANCE);
            showInstructions = true;
        }
    }

    instructionsText_->SetVisible(showInstructions);
}

void SceneReplication::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    // We only rotate the camera according to mouse movement since last frame, so do not need the time step
    MoveCamera();
    ui_scene_->render(ui_renderder_, cocos2d::Mat4::IDENTITY, nullptr);
}

void SceneReplication::HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData)
{
    // This function is different on the client and server. The client collects controls (WASD controls + yaw angle)
    // and sets them to its server connection object, so that they will be sent to the server automatically at a
    // fixed rate, by default 30 FPS. The server will actually apply the controls (authoritative simulation.)
    auto* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();

    // Client: collect controls
    if (serverConnection)
    {
        auto* ui = GetSubsystem<UI>();
        auto* input = GetSubsystem<Input>();
        Controls controls;

        // Copy mouse yaw
        controls.yaw_ = yaw_;

        // Only apply WASD controls if there is no focused UI element
        if (!ui->GetFocusElement())
        {
            controls.Set(race::CTRL_FORWARD, input->GetKeyDown(KEY_W));
            controls.Set(race::CTRL_BACK, input->GetKeyDown(KEY_S));
            controls.Set(race::CTRL_LEFT, input->GetKeyDown(KEY_A));
            controls.Set(race::CTRL_RIGHT, input->GetKeyDown(KEY_D));
			controls.Set(race::CTRL_JUMP, input->GetKeyDown(KEY_SPACE));
        }

        serverConnection->SetControls(controls);
        // In case the server wants to do position-based interest management using the NetworkPriority components, we should also
        // tell it our observer (camera) position. In this sample it is not in use, but eg. the NinjaSnowWar game uses it
        serverConnection->SetPosition(cameraNode_->GetPosition());
    }
    // Server: apply controls to client objects
    else if (network->IsServerRunning())
    {
        const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();

        for (unsigned i = 0; i < connections.Size(); ++i)
        {
            Connection* connection = connections[i];
			auto avatar = serverObjects_[connection]->GetAvatar();
            // Get the object this connection is controlling
            Node* ballNode = avatar->GetNode();
            if (!ballNode)
                continue;

            auto* body = ballNode->GetComponent<RigidBody>();

            // Get the last controls sent by the client
            const Controls& controls = connection->GetControls();
			//ApplyContrlToNode(ballNode, controls, avatar->GetRoleId());
			avatar->ApplyContrlToNode(controls);
            // Torque is relative to the forward vector
//             Quaternion rotation(0.0f, controls.yaw_, 0.0f);
// 
//             const float MOVE_TORQUE = 3.0f;
// 
//             // Movement torque is applied before each simulation step, which happen at 60 FPS. This makes the simulation
//             // independent from rendering framerate. We could also apply forces (which would enable in-air control),
//             // but want to emphasize that it's a ball which should only control its motion by rolling along the ground
//             if (controls.buttons_ & CTRL_FORWARD)
//                 body->ApplyTorque(rotation * Vector3::RIGHT * MOVE_TORQUE);
//             if (controls.buttons_ & CTRL_BACK)
//                 body->ApplyTorque(rotation * Vector3::LEFT * MOVE_TORQUE);
//             if (controls.buttons_ & CTRL_LEFT)
//                 body->ApplyTorque(rotation * Vector3::FORWARD * MOVE_TORQUE);
//             if (controls.buttons_ & CTRL_RIGHT)
//                 body->ApplyTorque(rotation * Vector3::BACK * MOVE_TORQUE);
        }
    }
}

void SceneReplication::HandleConnect(StringHash eventType, VariantMap& eventData)
{
    auto* network = GetSubsystem<Network>();
    String address = textEdit_->GetText().Trimmed();
    if (address.Empty())
        address = "localhost"; // Use localhost to connect if nothing else specified

    // Connect to server, specify scene to use as a client for replication
    clientObjectID_ = 0; // Reset own object ID from possible previous connection
    network->Connect(address, SERVER_PORT, scene_);

    UpdateButtons();
}

void SceneReplication::HandleDisconnect(StringHash eventType, VariantMap& eventData)
{
    auto* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();
    // If we were connected to server, disconnect. Or if we were running a server, stop it. In both cases clear the
    // scene of all replicated content, but let the local nodes & components (the static world + camera) stay
    if (serverConnection)
    {
        serverConnection->Disconnect();
        scene_->Clear(true, false);
        clientObjectID_ = 0;
    }
    // Or if we were running a server, stop it
    else if (network->IsServerRunning())
    {
        network->StopServer();
        scene_->Clear(true, false);
    }

    UpdateButtons();
}

void SceneReplication::HandleStartServer(StringHash eventType, VariantMap& eventData)
{
    auto* network = GetSubsystem<Network>();
    network->StartServer(SERVER_PORT);

    UpdateButtons();
}

void SceneReplication::HandleConnectionStatus(StringHash eventType, VariantMap& eventData)
{
    UpdateButtons();
}

void SceneReplication::HandleClientConnected(StringHash eventType, VariantMap& eventData)
{
    using namespace ClientConnected;

    // When a client connects, assign to scene to begin scene replication
    auto* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
    newConnection->SetScene(scene_);

    // Then create a controllable object for that client
    auto* newObject = CreatePlayer(newConnection);
    serverObjects_[newConnection] = newObject;

    // Finally send the object's node ID using a remote event
    VariantMap remoteEventData;
    remoteEventData[P_ID] = newObject->GetUrhoID();
    newConnection->SendRemoteEvent(E_CLIENTOBJECTID, true, remoteEventData);
}

void SceneReplication::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
    using namespace ClientConnected;

    // When a client disconnects, remove the controlled object
    auto* connection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
    auto player = serverObjects_[connection];
	Node* object = player->GetAvatar()->GetNode();
    if (object)
        object->Remove();

    serverObjects_.Erase(connection);
}

void SceneReplication::HandleClientObjectID(StringHash eventType, VariantMap& eventData)
{
    clientObjectID_ = eventData[P_ID].GetUInt();
}
