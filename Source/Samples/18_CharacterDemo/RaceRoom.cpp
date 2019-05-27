#include "RaceRoom.h"
namespace server
{
	RaceRoom::RaceRoom(const TrackInfo& ti, int id, std::string name)
		: track_info_{ ti }, id_{ id }, name_{ std::move(name) }
	{

	}
	
	RaceRoom::~RaceRoom()
	{
		Clean();
	}
	
	void RaceRoom::Init(int play_count)
	{
		play_count_ = play_count;
		players_.resize(play_count);
		for (int i = 0; i < play_count; i++) {
			racetracks_.push_back(std::make_unique<Racetrack>());
			players_[i] = nullptr;
		}
	}

	void RaceRoom::AddPlayer(Player* player, int racetrackId)
	{
		players_[racetrackId] = player;
	}

	void RaceRoom::DelPlayer(Player* player)
	{
		for (int i = 0; i < play_count_; i++) {
			if (players_[i] == player) {
				players_[i] = nullptr;
				break;
			}
		}
	}

	void RaceRoom::Clean()
	{

	}

	void RaceRoom::Update(float elapsedTime)
	{

	}
	RaceRoomManager* RaceRoomManager::GetInstancePtr()
	{
		static RaceRoomManager instance_;
		return &instance_;
	}
	
	void RaceRoomManager::Clean()
	{

	}

	RaceRoom* RaceRoomManager::CreateRoom()
	{
		static int s_room_id = { 0 };
		TrackInfo ti;
		auto race_room = std::make_unique<RaceRoom>(ti);
		auto race_room_ptr = race_room.get();
		race_room_map_.insert({ s_room_id,  std::move(race_room) });
		return race_room_ptr;
	}
	
	RaceRoom* RaceRoomManager::FindRoom(int room_id)
	{
		auto itor = race_room_map_.find(room_id);
		if (itor != race_room_map_.end()) {
			return itor->second.get();
		} else {
			return nullptr;
		}
	}

	RaceRoomManager::RaceRoomManager()
	{

	}
	RaceRoomManager::~RaceRoomManager()
	{

	}
}