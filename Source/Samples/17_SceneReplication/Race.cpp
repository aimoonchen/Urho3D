#include "Race.h"
namespace race
{
	ModelRes g_model_res[kMaxRoleId] = {
		{"Models/Mutant/Mutant.mdl", "Models/Mutant/Materials/mutant_M.xml"},
		{"Models/Wow/Bloodelf/Female/Purple_Old.mdl", "Models/Wow/Bloodelf/Female/Materials/JoinedMaterial_#15.xml"},
		{"Models/Wow/Human/Female/Red_Old.mdl", "Models/Wow/Human/Female/Materials/JoinedMaterial_#13.xml"},
		{"Models/Wow/Orc/Male/Green_Old.mdl", "Models/Wow/Orc/Male/Materials/JoinedMaterial_#11.xml"},
		{"Models/Wow/Tauren/Male/Yellow_Old.mdl", "Models/Wow/Tauren/Male/Materials/JoinedMaterial_#12.xml"},
		{"Models/Wow/Pandaren/Male/Black_Old.mdl", "Models/Wow/Pandaren/Male/Materials/characterpandarenmalepandarenmale_0.xml"}
	};

	std::string g_ani_state[kMaxRoleId][kMaxAniState] = {
		{{"Models/Mutant/Mutant_Idle0.ani"},{"Models/Mutant/Mutant_Walk.ani"},{"Models/Mutant/Mutant_Run.ani"},{"Models/Mutant/Mutant_Jump1.ani"}},
		{{"Models/Wow/Bloodelf/Female/Purple_Stand_Take 001.ani"},{"Models/Wow/Bloodelf/Female/Purple_Walk_Take 001.ani"},{"Models/Wow/Bloodelf/Female/Purple_Run_Take 001.ani"},{"Models/Wow/Bloodelf/Female/Purple_Jump_Take 001.ani"}},
		{{"Models/Wow/Human/Female/Red_Stand_Take 001.ani"},{"Models/Wow/Human/Female/Red_Walk_Take 001.ani"},{"Models/Wow/Human/Female/Red_Run_Take 001.ani"},{"Models/Wow/Human/Female/Red_Jump_Take 001.ani"}},
		{{"Models/Wow/Orc/Male/Green_Stand_Take 001.ani"},{"Models/Wow/Orc/Male/Green_Walk_Take 001.ani"},{"Models/Wow/Orc/Male/Green_Run_Take 001.ani"},{"Models/Wow/Orc/Male/Green_Jump_Take 001.ani"}},
		{{"Models/Wow/Tauren/Male/Yellow_Stand_Take 001.ani"},{"Models/Wow/Tauren/Male/Yellow_Walk_Take 001.ani"},{"Models/Wow/Tauren/Male/Yellow_Run_Take 001.ani"},{"Models/Wow/Tauren/Male/Yellow_Jump_Take 001.ani"}},
		{{"Models/Wow/Pandaren/Male/Black_Stand_Take 001.ani"},{"Models/Wow/Pandaren/Male/Black_Walk_Take 001.ani"},{"Models/Wow/Pandaren/Male/Black_Run_Take 001"},{"Models/Wow/Pandaren/Male/Black_Jump_Take 001.ani"}}
	};
}