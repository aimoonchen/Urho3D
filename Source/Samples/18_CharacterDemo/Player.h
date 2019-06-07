#pragma once
#include <string>
#include <Urho3D/Container/Ptr.h>
class Character;

namespace Urho3D
{
	class Node;
	class Scene;
}

namespace race
{
	enum RoleId
	{
		kMutant = 0,
		kBloodelf,
		kHuman,
		kOrc,
		kPandaren,
		kMaxRoleId
	};
	enum AniState
	{
		kIdle = 0,
		kWalk,
		kRun,
		kJump,
		kMaxAniState
	};
	extern std::string g_ani_state[kMaxRoleId][kMaxAniState];
	class Track;
	class Room;
	class Player
	{
	public:
		Player(int uid, const std::string& nick_name)
			: uid_{ uid }, nick_name_{ nick_name }
		{}
		void SetScene(Urho3D::Scene* scene);
		void SetRoleId(int roleId);
		void SetRemoteTrackId(int trackId) { remote_track_id_ = trackId; }
		int GetRemoteTrackId() const { return remote_track_id_; }
		void LeaveScene();
		void SetRoom(Room* room) { room_ = room; }
		Room* GetRoom() const { return room_; }
		void SetTrack(Track* track) { track_ = track; }
		Track* GetTrack() const { return track_; }
		int GetId() const { return uid_; }
		const std::string& GetName() const { return nick_name_; }
		void Update(float elaspedTime);
		Urho3D::WeakPtr<Character> GetCharacter() const { return character_; }
	private:
		int			role_id_;
		int			uid_;
		std::string nick_name_;
		Room*		room_;
		Track*		track_{ nullptr };
		Urho3D::Scene* scene_{ nullptr };
		Urho3D::Node*	entity_{ nullptr };
		/// The controllable character component.
		Urho3D::WeakPtr<Character> character_;
		int			remote_track_id_{ -1 };
	};
}
