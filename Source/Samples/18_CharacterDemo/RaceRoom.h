#pragma once
#include <memory>
#include <vector>
#include <map>
#include "Urho3D/Math/Vector3.h"

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
	class RaceRoom;
	class Player
	{
	public:
		void EnterRoom() {}
		void LeaveRoon() {}
	private:
		std::string name_;
		RaceRoom* race_room_;
	};
	
	struct TrackInfo
	{
		float	width_{ 10.0f };
		float	length_{ 50.0f };
		int		grid_count_{ 3 };
		float	cell_dimension_{ 0.0f };
	};

	class Racetrack
	{
	public:
		Racetrack(RaceRoom* raceRoom, const Urho3D::Vector3& bornPos, int id);
		~Racetrack();
		void SetPlayer(Player* player) { player_ = player; }
		Player* GetPlayer() const { return player_; }
		int GetId() const { return id_; }
		bool IsFree() const { return player_ == nullptr; }
	private:
		struct Cell
		{
			void CreateBarrier(const Urho3D::Vector3& pos, float width, float duration);
			void DestoryBarrier();
			void Update(float elapsedTime);
			std::unique_ptr<Barrier> barrier_{ nullptr };
		};

		int				id_;
		RaceRoom*		race_room_{ nullptr };
		Player*			player_{ nullptr };
		Urho3D::Vector3 born_pos_;
	};
	
	class RaceRoom
	{
	public:
		RaceRoom(const TrackInfo& ti, int id = 0, std::string name = "");
		~RaceRoom();
		bool AddPlayer(Player* player);
		void DelPlayer(Player* player);
		void AddObserver(Player* observer);
		void DelObserver(Player* observer);
		void Init(int playerCount);
		void Clean();
		void Update(float elapsedTime);
	protected:
	private:
		int GetFreeTack();
		int										id_;
		std::string								name_;
		int										player_count_;
		std::vector<Player*>					observers_;
		TrackInfo								track_info_;
		std::vector<std::unique_ptr<Racetrack>> racetracks_;
	};

	class RaceRoomManager
	{
	public:
		static RaceRoomManager* GetInstancePtr();
		void Clean();
		RaceRoom* CreateRoom(const TrackInfo& ti);
		RaceRoom* FindRoom(int room_id);
	private:
		RaceRoomManager();
		~RaceRoomManager();
		std::map<int, std::unique_ptr<RaceRoom>> race_room_map_;
	};
}
