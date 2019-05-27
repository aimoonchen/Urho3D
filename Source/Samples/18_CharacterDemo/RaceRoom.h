#pragma once
#include <memory>
#include <vector>
#include <map>

namespace server
{
// 	struct Barrier
// 	{
// 		Barrier(Scene* scene, const Vector3& pos, float size, float duration);
// 		~Barrier();
// 		void Destory();
// 		void Update(float elapsedTime);
// 
// 		Vector3 pos_;
// 		float size_{ 1.0f };
// 		float duration_{ 0.0f };
// 		float born_time_{ 0.0f };
// 		bool active_{ false };
// 	};
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
		void SetPlayer(Player* player) { player_ = player; }
		Player* GetPlayer() const { return player_; }
	private:
		Player* player_;
	};
	
	class RaceRoom
	{
	public:
		RaceRoom(const TrackInfo& ti, int id = 0, std::string name = "");
		~RaceRoom();
		void AddPlayer(Player* player, int racetrackId);
		void DelPlayer(Player* player);
		void Init(int play_count);
		void Clean();
		void Update(float elapsedTime);
	protected:
	private:
		int										id_;
		std::string								name_;
		int										play_count_;
		std::vector<Player*>					players_;
		TrackInfo								track_info_;
		std::vector<std::unique_ptr<Racetrack>> racetracks_;
	};

	class RaceRoomManager
	{
	public:
		static RaceRoomManager* GetInstancePtr();
		void Clean();
		RaceRoom* CreateRoom();
		RaceRoom* FindRoom(int room_id);
	private:
		RaceRoomManager();
		~RaceRoomManager();
		std::map<int, std::unique_ptr<RaceRoom>> race_room_map_;
	};
}
