#include "../plugin_sdk/plugin_sdk.hpp"
#pragma once

namespace summoner_spell
{
	void cleanse_menu();
	void summoner_spell_menu();

	void flash_gapcloser(script_spell* spell, game_object_script sender);
	void cast_snow_ball(script_spell* spell, game_object_script target);

	void all_menu();
	void all_logic();
	void all_draw();
	void all_new_draw();
	void all_unload();
};
