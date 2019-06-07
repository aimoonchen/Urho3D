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

#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/IOEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/IO/VectorBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

#include "Chat.h"
#include "../18_CharacterDemo/NetMessage.h"
#include <Urho3D/DebugNew.h>
#include <SLikeNet/MessageIdentifiers.h>

// Undefine Windows macro, as our Connection class has a function called SendMessage
#ifdef SendMessage
#undef SendMessage
#endif

// UDP port we will use
const unsigned short CHAT_SERVER_PORT = 2345;

URHO3D_DEFINE_APPLICATION_MAIN(Chat)

Chat::Chat(Context* context) :
    Sample(context)
{
}

void Chat::Start()
{
    // Execute base class startup
    Sample::Start();

    // Enable OS cursor
    GetSubsystem<Input>()->SetMouseVisible(true);

    // Create the user interface
    CreateUI();

    // Subscribe to UI and network events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_FREE);
}

void Chat::CreateUI()
{
    SetLogoVisible(false); // We need the full rendering window

    auto* graphics = GetSubsystem<Graphics>();
    UIElement* root = GetSubsystem<UI>()->GetRoot();
    auto* cache = GetSubsystem<ResourceCache>();
    auto* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Set style to the UI root so that elements will inherit it
    root->SetDefaultStyle(uiStyle);

    auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
    chatHistoryText_ = root->CreateChild<Text>();
    chatHistoryText_->SetFont(font, 12);

    buttonContainer_ = root->CreateChild<UIElement>();
    buttonContainer_->SetFixedSize(graphics->GetWidth(), 20);
    buttonContainer_->SetPosition(0, graphics->GetHeight() - 20);
    buttonContainer_->SetLayoutMode(LM_HORIZONTAL);

    textEdit_ = buttonContainer_->CreateChild<LineEdit>();
    textEdit_->SetStyleAuto();

    sendButton_ = CreateButton("Send", 70);
    connectButton_ = CreateButton("Connect", 90);
    disconnectButton_ = CreateButton("Disconnect", 100);
    startServerButton_ = CreateButton("Start Server", 110);

    UpdateButtons();

    float rowHeight = chatHistoryText_->GetRowHeight();
    // Row height would be zero if the font failed to load
    if (rowHeight)
    {
        float numberOfRows = (graphics->GetHeight() - 100) / rowHeight;
        chatHistory_.Resize(static_cast<unsigned int>(numberOfRows));
    }

    // No viewports or scene is defined. However, the default zone's fog color controls the fill color
    GetSubsystem<Renderer>()->GetDefaultZone()->SetFogColor(Color(0.0f, 0.0f, 0.1f));
}

void Chat::SubscribeToEvents()
{
    // Subscribe to UI element events
    SubscribeToEvent(textEdit_, E_TEXTFINISHED, URHO3D_HANDLER(Chat, HandleSend));
    SubscribeToEvent(sendButton_, E_RELEASED, URHO3D_HANDLER(Chat, HandleSend));
    SubscribeToEvent(connectButton_, E_RELEASED, URHO3D_HANDLER(Chat, HandleConnect));
    SubscribeToEvent(disconnectButton_, E_RELEASED, URHO3D_HANDLER(Chat, HandleDisconnect));
    SubscribeToEvent(startServerButton_, E_RELEASED, URHO3D_HANDLER(Chat, HandleStartServer));

    // Subscribe to log messages so that we can pipe them to the chat window
    SubscribeToEvent(E_LOGMESSAGE, URHO3D_HANDLER(Chat, HandleLogMessage));

    // Subscribe to network events
    SubscribeToEvent(E_NETWORKMESSAGE, URHO3D_HANDLER(Chat, HandleNetworkMessage));
    SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(Chat, HandleConnectionStatus));
    SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(Chat, HandleConnectionStatus));
    SubscribeToEvent(E_CONNECTFAILED, URHO3D_HANDLER(Chat, HandleConnectionStatus));

	SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(Chat, HandleClientDisconnect));
}

Button* Chat::CreateButton(const String& text, int width)
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

void Chat::ShowChatText(const String& row)
{
    chatHistory_.Erase(0);
    chatHistory_.Push(row);

    // Concatenate all the rows in history
    String allRows;
    for (unsigned i = 0; i < chatHistory_.Size(); ++i)
        allRows += chatHistory_[i] + "\n";

    chatHistoryText_->SetText(allRows);
}

void Chat::UpdateButtons()
{
    auto* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();
    bool serverRunning = network->IsServerRunning();

    // Show and hide buttons so that eg. Connect and Disconnect are never shown at the same time
    sendButton_->SetVisible(serverConnection != nullptr);
    connectButton_->SetVisible(!serverConnection && !serverRunning);
    disconnectButton_->SetVisible(serverConnection || serverRunning);
    startServerButton_->SetVisible(!serverConnection && !serverRunning);
}

void Chat::HandleLogMessage(StringHash /*eventType*/, VariantMap& eventData)
{
    using namespace LogMessage;

    ShowChatText(eventData[P_MESSAGE].GetString());
}

void Chat::HandleSend(StringHash /*eventType*/, VariantMap& eventData)
{
    String text = textEdit_->GetText();
    if (text.Empty())
        return; // Do not send an empty message

    auto* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();

    if (serverConnection)
    {
        // A VectorBuffer object is convenient for constructing a message to send
        VectorBuffer msg;
        msg.WriteString(text);
        // Send the chat message as in-order and reliable
        serverConnection->SendMessage(MSG_CHAT, true, true, msg);
        // Empty the text edit after sending
        textEdit_->SetText(String::EMPTY);
    }
}

void Chat::HandleConnect(StringHash /*eventType*/, VariantMap& eventData)
{
    auto* network = GetSubsystem<Network>();
    String address = textEdit_->GetText().Trimmed();
    if (address.Empty())
        address = "localhost"; // Use localhost to connect if nothing else specified
    // Empty the text edit after reading the address to connect to
    textEdit_->SetText(String::EMPTY);

    // Connect to server, do not specify a client scene as we are not using scene replication, just messages.
    // At connect time we could also send identity parameters (such as username) in a VariantMap, but in this
    // case we skip it for simplicity
    network->Connect(address, CHAT_SERVER_PORT, nullptr);

    UpdateButtons();
}

void Chat::HandleDisconnect(StringHash /*eventType*/, VariantMap& eventData)
{
    auto* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();
    // If we were connected to server, disconnect
    if (serverConnection)
        serverConnection->Disconnect();
    // Or if we were running a server, stop it
    else if (network->IsServerRunning())
        network->StopServer();

    UpdateButtons();
}

void Chat::HandleStartServer(StringHash /*eventType*/, VariantMap& eventData)
{
    auto* network = GetSubsystem<Network>();
    network->StartServer(CHAT_SERVER_PORT);
	server::TrackInfo ti;
	server::RaceRoomManager::GetInstancePtr()->CreateRoom(ti);
    UpdateButtons();
	players_.reserve(kTrackCount);
}

int GetRoleId()
{
	static constexpr int max_role_id = 5;
	static int current_id = 0;
	return current_id++ % max_role_id;
}
void Chat::HandleNetworkMessage(StringHash /*eventType*/, VariantMap& eventData)
{
	
    auto* network = GetSubsystem<Network>();

    using namespace NetworkMessage;

    int msgID = eventData[P_MESSAGEID].GetInt();
	
    if (msgID == MSG_CHAT)
    {
        const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
        // Use a MemoryBuffer to read the message data so that there is no unnecessary copying
        MemoryBuffer msg(data);
        //String text = msg.ReadString();

        // If we are the server, prepend the sender's IP address and port and echo to everyone
        // If we are a client, just display the message
//         if (network->IsServerRunning())
//         {
//             //auto* sender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
// 
//             //text = sender->ToString() + " " + text;
// 
//             //VectorBuffer sendMsg;
//             //sendMsg.WriteString(text);
//             // Broadcast as in-order and reliable
//             //network->BroadcastMessage(MSG_CHAT, true, true, sendMsg);
// 			network->BroadcastMessage(MSG_CHAT, true, true, msg.GetData(), msg.GetSize());
//         }
		
		auto pdata = (message::MessageHead*)msg.GetData();
		auto src_id = pdata->src;
		auto dst_id = pdata->dst;
		URHO3D_LOGINFOF(" receive a packet player[%d]->player[%d] %s", src_id, dst_id, message::id_to_str[static_cast<int>(pdata->id)]);
		switch (pdata->id)
		{
		case message::MessageId::kEnterRoom :
		{
			auto realmsg =(message::Enter*)pdata;
			auto room = server::RaceRoomManager::GetInstancePtr()->FindRoom(realmsg->room_id);
			
			auto track_id = room->GetFreeTack();
			if (track_id != -1) {
				players_.push_back(std::make_unique<server::Player>(current_player_id_++));
				auto new_player = players_.back().get();
				new_player->SetRoleId(GetRoleId());
				auto* connection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
				new_player->SetConnection(connection);
				room->AddPlayer(new_player);
				
				message::PlayerId pid;
				pid.head.src = pid.head.dst = new_player->GetId();
				pid.head.src_role_id = new_player->GetRoleId();
				for (int i = 0; i < kTrackCount; i++) {
					int player_id = -1;
					int role_id = -1;
					int track_id = -1;
					auto player = room->GetPlayer(i);
					if (player) {
						player_id = player->GetId();
						role_id = player->GetRoleId();
						track_id = player->GetTrackId();
					}
					pid.other_player_id[i] = player_id;
					pid.other_role_id[i] = role_id;
					pid.other_track_id[i] = track_id;
				}
				
				connection->SendMessage(MSG_CHAT, true, true, (const unsigned char*)&pid, sizeof(pid));
				URHO3D_LOGINFOF("Send kPlayerId : %d role[%d] track[%d]", pid.head.src, pid.head.src_role_id, new_player->GetTrackId());

				// notify other player
				message::Enter enter;
				enter.head.src = enter.head.dst = pid.head.src;
				enter.head.src_role_id = pid.head.src_role_id;
				memcpy(enter.head.src_nick_name, realmsg->head.src_nick_name, 16);
				enter.track_id = new_player->GetTrackId();
				network->BroadcastMessage(MSG_CHAT, true, true, (const unsigned char*)&enter, sizeof(enter));
				URHO3D_LOGINFOF("Broadcast kEnterRoom : %d role[%d] track[%d]", pid.head.src, pid.head.src_role_id, enter.track_id);
			}
		}
		break;
		case message::MessageId::kLeaveRoom:
		{
			message::Enter enter;
			auto* connection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
			server::Player* ptr = FindPlayer(connection);
			if (ptr != nullptr) {
				message::Leave leave;
				leave.head.src = leave.head.dst = ptr->GetId();
				network->BroadcastMessage(MSG_CHAT, true, true, (const unsigned char*)&leave, sizeof(leave));
				RemovePlayer(ptr);
				URHO3D_LOGINFOF("player[%d] leave room.", ptr->GetId());
			} else {
				URHO3D_LOGINFOF("can't find player, leave room.");
			}
		}
		break;
		case message::MessageId::kFastPlayer :
			break;
		case message::MessageId::kSlowPlayer :
			break;
		case message::MessageId::kBarrier :
			break;
		case message::MessageId::kFreeze :
			break;
		case message::MessageId::kBlink :
			break;
		case message::MessageId::kUpdateLocation :
			break;
		default:
			break;
		}
    }
	
}

void Chat::RemovePlayer(server::Player* player)
{
	auto room = server::RaceRoomManager::GetInstancePtr()->FindRoom(0);
	if (!room) {
		return;
	}
	server::Player* p = nullptr;
	int idx = 0;
	bool find = false;
	for (; idx < players_.size(); idx++) {
		if (players_[idx].get() == player) {
			find = true;
			break;
		}
	}
	if (find) {
		URHO3D_LOGINFOF("DelPlayer %d[%d]", player->GetId(), player->GetTrackId());
		room->DelPlayer(player);
		players_.erase(players_.begin() + idx);
	}
	
}

server::Player* Chat::FindPlayer(Connection* connection)
{
	for (auto& player : players_) {
		if (player->GetConnection() == connection) {
			return player.get();
		}
	}
	return nullptr;
}

void Chat::HandleConnectionStatus(StringHash /*eventType*/, VariantMap& eventData)
{
    UpdateButtons();
}

void Chat::HandleClientDisconnect(StringHash eventType, VariantMap& eventData)
{
	auto* network = GetSubsystem<Network>();
	using namespace NetworkMessage;
	auto* connection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	server::Player* ptr = FindPlayer(connection);
	if (ptr != nullptr) {
		URHO3D_LOGINFOF("player[%d] leave room.", ptr->GetId());

		message::Leave leave;
		leave.head.src = leave.head.dst = ptr->GetId();
		network->BroadcastMessage(MSG_CHAT, true, true, (const unsigned char*)&leave, sizeof(leave));
		RemovePlayer(ptr);
	}
}
