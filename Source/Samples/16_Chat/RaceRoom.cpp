#include "RaceRoom.h"
namespace server
{
	Barrier::Barrier(const Urho3D::Vector3& pos, float width, float duration)
		: pos_{ pos }, width_{ width }, duration_{ duration }
	{
		born_time_ = 0.0f;// GetElapsedTime();
		active_ = true;
	}

	Barrier::~Barrier()
	{
		Destory();
	}

	void Barrier::Destory()
	{
		active_ = false;
	}

	void Barrier::Update(float elapsedTime)
	{
		auto current_time = elapsedTime - born_time_;

		if (current_time > duration_) {
			Destory();
		} else {
			float fade_factor = current_time / duration_;
// 			if (mtl_) {
// 				mtl_->SetShaderParameter("MatDiffColor", Color{ 1.0f, 1.0f, 1.0f, 0.6f * (1.0f - fade_factor) });
// 			}
		}
	}

	Racetrack::Racetrack(RaceRoom* raceRoom, const Urho3D::Vector3& bornPos, int id)
		: race_room_{ raceRoom }, born_pos_{ bornPos }, id_{ id }
	{

	}
	
	Racetrack::~Racetrack()
	{

	}

	RaceRoom::RaceRoom(const TrackInfo& ti, int id, std::string name)
		: track_info_{ ti }, id_{ id }, name_{ std::move(name) }
	{

	}
	
	RaceRoom::~RaceRoom()
	{
		Clean();
	}
	
	void RaceRoom::Init(int playerCount)
	{
		// player count must be odd number
		player_count_ = playerCount;
		auto start_x = -(track_info_.width_ * playerCount) * 0.5f;
		for (int i = 0; i < playerCount; i++) {
			racetracks_.push_back(std::make_unique<Racetrack>(this, Urho3D::Vector3{ start_x + i * track_info_.width_, 0, 0 }, i));
		}
	}
	int RaceRoom::GetFreeTack()
	{
		for (auto& track : racetracks_) {
			if (track->IsFree()) {
				return track->GetId();
			}
		}
		return -1;
	}
	bool RaceRoom::AddPlayer(Player* player)
	{
		auto tid = GetFreeTack();
		if (tid < 0) {
			return false;
		}
		racetracks_[tid]->SetPlayer(player);
		return true;
	}

	void RaceRoom::DelPlayer(Player* player)
	{
		for (auto& track : racetracks_) {
			if (track->GetPlayer() == player) {
				track->SetPlayer(nullptr);
				break;
			}
		}
	}
	
	void RaceRoom::AddObserver(Player* observer)
	{

	}

	void RaceRoom::DelObserver(Player* observer)
	{

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

	RaceRoom* RaceRoomManager::CreateRoom(const TrackInfo& ti)
	{
		static int s_room_id = { 0 };
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