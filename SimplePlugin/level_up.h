#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

namespace level_up
{
	void load();
	void unload();

	struct s_champion_entry
	{
		const char* role_name;
		int matches;

		std::vector<const char*> other;
		std::vector<const char*> first;
	};
	std::map<champion_id, std::vector<s_champion_entry>> load_data();
	

	//menu
	void load_preset(bool first_load);

	void create_menu();
}