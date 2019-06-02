#pragma once
#include <string>
#include <Urho3D/Container/Ptr.h>
class Character;

namespace Urho3D
{
	class Scene;
}

namespace race
{
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
		void SetRoom(Room* room) { race_room_ = room; }
		Room* GetRoom() const { return race_room_; }
		void SetTrack(Track* track) { track_ = track; }
		Track* GetTrack() const { return track_; }
		int GetId() const { return uid_; }
		const std::string& GetName() const { return nick_name_; }
		void Update(float elaspedTime);
	private:
		int			role_id_;
		int			uid_;
		std::string nick_name_;
		Room*		race_room_;
		Track*		track_{ nullptr };
		Urho3D::Scene* scene_{ nullptr };
		/// The controllable character component.
		Urho3D::WeakPtr<Character> character_;
	};
}
