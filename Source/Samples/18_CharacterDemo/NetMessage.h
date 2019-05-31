#pragma once
// Identifier for the chat network messages
constexpr int MSG_CHAT = 153;

namespace message
{
enum class MessageId : int
{
	kEnterRoom = 0,
	kLeaveRoom,
	kFastPlayer,
	kSlowPlayer,
	kBarrier,
	kFreeze,
	kBlink,
	kUpdateLocation,
	kPlayerId,
	kCommandCount
};

#pragma pack(push, 1)

struct MessageHead
{
	MessageId		id;
	int				src;
	int				dst;
};

struct PlayerId
{
	PlayerId() { head.id = MessageId::kPlayerId; }
	MessageHead head;
};

struct UpdateLocation
{
	UpdateLocation() { head.id = MessageId::kUpdateLocation; }
	MessageHead head;
	float px;
	float py;
	float pz;
	float rx;
	float ry;
	float rz;
	float rw;
};

struct Enter
{
	Enter() { head.id = MessageId::kEnterRoom; }
	MessageHead head;
	int room_id;
};

struct Leave
{
	Leave() { head.id = MessageId::kLeaveRoom; }
	MessageHead head;
	int room_id;
};

struct Fast
{
	Fast() { head.id = MessageId::kFastPlayer; }
	MessageHead head;
	float ratio;
};

struct Slow
{
	Slow() { head.id = MessageId::kSlowPlayer; }
	MessageHead head;
	float ratio;
};

struct Barrier
{
	Barrier() { head.id = MessageId::kBarrier; }
	MessageHead head;
	float x_pos;
	float z_pos;
	float width;
	float duration;
};

struct Freeze
{
	Freeze() { head.id = MessageId::kFreeze; }
	MessageHead head;
	float duration;
};

struct Blink
{
	Blink() { head.id = MessageId::kBlink; }
	MessageHead head;
	float distance;
};

// struct Command
// {
// 	CommandHead		head;
// 	int				size;
// 	void*			data;
// };

#pragma pack(pop)

}
