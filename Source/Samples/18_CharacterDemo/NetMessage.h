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
	MessageHead head;
};

struct UpdateLocation
{
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
	MessageHead head;
	int room_id;
};

struct Leave
{
	MessageHead head;
	int room_id;
};

struct Fast
{
	MessageHead head;
	float ratio;
};

struct Slow
{
	MessageHead head;
	float ratio;
};

struct Barrier
{
	MessageHead head;
	float x_pos;
	float z_pos;
	float width;
	float duration;
};

struct Freeze
{
	MessageHead head;
	float duration;
};

struct Blink
{
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
