#pragma once
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "Urho3D/Math/Vector3.h"

namespace Urho3D
{
	class Connection;
	class Node;
}
namespace race
{
	class Avatar;
}
namespace server
{
	struct Barrier
	{
		Barrier(const Urho3D::Vector3& pos, float width, float duration);
		~Barrier();
		void Destory();
		void Update(float elapsedTime);

		Urho3D::Vector3 pos_;
		float width_{ 1.0f };
		float duration_{ 0.0f };
		float born_time_{ 0.0f };
		bool active_{ false };
	};
	class Track;
	class Room;
	class Player
	{
	public:
		Player(int uid) : uid_{ uid } {}
		void EnterRoom() {}
		void LeaveRoon() {}
		int GetId() const { return uid_; }
		int GetRoleId() const;
// 		void SetRoleId(int roleId) { role_id_ = roleId; }
		void SetAvatar(std::unique_ptr<race::Avatar> avatar);
		race::Avatar* GetAvatar() const { return avatar_.get(); }
		unsigned int GetUrhoID() const;
		int GetTrackId() const;
		void SetTrack(Track* track) { track_ = track; }
		Track* GetTrack() const { return track_; }
		void SetConnection(Urho3D::Connection* con) { connection_ = con; }
		Urho3D::Connection* GetConnection() const { return connection_; }
	private:
		//
		int			uid_{ -1 };
		std::string name_;
		Room*		race_room_{ nullptr };
		Track*		track_{ nullptr };
		std::unique_ptr<race::Avatar> avatar_;
		Urho3D::Connection* connection_{ nullptr };
	};
	
	struct TrackInfo
	{
		float	width_{ 10.0f };
		float	length_{ 50.0f };
		int		grid_count_{ 3 };
		float	cell_dimension_{ 0.0f };
	};

	class Track
	{
	public:
		Track(Room* raceRoom, const Urho3D::Vector3& bornPos, int id);
		~Track();
		void SetPlayer(Player* player) { player_ = player; }
		Player* GetPlayer() const { return player_; }
		int GetId() const { return id_; }
		bool IsFree() const { return player_ == nullptr; }
		const Urho3D::Vector3& GetPos() const { return born_pos_; }
	private:
		struct Cell
		{
			void CreateBarrier(const Urho3D::Vector3& pos, float width, float duration);
			void DestoryBarrier();
			void Update(float elapsedTime);
			std::unique_ptr<Barrier> barrier_{ nullptr };
		};

		int				id_;
		Room*			race_room_{ nullptr };
		Player*			player_{ nullptr };
		Urho3D::Vector3 born_pos_;
	};
	
	class Room
	{
	public:
		Room(const TrackInfo& ti, int id = 0, std::string name = "");
		~Room();
		bool AddPlayer(Player* player);
		void DelPlayer(Player* player);
		void AddObserver(Player* observer);
		void DelObserver(Player* observer);
		void Init(int playerCount);
		void Clean();
		void Update(float elapsedTime);
		int GetFreeTack();
		int GetPlayerId(int trackId) const;
		Player* GetPlayer(int trackId) const;
		const TrackInfo& GetTrackInfo() const { return track_info_; }
	protected:
	private:
		int										id_;
		std::string								name_;
		int										player_count_;
		std::vector<Player*>					observers_;
		TrackInfo								track_info_;
		std::vector<std::unique_ptr<Track>>		racetracks_;
	};

	class RaceRoomManager
	{
	public:
		static RaceRoomManager* GetInstancePtr();
		void Clean();
		Room* CreateRoom(const TrackInfo& ti, int playerCount = 5);
		Room* FindRoom(int room_id);
	private:
		RaceRoomManager();
		~RaceRoomManager();
		std::map<int, std::unique_ptr<Room>> race_room_map_;
	};
}
