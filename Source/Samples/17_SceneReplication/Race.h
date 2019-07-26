#pragma once

#include <string>

namespace race
{
	const unsigned CTRL_FORWARD = 1;
	const unsigned CTRL_BACK = 2;
	const unsigned CTRL_LEFT = 4;
	const unsigned CTRL_RIGHT = 8;
	const unsigned CTRL_JUMP = 16;

	const float MOVE_FORCE = 0.8f;
	const float INAIR_MOVE_FORCE = 0.02f;
	const float BRAKE_FORCE = 0.2f;
	const float JUMP_FORCE = 7.0f;
	const float YAW_SENSITIVITY = 0.1f;
	const float INAIR_THRESHOLD_TIME = 0.1f;

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