#pragma once
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
}