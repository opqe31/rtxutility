#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

namespace spell_database
{
	enum e_force_detection_type
	{
		on_proc = 0,
		on_tick = 1
	};

	struct spell_data
	{
		const char* menuslot = "?";
		const std::vector<const char*> spell_names;
		const spellslot slot;

		const bool targeted;
		const bool missile;
		const bool off_by_default;

		const std::vector<buff_type> cc_types = { };
		const std::vector<damage_type> damage_types = { };
		const skillshot_type spellType;
		const float spellDelay;
		const float spellSpeed;

		const uint32_t buff_me = 0;
		const bool track_only_buff = false;
		const std::vector<uint32_t> missile_hashes = { };

		bool force_add = false;
		float force_radius = 0.f;
		e_force_detection_type force_detection_type = e_force_detection_type::on_proc;
		uint32_t buff_enemy;
		uint32_t buff_me_stacks = 1;
	};

	struct c_activate_spell_data
	{
		const char* menuslot = "?";
		const std::vector<const char*> spell_names;
		const spellslot slot;
		const bool menu_disabled;

		const float delay;
		const uint32_t buff;
		const int buff_stacks;
		const bool buff_holder_enemy = false;
		const float range = FLT_MAX;
		const std::vector<buff_type> cc_types = { };
	};

	std::map<champion_id, std::vector<spell_data>> load( );
	std::map< champion_id, std::vector<c_activate_spell_data>> load_activate( );
};