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
		players_.resize(playerCount);
	}
	int Room::GetFreeTack()
	{
		for (auto& track : racetracks_) {
			if (track->IsFree()) {
				return track->GetTrackId();
			}
		}
		return -1;
	}

	Player* Room::AddPlayer(int player_id, std::string name)
	{
		if (!players_[player_id]) {
			return nullptr;
		}
		players_[player_id] = std::make_unique<Player>(player_id, name);
		auto new_player = players_[player_id].get();
		new_player->SetRoom(this);
		return new_player;
	}

	void Room::DelPlayer(int player_id)
	{
		for (auto& track : racetracks_) {
			if (track->GetPlayerId() == player_id) {
				auto player = track->GetPlayer();
				player->SetRoom(nullptr);
				player->SetTrack(nullptr);
				track->SetPlayer(nullptr);
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
}