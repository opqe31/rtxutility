#pragma once

#include "../plugin_sdk/plugin_sdk.hpp"

class c_fake_cursor
{
public:
	void load(TreeEntry* red_enabled, TreeEntry* red_extra_range, TreeEntry* green_enabled, TreeEntry* green_delay_max, TreeEntry* green_delay_min);

	void on_draw();
	void on_process_spell_cast(game_object_script sender, spell_instance_script spell);
private:
	TreeEntry* _red_enabled; TreeEntry* _red_extra_range;  TreeEntry* _green_enabled;  TreeEntry* _green_delay_max;  TreeEntry* _green_delay_min;

	/*loaded_texture* target_enemy_precise;
	loaded_texture* target_ally_precise;
	loaded_texture* circularrangeindicator_dark;*/

	struct s_render_task
	{
		bool is_aa = false;
		bool is_enemy = false;

		float start_time = 0.f;
		float end_time = 0.f;

		vector pos = vector::zero;
	};

	s_render_task current_render{  };
	s_render_task next_render{  };

	bool initialized = false;

	//vector randomize(const vector& current_position, float min, float max);
};

extern c_fake_cursor* g_cursor;