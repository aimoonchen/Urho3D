#include "Race.h"
#include "Player.h"
namespace race
{
	Track::Track(Room* raceRoom, const Urho3D::Vector3& bornPos, int id)
		: race_room_{ raceRoom }, born_pos_{ bornPos }, id_{ id }
	{

	}

	Track::~Track()
	{

	}

	int Track::GetPlayerId() const
	{
		if (!player_) {
			return -1;
		}
		return player_->GetId();
	}

	Room::Room(const TrackInfo& ti, int id, std::string name)
		: track_info_{ ti }, id_{ id }, name_{ std::move(name) }
	{

	}

	Room::~Room()
	{
		Clean();
	}

	void Room::Init(int playerCount)
	{
		// player count must be odd number
		player_count_ = playerCount;
		auto start_x = -(track_info_.width_ * playerCount) * 0.5f;
		for (int i = 0; i < playerCount; i++) {
			racetracks_.push_back(std::make_unique<Track>(this, Urho3D::Vector3{ start_x + i * track_info_.width_, 0, 0 }, i));
		}
	}
	int Room::GetFreeTack()
	{
		for (auto& track : racetracks_) {
			if (track->IsFree()) {
				return track->GetId();
			}
		}
		return -1;
	}

	Player* Room::AddPlayer(int player_id, std::string name)
	{
		Player* new_player = FindPlayer(player_id);
		if (new_player) {
			return new_player;
		}
		players_.push_back(std::make_unique<Player>(player_id, name));
		new_player = players_.back().get();
		new_player->SetRoom(this);
		return new_player;
	}
	
	Player* Room::FindPlayer(int playerId)
	{
		for (auto& player : players_) {
			if (player->GetId() == playerId) {
				return player.get();
			}
		}
		return nullptr;
	}

	void Room::DelPlayer(int playerId)
	{
		for (auto& track : racetracks_) {
			if (track->GetPlayerId() == playerId) {
				auto player = track->GetPlayer();
				player->SetRoom(nullptr);
				player->SetTrack(nullptr);
				track->SetPlayer(nullptr);
				for (auto itor = players_.begin(); itor != players_.end(); itor++) {
					if ((*itor)->GetId() == playerId) {
						players_.erase(itor);
						break;
					}
				}
				break;
			}
		}
	}

	void Room::AddObserver(Player* observer)
	{

	}

	void Room::DelObserver(Player* observer)
	{

	}

	void Room::Clean()
	{

	}

	void Room::Update(float elapsedTime)
	{

	}

	Track* Room::GetTrack(int trackId) const
	{
		return racetracks_[trackId].get();
	}

	Track* Room::GetTrackByPosition(const Urho3D::Vector3& pos) const
	{
		auto end_x = (track_info_.width_ * player_count_) * 0.5f;
		auto start_x = -end_x;
		if (pos.x_ < start_x || pos.x_ > end_x
			|| pos.y_ > track_info_.length_ || pos.y_ < 0.0f) {
			return nullptr;
		}
		int trackIdx = (pos.x_ - start_x) / track_info_.width_;
		return racetracks_[trackIdx].get();
	}
}