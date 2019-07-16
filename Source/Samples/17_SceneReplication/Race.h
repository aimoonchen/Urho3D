#pragma once

#include <string>

namespace race
{
	enum RoleId
	{
		kMutant = 0,
		kBloodelf,
		kHuman,
		kOrc,
		kTauren,
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

	struct ModelRes
	{
		std::string model;
		std::string mtl;
	};

	extern std::string g_ani_state[kMaxRoleId][kMaxAniState];
	extern ModelRes g_model_res[kMaxRoleId];
}