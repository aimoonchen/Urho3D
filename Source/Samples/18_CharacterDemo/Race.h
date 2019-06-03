#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#include "Urho3D/Math/Vector3.h"
namespace race
{
	class Room;
	class Player;
	class Track
	{
	public:
		Track(Room* raceRoom, const Urho3D::Vector3& bornPos, int id);
		~Track();
		void SetPlayer(Player* player) { player_ = player; }
		Player* GetPlayer() const { return player_; }
		int GetPlayerId() const;
		int GetTrackId() const { return id_; }
		bool IsFree() const { return player_ == nullptr; }
	private:
		struct Cell
		{
			void CreateBarrier(const Urho3D::Vector3& pos, float width, float duration);
			void DestoryBarrier();
			void Update(float elapsedTime);
			//std::unique_ptr<Barrier> barrier_{ nullptr };
		};

		int				id_;
		Room*			race_room_{ nullptr };
		Player*			player_{ nullptr };
		Urho3D::Vector3 born_pos_;
	};

	struct TrackInfo
	{
		float	width_{ 10.0f };
		float	length_{ 50.0f };
		int		grid_count_{ 3 };
		float	cell_dimension_{ 0.0f };
	};

	class Room
	{
	public:
		Room(const TrackInfo& ti, int id = 0, std::string name = "");
		~Room();
		Player* AddPlayer(int player_id, std::string name);
		void DelPlayer(int player_id);
		void AddObserver(Player* observer);
		void DelObserver(Player* observer);
		void Init(int playerCount);
		void Clean();
		void Update(float elapsedTime);
		Track* GetTrack(int trackId) const;
	protected:
	private:
		int GetFreeTack();
		int										id_;
		std::string								name_;
		int										player_count_;
		std::vector<std::unique_ptr<Player>>	players_;
		std::vector<std::unique_ptr<Player>>	observers_;
		TrackInfo								track_info_;
		std::vector<std::unique_ptr<Track>>		racetracks_;
	};

}