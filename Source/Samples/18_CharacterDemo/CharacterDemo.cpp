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

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include "CEGUI/SchemeManager.h"
#include "CEGUI/WindowManager.h"
#include "CEGUI/widgets/PushButton.h"
#include "CEGUI/Event.h"
#include <Urho3D/Gui/Gui.h>

#include "Mover.h"
#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"
#include "Player.h"
#include <Urho3D/DebugNew.h>

#ifdef SendMessage
#undef SendMessage
#endif

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr unsigned int VIEW_MASK_FLOOR = 0;
const unsigned short SERVER_PORT = 2345;

SkillTip::SkillTip(Scene* scene, CEGUI::Window* tip)
	: scene_{ scene }, tip_{ tip }
{
}

SkillTip::~SkillTip()
{
}

void SkillTip::Init(const CEGUI::String& content, float duration)
{
	tip_->setText(content);
	tip_->setVisible(true);
	tip_->setAlpha(1.0f);
	start_time_ = scene_->GetSubsystem<Time>()->GetElapsedTime();
	duration_ = duration;
	active_ = true;
}

void SkillTip::Update(float elapsedTime)
{
	if (!active_) {
		return;
	}
	float delta_time = elapsedTime - start_time_;
	if (delta_time >= duration_) {
		tip_->setVisible(false);
		active_ = false;
	} else {
		tip_->setAlpha((1.0f - delta_time / duration_));
	}
}

Barrier::Barrier(Scene* scene, const Vector3& pos, float size, float duration)
	: scene_{ scene }, pos_{ pos }, size_{ size }, duration_{ duration }
{
	Node* objectNode = scene->CreateChild("Box");
	objectNode->SetPosition(pos);
	objectNode->SetScale(size);
	auto* object = objectNode->CreateComponent<StaticModel>();
	auto* cache = scene->GetSubsystem<ResourceCache>();
	object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	auto mtl = cache->GetResource<Material>("Materials/GreenTransparent.xml");
	object->SetMaterial(mtl->Clone());
	mtl_ = object->GetMaterial();
	mtl_->SetShaderParameter("MatDiffColor", Color{ 1.0f, 1.0f, 1.0f, 0.55f });
	//object->SetCastShadows(true);

	auto* body = objectNode->CreateComponent<RigidBody>();
	body->SetCollisionLayer(2);
	// Bigger boxes will be heavier and harder to move
	//body->SetMass(scale * 2.0f);
	auto* shape = objectNode->CreateComponent<CollisionShape>();
	shape->SetBox(Vector3::ONE);

	born_time_ = scene->GetSubsystem<Time>()->GetElapsedTime();
	node_ = objectNode;
	active_ = true;
}

Barrier::~Barrier()
{
	Destory();
}

void Barrier::Destory()
{
	active_ = false;
	if (node_) {
		scene_->RemoveChild(node_);
		node_ = nullptr;
		mtl_ = nullptr;
	}
}

void Barrier::Update(float elapsedTime)
{
	auto current_time = elapsedTime - born_time_;
	
	if (current_time > duration_) {
		Destory();
	} else {
		float fade_factor = current_time / duration_;
		if (mtl_) {
			mtl_->SetShaderParameter("MatDiffColor", Color{ 1.0f, 1.0f, 1.0f, 0.15f + 0.6f * (1.0f - fade_factor) });
		}
	}
}

Racetrack::Racetrack(Scene* scene)
	: scene_{ scene }
{
	grid_width_ = width_ * 1.0f / (float)grid_count_;
	int loop_count = length_ / grid_width_;
	cells_.resize(loop_count);
	for (int i = 0; i < loop_count; i++)
	{
		cells_[i].resize(count_ * grid_count_);
	}
}

void Racetrack::Cell::CreateBarrier(Scene* scene, const Vector3& pos, float size, float duration)
{
	barrier_ = std::make_unique<Barrier>(scene, pos, size, duration);
}

void Racetrack::Cell::DestoryBarrier()
{
	barrier_ = nullptr;
}

Racetrack::Cell* Racetrack::LocateCell(const Vector3& worldPos)
{
	float min_x = -count_ * 0.5 * width_;
	float max_x = count_ * 0.5 * width_;
	if (worldPos.x_ < min_x || worldPos.x_ > max_x
		|| worldPos.z_ < 0.0f || worldPos.z_ > length_) {
		return nullptr;
	}
	int x_index = (worldPos.x_ - min_x) / grid_width_;
	int z_index = worldPos.z_ / grid_width_;
	return &cells_[z_index][x_index];
}

void Racetrack::Cell::Update(float elapsedTime)
{
	if (barrier_) {
		barrier_->Update(elapsedTime);
	}
}

void Racetrack::Update(float elapsedTime)
{
	for (auto& rowcell: cells_) {
		for (auto& cell : rowcell) {
			cell.Update(elapsedTime);
		}
	}
}

void Racetrack::CreateBarrier(const Vector3& worldPos)
{
	Vector3 pos = worldPos;
	float min_x = -count_ * 0.5 * width_;
	float max_x = count_ * 0.5 * width_;
	pos.x_ -= fmod(worldPos.x_ - min_x, grid_width_);
	pos.z_ -= fmod(worldPos.z_, grid_width_);
	pos += Vector3{ 0.5f * grid_width_, 0.0f, 0.5f * grid_width_ };
	pos.y_ = grid_width_ * 0.5f;
	auto cell = LocateCell(worldPos);
	if (cell) {
		cell->CreateBarrier(scene_, pos, grid_width_ * 0.95f, 4.0f);
	}
}

void Racetrack::DestoryBarrier(const Vector3& worldPos)
{
	auto cell = LocateCell(worldPos);
	if (cell) {
		cell->DestoryBarrier();
	}
}

void Racetrack::UpdateRacetrack()
{
	static DebugRenderer* debug_ = scene_->GetComponent<DebugRenderer>();
	// racetrack
	Vector3 bias(0.0f, 0.05f, 0.0f);
	Vector3 left_start_point{ 0.0f, bias.y_, 0.0f };
	Vector3 left_end_point{ 0.0f, bias.y_, length_ };
	Vector3 right_start_point{ 0.0f, bias.y_, 0.0f };
	Vector3 right_end_point{ 0.0f, bias.y_, length_ };
	auto half_width = 0.5f * width_;
	left_start_point.x_ -= half_width;
	left_end_point.x_ -= half_width;
	right_start_point.x_ += half_width;
	right_end_point.x_ += half_width;
	Color outside_color{ 1.0f, 0.0f, 0.0f };
	debug_->AddLine(left_start_point, left_end_point, outside_color);
	debug_->AddLine(right_start_point, right_end_point, outside_color);
	auto loop_count = (count_ - 1) / 2;
	for (int i = 0; i < loop_count; i++)
	{
		left_start_point.x_ -= width_;
		left_end_point.x_ -= width_;
		right_start_point.x_ += width_;
		right_end_point.x_ += width_;
		debug_->AddLine(left_start_point, left_end_point, outside_color);
		debug_->AddLine(right_start_point, right_end_point, outside_color);
	}
	//
	debug_->AddLine(left_start_point, right_start_point, outside_color);
	debug_->AddLine(left_end_point, right_end_point, outside_color);
	//
	auto old_left_start_point = left_start_point;
	Color inside_color{ 0.5f, 1.0f, 0.5f };
	auto inside_line_count = (int)(length_ / grid_width_);
	for (int i = 1; i < inside_line_count; i++)
	{
		left_start_point.z_ += grid_width_; right_start_point.z_ += grid_width_;
		debug_->AddLine(left_start_point, right_start_point, inside_color);
	}

	left_start_point = old_left_start_point;
	inside_line_count = grid_count_ * count_;
	for (int i = 0; i < inside_line_count; i++)
	{
		if (i % grid_count_ != 0)
		{
			debug_->AddLine(left_start_point, left_end_point, inside_color);
		}
		left_start_point.x_ += grid_width_; left_end_point.x_ += grid_width_;
	}
}

CharacterDemo::CharacterDemo(Context* context) :
    Sample(context),
    firstPerson_(false)
{
    // Register factory and attributes for the Character component so it can be created via CreateComponent, and loaded / saved
    Character::RegisterObject(context);
	context->RegisterFactory<Mover>();

	FILE* new_file;
	AllocConsole();
	freopen_s(&new_file, "CONIN$", "r", stdin);
	freopen_s(&new_file, "CONOUT$", "w", stdout);
	freopen_s(&new_file, "CONOUT$", "w", stderr);
	SetConsoleOutputCP(CP_UTF8);
}

CharacterDemo::~CharacterDemo() = default;

void CharacterDemo::Setup()
{
	Sample::Setup();
	engineParameters_[EP_WINDOW_WIDTH] = WINDOW_WIDTH;
	engineParameters_[EP_WINDOW_HEIGHT] = WINDOW_HEIGHT;
	//engineParameters_[EP_TOUCH_EMULATION] = true;
}

void CharacterDemo::Start()
{
    // Execute base class startup
    Sample::Start();
    if (touchEnabled_)
        touch_ = new Touch(context_, TOUCH_SENSITIVITY);

    // Create static scene content
    CreateScene();

    // Create the controllable character
    CreateCharacter();

    // Create the UI content
    CreateInstructions();

    // Subscribe to necessary events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_ABSOLUTE/*MM_RELATIVE*/);
	auto* input = GetSubsystem<Input>();
	input->SetMouseVisible(true);
	if (!ConnectServer()) {
		printf("connect server failed.\n");
	}
}

void CharacterDemo::Stop()
{
	FreeConsole();
	Sample::Stop();
}

bool CharacterDemo::ConnectServer()
{
	auto* network = GetSubsystem<Network>();
	String address = "";// textEdit_->GetText().Trimmed();
	if (address.Empty())
		address = "localhost"; // Use localhost to connect if nothing else specified
	// Empty the text edit after reading the address to connect to
	//textEdit_->SetText(String::EMPTY);

	// Connect to server, do not specify a client scene as we are not using scene replication, just messages.
	// At connect time we could also send identity parameters (such as username) in a VariantMap, but in this
	// case we skip it for simplicity
	return network->Connect(address, SERVER_PORT, nullptr);
}

void CharacterDemo::SendCommand(const unsigned char* data, unsigned int len)
{
// 	String text = textEdit_->GetText();
// 	if (text.Empty())
// 		return; // Do not send an empty message

	auto* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	if (serverConnection)
	{
		// A VectorBuffer object is convenient for constructing a message to send
// 		VectorBuffer msg;
// 		msg.WriteString(text);
		// Send the chat message as in-order and reliable
		URHO3D_LOGINFOF("	SendCommand %d byte data to server.");
		serverConnection->SendMessage(MSG_CHAT, true, true, data, len);
		// Empty the text edit after sending
//		textEdit_->SetText(String::EMPTY);
	}
}

race::Player* CharacterDemo::AddPlayer(int playerId, const std::string& nickName, int roleId, int trackId)
{
	auto new_player = race_room_->AddPlayer(playerId, nickName);
	new_player->SetScene(scene_);
	new_player->SetRemoteTrackId(trackId);
	int localTrackId;
	if (my_player_ == nullptr) {
		localTrackId = kTrackCount / 2;
	} else {
		localTrackId = GetLocalTrackId(trackId, kTrackCount);
	}
	auto track = race_room_->GetTrack(localTrackId);
	track->SetPlayer(new_player);
	new_player->SetTrack(track);
	new_player->SetRoleId(roleId);
	return new_player;
}

void CharacterDemo::DelPlayer(int playerId)
{
	auto player = race_room_->FindPlayer(playerId);
	if (player) {
		player->LeaveScene();
		race_room_->DelPlayer(playerId);
	}
}

void CharacterDemo::HandleNetworkMessage(StringHash /*eventType*/, VariantMap& eventData)
{
	auto* network = GetSubsystem<Network>();

	using namespace NetworkMessage;

	int msgID = eventData[P_MESSAGEID].GetInt();
	if (msgID == MSG_CHAT)
	{
		const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
		// Use a MemoryBuffer to read the message data so that there is no unnecessary copying
		MemoryBuffer msg(data);
// 		String text = msg.ReadString();
// 
// 		// If we are the server, prepend the sender's IP address and port and echo to everyone
// 		// If we are a client, just display the message
// 		if (network->IsServerRunning())
// 		{
// 			auto* sender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
// 
// 			text = sender->ToString() + " " + text;
// 
// 			VectorBuffer sendMsg;
// 			sendMsg.WriteString(text);
// 			// Broadcast as in-order and reliable
// 			network->BroadcastMessage(MSG_CHAT, true, true, sendMsg);
// 		}

		auto pdata = (message::MessageHead*)msg.GetData();
		auto src_id = pdata->src;
		auto dst_id = pdata->dst;
		switch (pdata->id)
		{
		case message::MessageId::kPlayerId:
		{
			auto realmsg = (message::PlayerId*)pdata;
			player_id_ = realmsg->head.src;
			race::TrackInfo ti;
			race_room_ = std::make_unique<race::Room>(ti);
			race_room_->Init(5);
			// my player
			for (int i = 0; i < kTrackCount; i++) {
				auto player_id = realmsg->other_player_id[i];
				if (player_id == player_id_) {
					std::string nick_name = "Player_";
					nick_name += std::to_string(player_id);
					my_player_ = AddPlayer(player_id, nick_name, realmsg->other_role_id[i], realmsg->other_track_id[i]);
					character_ = my_player_->GetCharacter();
					break;
				}
			}
			// other player
			for (int i = 0; i < kTrackCount; i++) {
				auto player_id = realmsg->other_player_id[i];
				auto role_id = realmsg->other_role_id[i];
				auto track_id = realmsg->other_track_id[i];
				if (player_id != -1 && player_id != player_id_) {
					std::string nick_name = "Player_";
					nick_name += std::to_string(player_id);
					AddPlayer(player_id, nick_name, role_id, track_id);
				}
			}
		}
		break;
		case message::MessageId::kEnterRoom:
		{
			auto realmsg = (message::Enter*)pdata;
			auto player_id = realmsg->head.src;
			if (player_id_ != player_id) {
// 				std::string nick_name = "Player_";
// 				nick_name += std::to_string(player_id);

				URHO3D_LOGINFOF("player[%d] join the game", player_id);
				CEGUI::String tips = U"玩家";
				tips += U"[";
// 				std::string nick_name;
// 				nick_name.assign(realmsg->head.src_nick_name, 0, 16);
// 				tips += CEGUI::String::convertUtf8ToUtf32(nick_name);
				tips += U"]<";
				tips += std::to_string(player_id);
				tips += U">进入游戏";
				skill_tip_->Init(tips);

				std::string nick_name = "Player_";
				nick_name += std::to_string(player_id);
				AddPlayer(player_id, nick_name, realmsg->head.src_role_id, realmsg->track_id);
			}
		}
		break;
		case message::MessageId::kLeaveRoom:
		{
			auto realmsg = (message::Leave*)pdata;
			URHO3D_LOGINFOF("player[%d] leave the game", realmsg->head.src);
			CEGUI::String tips = U"玩家[";
			tips += std::to_string(realmsg->head.src);
			tips += U"]离开游戏";
			skill_tip_->Init(tips);
			if (player_id_ != realmsg->head.src) {
				DelPlayer(realmsg->head.src);
			}
		}
		break;
		case message::MessageId::kFastPlayer:
		{
			auto realmsg = (message::Fast*)pdata;
			URHO3D_LOGINFOF("player[%d] cast fast", realmsg->head.src);
			CEGUI::String tips = U"玩家[";
			tips += std::to_string(realmsg->head.src);
			tips += U"]开启加速";
			skill_tip_->Init(tips);
		}
		break;
		case message::MessageId::kSlowPlayer:
		{
			auto realmsg = (message::Fast*)pdata;
			URHO3D_LOGINFOF("player[%d] cast slow to player[%d]", realmsg->head.src, realmsg->head.dst);
			CEGUI::String tips = U"玩家[";
			tips += std::to_string(realmsg->head.src);
			tips += U"]对玩家[";
			tips += std::to_string(realmsg->head.dst);
			tips += U"]使用减速";
			skill_tip_->Init(tips);
		}
		break;
		case message::MessageId::kBarrier:
		{
			auto realmsg = (message::Barrier*)pdata;
			URHO3D_LOGINFOF("player[%d] cast barrier to player[%d]", realmsg->head.src, realmsg->head.dst);
			CEGUI::String tips = U"玩家[";
			tips += std::to_string(realmsg->head.src);
			tips += U"]对玩家[";
			tips += std::to_string(realmsg->head.dst);
			tips += U"]使用障碍";
			skill_tip_->Init(tips);
		}
		break;
		case message::MessageId::kFreeze:
		{
			auto realmsg = (message::Freeze*)pdata;
			URHO3D_LOGINFOF("player[%d] cast freeze to player[%d]", realmsg->head.src, realmsg->head.dst);
			CEGUI::String tips = U"玩家[";
			tips += std::to_string(realmsg->head.src);
			tips += U"]对玩家[";
			tips += std::to_string(realmsg->head.dst);
			tips += U"]使用冰冻";
			skill_tip_->Init(tips);
		}
		break;
		case message::MessageId::kBlink:
		{
			auto realmsg = (message::Freeze*)pdata;
			URHO3D_LOGINFOF("player[%d] cast blink", realmsg->head.src);
			CEGUI::String tips = U"玩家[";
			tips += std::to_string(realmsg->head.src);
			tips += U"]开启闪现";
			skill_tip_->Init(tips);
		}
		break;
		case message::MessageId::kUpdateLocation:
		{

		}
		break;
		default:
			break;
		}

	}
}
void CharacterDemo::CreateScene()
{
    auto* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create scene subsystem components
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<PhysicsWorld>();
	auto debug = scene_->CreateComponent<DebugRenderer>();
	debug->SetLineAntiAlias(true);
	racetrack_ = std::make_unique<Racetrack>(scene_);

    // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
    // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
    cameraNode_ = new Node(context_);
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);
    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
	Vector3 camera_dir{0.0f, -1.0f, 1.0f};
	camera_dir.Normalize();
	float camera_dist = 40.0f;
	Vector3 target_pos{ 0.0f, 0.0f, 15.0f };
	Vector3 camera_pos = -camera_dist * camera_dir + target_pos;
	cameraNode_->SetPosition(camera_pos);
	cameraNode_->LookAt(target_pos);

    // Create static scene content. First create a zone for ambient lighting and fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

    // Create a directional light with cascaded shadow mapping
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);

    // Create the floor object
    Node* floorNode = scene_->CreateChild("Floor");
    floorNode->SetPosition(Vector3(0.0f, -0.5f, 0.0f));
    floorNode->SetScale(Vector3(200.0f, 1.0f, 200.0f));
	floor_ = floorNode->CreateComponent<StaticModel>();
	floor_->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	floor_->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	//object->SetViewMask(VIEW_MASK_FLOOR);

    auto* body = floorNode->CreateComponent<RigidBody>();
    // Use collision layer bit 2 to mark world scenery. This is what we will raycast against to prevent camera from going
    // inside geometry
    body->SetCollisionLayer(2);
	//body->SetCollisionMask(VIEW_MASK_FLOOR);
    auto* shape = floorNode->CreateComponent<CollisionShape>();
    shape->SetBox(Vector3::ONE);

    // Create mushrooms of varying sizes
    const unsigned NUM_MUSHROOMS = 0;
    for (unsigned i = 0; i < NUM_MUSHROOMS; ++i)
    {
        Node* objectNode = scene_->CreateChild("Mushroom");
        objectNode->SetPosition(Vector3(Random(180.0f) - 90.0f, 0.0f, Random(180.0f) - 90.0f));
        objectNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
        objectNode->SetScale(2.0f + Random(5.0f));
        auto* object = objectNode->CreateComponent<StaticModel>();
        object->SetModel(cache->GetResource<Model>("Models/Mushroom.mdl"));
        object->SetMaterial(cache->GetResource<Material>("Materials/Mushroom.xml"));
        object->SetCastShadows(true);

        auto* body = objectNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2);
        auto* shape = objectNode->CreateComponent<CollisionShape>();
        shape->SetTriangleMesh(object->GetModel(), 0);
    }

    // Create movable boxes. Let them fall from the sky at first
    const unsigned NUM_BOXES = 0;
    for (unsigned i = 0; i < NUM_BOXES; ++i)
    {
        float scale = Random(2.0f) + 0.5f;

        Node* objectNode = scene_->CreateChild("Box");
        objectNode->SetPosition(Vector3(Random(180.0f) - 90.0f, Random(10.0f) + 10.0f, Random(180.0f) - 90.0f));
        objectNode->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
        objectNode->SetScale(scale);
        auto* object = objectNode->CreateComponent<StaticModel>();
        object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		//object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
		object->SetMaterial(cache->GetResource<Material>("Materials/GreenTransparent.xml"));
		object->GetMaterial()->SetShaderParameter("MatDiffColor", Color{ 1.0f, 1.0f, 1.0f, 0.55f});
        object->SetCastShadows(true);

        auto* body = objectNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2);
        // Bigger boxes will be heavier and harder to move
        body->SetMass(scale * 2.0f);
        auto* shape = objectNode->CreateComponent<CollisionShape>();
        shape->SetBox(Vector3::ONE);
    }
}

void CharacterDemo::CreateCharacter()
{
	return;
    auto* cache = GetSubsystem<ResourceCache>();

    Node* objectNode = scene_->CreateChild("Jack");
    objectNode->SetPosition(Vector3(0.0f, 1.0f, 0.0f));

    // spin node
    Node* adjustNode = objectNode->CreateChild("AdjNode");
    adjustNode->SetRotation( Quaternion(180, Vector3(0,1,0) ) );

    // Create the rendering component + animation controller
    auto* object = adjustNode->CreateComponent<AnimatedModel>();
    object->SetModel(cache->GetResource<Model>("Models/Mutant/Mutant.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Models/Mutant/Materials/mutant_M.xml"));
    object->SetCastShadows(true);
    adjustNode->CreateComponent<AnimationController>();

    // Set the head bone for manual control
    object->GetSkeleton().GetBone("Mutant:Head")->animated_ = false;

    // Create rigidbody, and set non-zero mass so that the body becomes dynamic
    auto* body = objectNode->CreateComponent<RigidBody>();
    body->SetCollisionLayer(1);
    body->SetMass(1.0f);

    // Set zero angular factor so that physics doesn't turn the character on its own.
    // Instead we will control the character yaw manually
    body->SetAngularFactor(Vector3::ZERO);

    // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
    body->SetCollisionEventMode(COLLISION_ALWAYS);

    // Set a capsule shape for collision
    auto* shape = objectNode->CreateComponent<CollisionShape>();
    shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));

    // Create the character logic component, which takes care of steering the rigidbody
    // Remember it so that we can set the controls. Use a WeakPtr because the scene hierarchy already owns it
    // and keeps it alive as long as it's not removed from the hierarchy
    character_ = objectNode->CreateComponent<Character>();
	/*
	const unsigned NUM_MODELS = 9;
	const float MODEL_MOVE_SPEED = 2.0f;
	const float MODEL_ROTATE_SPEED = 100.0f;
	const BoundingBox bounds(Vector3(-20.0f, 0.0f, -20.0f), Vector3(20.0f, 0.0f, 20.0f));

	struct ModelRes 
	{
		std::string model;
		std::string mtl;
		std::string ani;
	};
	ModelRes mr[3] = {
		{"Models/Kachujin/Kachujin.mdl", "Models/Kachujin/Materials/Kachujin.xml", "Models/Kachujin/Kachujin_Walk.ani"},
		{"Models/NinjaSnowWar/Ninja.mdl", "Materials/NinjaSnowWar/Ninja.xml", "Models/NinjaSnowWar/Ninja_Walk.ani"},
		{"Models/Jack.mdl", "Models/Jack.xml", "Models/Jack_Walk.ani"},
	};
	for (unsigned i = 0; i < NUM_MODELS; ++i)
	{
		Node* modelNode = scene_->CreateChild("Jill");
		modelNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 0.0f, Random(40.0f) - 20.0f));
		modelNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));

		auto* modelObject = modelNode->CreateComponent<AnimatedModel>();
		auto index = i % 3;
		modelObject->SetModel(cache->GetResource<Model>(mr[index].model.c_str()));
		modelObject->SetMaterial(cache->GetResource<Material>(mr[index].mtl.c_str()));
		modelObject->SetCastShadows(true);

		// Create an AnimationState for a walk animation. Its time position will need to be manually updated to advance the
		// animation, The alternative would be to use an AnimationController component which updates the animation automatically,
		// but we need to update the model's position manually in any case
		auto* walkAnimation = cache->GetResource<Animation>(mr[index].ani.c_str());

		AnimationState* state = modelObject->AddAnimationState(walkAnimation);
		// The state would fail to create (return null) if the animation was not found
		if (state)
		{
			// Enable full blending weight and looping
			state->SetWeight(1.0f);
			state->SetLooped(true);
			state->SetTime(Random(walkAnimation->GetLength()));
		}

		// Create our custom Mover component that will move & animate the model during each frame's update
		auto* mover = modelNode->CreateComponent<Mover>();
		mover->SetParameters(MODEL_MOVE_SPEED, MODEL_ROTATE_SPEED, bounds);
	}
	*/
}

void CharacterDemo::CreateInstructions()
{
	auto gui = GetSubsystem<Gui>();
	CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
	CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
	auto creat_button = [&winMgr, gui](const CEGUI::UVector2& pos, const CEGUI::String& name) {
		auto button = static_cast<CEGUI::PushButton*>(winMgr.createWindow("TaharezLook/Button", name));
		auto root = gui->GetRootWindow();
		root->addChild(button);
		button->setSize(CEGUI::USize(cegui_absdim(88.f), cegui_absdim(34.f)));
		button->setPosition(pos);
		button->setProperty("NormalImage", "TaharezLook/ButtonLeftNormal");
		button->setProperty("HoverImage", "TaharezLook/ButtonLeftHighlight");
		button->setProperty("PushedImage", "TaharezLook/ButtonLeftPushed");
		return button;
	};
	float posx = 300.0f; float posy = 680.0f;
	float step = 120.0f;
	auto button = creat_button(CEGUI::UVector2(cegui_absdim(posx), cegui_absdim(posy)), "enter_room");
	button->setText(CEGUI::String(U"进入房间"));
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CharacterDemo::OnEnterRoom, this));
	posx += step;
	button = creat_button(CEGUI::UVector2(cegui_absdim(posx), cegui_absdim(posy)), "skill_fast");
	button->setText(CEGUI::String(U"加速"));
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CharacterDemo::OnFast, this));
	posx += step;
	button = creat_button(CEGUI::UVector2(cegui_absdim(posx), cegui_absdim(posy)), "skill_blink");
	button->setText(CEGUI::String(U"闪现"));
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CharacterDemo::OnBlink, this));
	posx += step;
	button = creat_button(CEGUI::UVector2(cegui_absdim(posx), cegui_absdim(posy)), "skill_barrier");
	button->setText(CEGUI::String(U"放置障碍"));
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CharacterDemo::OnBarrier, this));
	posx += step;
	button = creat_button(CEGUI::UVector2(cegui_absdim(posx), cegui_absdim(posy)), "skill_slow");
	button->setText(CEGUI::String(U"减速"));
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CharacterDemo::OnSlow, this));
	posx += step;
	button = creat_button(CEGUI::UVector2(cegui_absdim(posx), cegui_absdim(posy)), "skill_freeze");
	button->setText(CEGUI::String(U"冰冻"));
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CharacterDemo::OnFreeze, this));
	button = creat_button(CEGUI::UVector2(cegui_absdim(posx), cegui_absdim(posy)), "leave_room");
	posx += step;
	button->setText(CEGUI::String(U"离开房间"));
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CharacterDemo::OnLeaveRoom, this));

	auto root = gui->GetRootWindow();
	auto text = winMgr.createWindow("TaharezLook/StaticText", "skill_tip");
	root->addChild(text);
	text->setFont("FZZYJ-14");
	text->setSize(CEGUI::USize(cegui_absdim(300.0f), cegui_absdim(34.0f)));
// 	text->setAdjustWidthToContent(true);
// 	text->setAdjustHeightToContent(true);
	text->setText(U"惟草木之零落兮，恐美人之迟暮。则为你如花美眷，似水流年。");
	auto width = text->getWidth();
	text->setPosition(CEGUI::UVector2(cegui_reldim(0.5f) - cegui_absdim(150.0f), cegui_reldim(0.5f)));
	skill_tip_ = std::make_unique<SkillTip>(scene_, text);
	
}

bool CharacterDemo::OnEnterRoom(const CEGUI::EventArgs& args)
{
	message::Enter enter;
	enter.head.src = enter.head.dst = player_id_;
	enter.room_id = 0;
	URHO3D_LOGINFOF("EnterRoom %d.", enter.room_id);
	SendCommand((const unsigned char*)&enter, sizeof(enter));
	return true;
}
bool CharacterDemo::OnLeaveRoom(const CEGUI::EventArgs& args)
{
	message::Leave leave;
	leave.head.src = leave.head.dst = player_id_;
	leave.room_id = 0;
	URHO3D_LOGINFOF("LeaveRoom %d.", leave.room_id);
	SendCommand((const unsigned char*)&leave, sizeof(leave));
	return true;
}
bool CharacterDemo::OnFreeze(const CEGUI::EventArgs& args)
{
	current_message_ = message::MessageId::kFreeze;
	return true;
}
bool CharacterDemo::OnBarrier(const CEGUI::EventArgs& args)
{
	current_message_ = message::MessageId::kBarrier;
	return true;
}
bool CharacterDemo::OnFast(const CEGUI::EventArgs& args)
{
	cast_fast_.head.src = cast_fast_.head.dst = player_id_;
	cast_fast_.ratio = 1.5f;
	cast_fast_.duration = 2.0f;
	URHO3D_LOGINFOF("cast fast %f, %f.", cast_fast_.ratio, cast_fast_.duration);
	SendCommand((const unsigned char*)&cast_fast_, sizeof(cast_fast_));
	return true;
}
bool CharacterDemo::OnSlow(const CEGUI::EventArgs& args)
{
	current_message_ = message::MessageId::kSlowPlayer;
	return true;
}
bool CharacterDemo::OnBlink(const CEGUI::EventArgs& args)
{
	cast_blink_.head.src = cast_blink_.head.dst = player_id_;
	cast_blink_.distance = 2.0f;
	URHO3D_LOGINFOF("cast blink %f.", cast_blink_.distance);
	SendCommand((const unsigned char*)&cast_blink_, sizeof(cast_blink_));
	return true;
}
int CharacterDemo::GetLocalTrackId(int remoteTrackId, int maxTrack)
{
	assert(remoteTrackId >= 0 && remoteTrackId < maxTrack);
	auto trackId = (int)(maxTrack / 2) + (remoteTrackId - my_player_->GetRemoteTrackId());
	if (trackId < 0) {
		trackId += maxTrack;
	}
	trackId %= maxTrack;
	return trackId;
}
void CharacterDemo::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);

	SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(CharacterDemo, HandleMouseButtonDown));
	SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(CharacterDemo, HandleMouseButtonUp));
	//SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(CharacterDemo, HandleMouseMove));
	SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(CharacterDemo, HandleMouseWheel));

	// Subscribe HandlePostRenderUpdate() function for processing the post-render update event, sent after Renderer subsystem is
	// done with defining the draw calls for the viewports (but before actually executing them.) We will request debug geometry
	// rendering during that event
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostRenderUpdate));

	SubscribeToEvent(E_NETWORKMESSAGE, URHO3D_HANDLER(CharacterDemo, HandleNetworkMessage));
}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;
	
    auto* input = GetSubsystem<Input>();

    if (character_)
    {
        // Clear previous controls
        character_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT | CTRL_JUMP, false);

        // Update controls using touch utility class
        if (touch_)
            touch_->UpdateTouches(character_->controls_);

        // Update controls using keys
        auto* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            if (!touch_ || !touch_->useGyroscope_)
            {
                character_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
                character_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
                character_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
                character_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
            }
            character_->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));

            // Add character yaw & pitch from the mouse motion or touch input
            if (touchEnabled_)
            {
                for (unsigned i = 0; i < input->GetNumTouches(); ++i)
                {
                    TouchState* state = input->GetTouch(i);
                    if (!state->touchedElement_)    // Touch on empty space
                    {
                        auto* camera = cameraNode_->GetComponent<Camera>();
                        if (!camera)
                            return;

                        auto* graphics = GetSubsystem<Graphics>();
                        character_->controls_.yaw_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.x_;
                        character_->controls_.pitch_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.y_;
                    }
                }
            }
            else
            {
//                 character_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
//                 character_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }
            // Limit pitch
            //character_->controls_.pitch_ = Clamp(character_->controls_.pitch_, -80.0f, 80.0f);
            // Set rotation already here so that it's updated every rendering frame instead of every physics frame
            //character_->GetNode()->SetRotation(Quaternion(character_->controls_.yaw_, Vector3::UP));
			
// 			if (target_pos_.DistanceToPoint(character_->GetNode()->GetWorldPosition()) < 0.2f)
// 			{
// 				character_->controls_.Set(CTRL_FORWARD, false);
// 			}
// 			if (!rot_one_time_.IsFinished())
// 			{
// 				auto* time = GetSubsystem<Time>();
// 				auto elapsedTime = time->GetElapsedTime();
// 				auto rot = rot_one_time_.GetValue(elapsedTime);
// 				character_->GetNode()->SetWorldRotation(rot);
// 				//
// 				if (touch_target_pos_)
// 				{
// 					end_rot_ = GetEndRotate();
// 					start_rot_ = character_->GetNode()->GetWorldRotation();
// 					//printf("	start_rot_ : %f end_rot_ : %f\n", start_rot_.Angle(), end_rot_.Angle());
// 					auto old_finished = rot_one_time_.finished_;
// 					rot_one_time_.Init(start_rot_, end_rot_, elapsedTime, rot_one_time_.end_time_ - elapsedTime);
// 					rot_one_time_.finished_ = old_finished;
// 				}
// 			}
			
            // Switch between 1st and 3rd person
            if (input->GetKeyPress(KEY_F))
                firstPerson_ = !firstPerson_;

            // Turn on/off gyroscope on mobile platform
            if (touch_ && input->GetKeyPress(KEY_G))
                touch_->useGyroscope_ = !touch_->useGyroscope_;

            // Check for loading / saving the scene
            if (input->GetKeyPress(KEY_F5))
            {
                File saveFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/CharacterDemo.xml", FILE_WRITE);
                scene_->SaveXML(saveFile);
            }
            if (input->GetKeyPress(KEY_F7))
            {
                File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/CharacterDemo.xml", FILE_READ);
                scene_->LoadXML(loadFile);
                // After loading we have to reacquire the weak pointer to the Character component, as it has been recreated
                // Simply find the character's scene node by name as there's only one of them
                Node* characterNode = scene_->GetChild("Jack", true);
                if (characterNode)
                    character_ = characterNode->GetComponent<Character>();
            }
        }
    }
	auto* time = GetSubsystem<Time>();
	racetrack_->Update(time->GetElapsedTime());
	if (skill_tip_) {
		skill_tip_->Update(time->GetElapsedTime());
	}
}

void CharacterDemo::ThirdPersonCamera()
{
	Node* characterNode = character_->GetNode();
	cameraNode_->SetPosition(characterNode->GetWorldPosition() + Vector3(0.0f, 10.0f, -10.0f));
	cameraNode_->LookAt(characterNode->GetWorldPosition());
}

void CharacterDemo::FreeCamera(float timeStep)
{
	return;
	// Do not move if the UI has a focused element (the console)
	if (GetSubsystem<UI>()->GetFocusElement())
		return;

	auto* input = GetSubsystem<Input>();

	// Movement speed as world units per second
	const float MOVE_SPEED = 20.0f;
	// Mouse sensitivity as degrees per pixel
	const float MOUSE_SENSITIVITY = 0.15f;

	// Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
	IntVector2 mouseMove = input->GetMouseMove();
	yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
	pitch_ = Clamp(pitch_, -90.0f, 90.0f);

	// Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
	//cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

	// Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
	if (input->GetKeyDown(KEY_W))
		cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_S))
		cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_A))
		cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_D))
		cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

	// Toggle debug geometry with space
	if (input->GetKeyPress(KEY_SPACE))
		drawDebug_ = !drawDebug_;
}
void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;
    if (!character_)
        return;
	//ThirdPersonCamera();
	// Take the frame time step, which is stored as a float
	float timeStep = eventData[P_TIMESTEP].GetFloat();
	FreeCamera(timeStep);
	return;
//     Node* characterNode = character_->GetNode();
// 
//     // Get camera lookat dir from character yaw + pitch
//     const Quaternion& rot = characterNode->GetRotation();
//     Quaternion dir = rot * Quaternion(character_->controls_.pitch_, Vector3::RIGHT);
// 
//     // Turn head to camera pitch, but limit to avoid unnatural animation
//     Node* headNode = characterNode->GetChild("Mutant:Head", true);
//     float limitPitch = Clamp(character_->controls_.pitch_, -45.0f, 45.0f);
//     Quaternion headDir = rot * Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
//     // This could be expanded to look at an arbitrary target, now just look at a point in front
//     Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);
//     headNode->LookAt(headWorldTarget, Vector3(0.0f, 1.0f, 0.0f));
//     if (firstPerson_)
//     {
//         cameraNode_->SetPosition(headNode->GetWorldPosition() + rot * Vector3(0.0f, 0.15f, 0.2f));
//         cameraNode_->SetRotation(dir);
//     }
//     else
//     {
//         // Third person camera: position behind the character
//         Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 1.7f, 0.0f);
// 
//         // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
//         Vector3 rayDir = dir * Vector3::BACK;
//         float rayDistance = touch_ ? touch_->cameraDistance_ : CAMERA_INITIAL_DIST;
//         PhysicsRaycastResult result;
//         scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, 2);
//         if (result.body_)
//             rayDistance = Min(rayDistance, result.distance_);
//         rayDistance = Clamp(rayDistance, CAMERA_MIN_DIST, CAMERA_MAX_DIST);
// 
//         cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
//         cameraNode_->SetRotation(dir);
//     }
}

void CharacterDemo::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	// If draw debug mode is enabled, draw viewport debug geometry, which will show eg. drawable bounding boxes and skeleton
	// bones. Note that debug geometry has to be separately requested each frame. Disable depth test so that we can see the
	// bones properly
	if (drawDebug_)
		GetSubsystem<Renderer>()->DrawDebugGeometry(false);
	racetrack_->UpdateRacetrack();
}

Quaternion CharacterDemo::GetEndRotate()
{
	auto ch_pos = character_->GetNode()->GetWorldPosition();
	ch_pos = ch_pos.ProjectOntoPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 1.0f, 0.0f });
	Quaternion end;
	end.FromRotationTo(Vector3::FORWARD, target_pos_ - ch_pos);
	return end;
}
void CharacterDemo::HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
{
// 	message::Enter enter;
// 	enter.room_id = 0;
// 	SendCommand((const unsigned char*)&enter, sizeof(message::Enter));
	

	using namespace MouseButtonDown;

// 	mouseButtons_ = MouseButtonFlags(eventData[P_BUTTONS].GetUInt());
// 	qualifiers_ = QualifierFlags(eventData[P_QUALIFIERS].GetUInt());
// 	usingTouchInput_ = false;

	IntVector2 cursorPos;
	//bool cursorVisible;
	//GetCursorPositionAndVisible(cursorPos, cursorVisible);
	auto* input = GetSubsystem<Input>();
	cursorPos = input->GetMousePosition();
	auto button = MouseButton(eventData[P_BUTTON].GetUInt());
	if (button == MOUSEB_LEFT || button == MOUSEB_RIGHT)
	{
// 		auto* camera = cameraNode_->GetComponent<Camera>();
// 		auto clickRay = camera->GetScreenRay(cursorPos.x_ / WINDOW_WIDTH, cursorPos.x_ / WINDOW_HEIGHT);
// 		static Plane groundPlane{ Vector3{ 0.0f, 1.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };
// 		clickRay.HitDistance(groundPlane);
// 		RayOctreeQuery query{ clickRay , };
// 		scene_->GetComponent<Octree>()->Raycast(query, clickRay);

		auto* graphics = GetSubsystem<Graphics>();
		auto* camera = cameraNode_->GetComponent<Camera>();
		Ray cameraRay = camera->GetScreenRay((float)cursorPos.x_ / graphics->GetWidth(), (float)cursorPos.y_ / graphics->GetHeight());
		// Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
		PODVector<RayQueryResult> results;
		RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 250.0f, DRAWABLE_GEOMETRY);
		//scene_->GetComponent<Octree>()->RaycastSingle(query);
		floor_->ProcessRayQuery(query, results);
// 		PhysicsRaycastResult result;
// 		auto physics_world = scene_->CreateComponent<PhysicsWorld>();
// 		physics_world->RaycastSingle(result, cameraRay, );
		if (!results.Empty())
		{
			target_pos_ = results[0].position_;

			/*
			if (true)//results[0].node_ == scene_->GetChild("Floor")
			{
				target_pos_ = target_pos_.ProjectOntoPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 1.0f, 0.0f });
				//character_->GetNode()->LookAt(target_pos_, Vector3::UP);
				character_->controls_.Set(CTRL_FORWARD, true);
				last_dist_ = target_pos_.DistanceToPoint(character_->GetNode()->GetWorldPosition());

// 				auto ch_pos = character_->GetNode()->GetWorldPosition();
// 				ch_pos = ch_pos.ProjectOntoPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 1.0f, 0.0f });
// 				target_dir_ = target_pos_ - ch_pos;
// 				end_rot_.FromRotationTo(Vector3::FORWARD, target_dir_);
// 
				end_rot_ = GetEndRotate();
 				start_rot_ = character_->GetNode()->GetWorldRotation();
				auto* time = GetSubsystem<Time>();
				rot_one_time_.Init(start_rot_, end_rot_, time->GetElapsedTime(), 0.5f);
			}
			*/
			if (current_message_ != message::MessageId::kCommandCount) {
				target_pos_ = target_pos_.ProjectOntoPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 1.0f, 0.0f });
				auto track = race_room_->GetTrackByPosition(target_pos_);
				if (track) {
					auto player = track->GetPlayer();
					auto src_id = my_player_->GetId();
					auto dst_id = player->GetId();
					if (current_message_ == message::MessageId::kSlowPlayer) {
						message::Slow slow;
						slow.head.src = src_id;
						slow.head.dst = dst_id;
						slow.ratio = 0.8f;
						URHO3D_LOGINFOF("Slow %d -> %d", src_id, dst_id);
						SendCommand((const unsigned char*)&slow, sizeof(slow));
					} else if (current_message_ == message::MessageId::kBarrier) {
						message::Barrier barrier;
						barrier.head.src = src_id;
						barrier.head.dst = dst_id;
						barrier.x_pos = target_pos_.x_;
						barrier.z_pos = target_pos_.z_;
						barrier.duration = 5.0f;
						barrier.width = 2.0f;
						URHO3D_LOGINFOF("Barrier %d -> %d", src_id, dst_id);
						SendCommand((const unsigned char*)&barrier, sizeof(barrier));
					} else if (current_message_ == message::MessageId::kFreeze) {
						message::Freeze freeze;
						freeze.head.src = src_id;
						freeze.head.dst = dst_id;
						freeze.duration = 4.0f;
						URHO3D_LOGINFOF("Freeze %d -> %d", src_id, dst_id);
						SendCommand((const unsigned char*)&freeze, sizeof(freeze));
					}
					current_message_ = message::MessageId::kCommandCount;
				}
			}
			
// 			if (button == MOUSEB_LEFT) {
// 				racetrack_->CreateBarrier(target_pos_);
// 			} else if (button == MOUSEB_RIGHT) {
// 				racetrack_->DestoryBarrier(target_pos_);
// 			}
		}
		
	}
	// Handle drag cancelling
// 	ProcessDragCancel();
// 
// 	auto* input = GetSubsystem<Input>();
// 
// 	if (!input->IsMouseGrabbed())
// 		ProcessClickBegin(cursorPos, MouseButton(eventData[P_BUTTON].GetUInt()), mouseButtons_, qualifiers_, cursor_, cursorVisible);
}

void CharacterDemo::HandleMouseButtonUp(StringHash eventType, VariantMap& eventData)
{
// 	using namespace MouseButtonUp;
// 
// 	mouseButtons_ = MouseButtonFlags(eventData[P_BUTTONS].GetUInt());
// 	qualifiers_ = QualifierFlags(eventData[P_QUALIFIERS].GetUInt());
// 
// 	IntVector2 cursorPos;
// 	bool cursorVisible;
// 	GetCursorPositionAndVisible(cursorPos, cursorVisible);
// 
// 	ProcessClickEnd(cursorPos, (MouseButton)eventData[P_BUTTON].GetUInt(), mouseButtons_, qualifiers_, cursor_, cursorVisible);
}

// void CharacterDemo::HandleMouseMove(StringHash eventType, VariantMap& eventData)
// {
// 	using namespace MouseMove;
// 
// 	mouseButtons_ = MouseButtonFlags(eventData[P_BUTTONS].GetUInt());
// 	qualifiers_ = QualifierFlags(eventData[P_QUALIFIERS].GetUInt());
// 	usingTouchInput_ = false;
// 
// 	auto* input = GetSubsystem<Input>();
// 	const IntVector2& rootSize = rootElement_->GetSize();
// 	const IntVector2& rootPos = rootElement_->GetPosition();
// 
// 	IntVector2 DeltaP = IntVector2(eventData[P_DX].GetInt(), eventData[P_DY].GetInt());
// 
// 	if (cursor_)
// 	{
// 		if (!input->IsMouseVisible())
// 		{
// 			if (!input->IsMouseLocked())
// 				cursor_->SetPosition(IntVector2(eventData[P_X].GetInt(), eventData[P_Y].GetInt()));
// 			else if (cursor_->IsVisible())
// 			{
// 				// Relative mouse motion: move cursor only when visible
// 				IntVector2 pos = cursor_->GetPosition();
// 				pos.x_ += eventData[P_DX].GetInt();
// 				pos.y_ += eventData[P_DY].GetInt();
// 				pos.x_ = Clamp(pos.x_, rootPos.x_, rootPos.x_ + rootSize.x_ - 1);
// 				pos.y_ = Clamp(pos.y_, rootPos.y_, rootPos.y_ + rootSize.y_ - 1);
// 				cursor_->SetPosition(pos);
// 			}
// 		}
// 		else
// 		{
// 			// Absolute mouse motion: move always
// 			cursor_->SetPosition(IntVector2(eventData[P_X].GetInt(), eventData[P_Y].GetInt()));
// 		}
// 	}
// 
// 	IntVector2 cursorPos;
// 	bool cursorVisible;
// 	GetCursorPositionAndVisible(cursorPos, cursorVisible);
// 
// 	ProcessMove(cursorPos, DeltaP, mouseButtons_, qualifiers_, cursor_, cursorVisible);
// }

void CharacterDemo::HandleMouseWheel(StringHash eventType, VariantMap& eventData)
{
// 	auto* input = GetSubsystem<Input>();
// 	if (input->IsMouseGrabbed())
// 		return;
// 
// 	using namespace MouseWheel;
// 
// 	mouseButtons_ = MouseButtonFlags(eventData[P_BUTTONS].GetInt());
// 	qualifiers_ = QualifierFlags(eventData[P_QUALIFIERS].GetInt());
// 	int delta = eventData[P_WHEEL].GetInt();
// 	usingTouchInput_ = false;
// 
// 	IntVector2 cursorPos;
// 	bool cursorVisible;
// 	GetCursorPositionAndVisible(cursorPos, cursorVisible);
// 
// 	if (!nonFocusedMouseWheel_ && focusElement_)
// 		focusElement_->OnWheel(delta, mouseButtons_, qualifiers_);
// 	else
// 	{
// 		// If no element has actual focus or in non-focused mode, get the element at cursor
// 		if (cursorVisible)
// 		{
// 			UIElement* element = GetElementAt(cursorPos);
// 			if (nonFocusedMouseWheel_)
// 			{
// 				// Going up the hierarchy chain to find element that could handle mouse wheel
// 				while (element && !element->IsWheelHandler())
// 				{
// 					element = element->GetParent();
// 				}
// 			}
// 			else
// 				// If the element itself is not focusable, search for a focusable parent,
// 				// although the focusable element may not actually handle mouse wheel
// 				element = GetFocusableElement(element);
// 
// 			if (element && (nonFocusedMouseWheel_ || element->GetFocusMode() >= FM_FOCUSABLE))
// 				element->OnWheel(delta, mouseButtons_, qualifiers_);
// 		}
// 	}
}

