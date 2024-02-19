#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

namespace cfg
{
}

namespace spell_tracker
{
	void create_menu( );

	void load( );
	void unload( );

	void draw_map_portrait( game_object_script x, vector position, bool fow = false, float time_in_fow = -1.f );
	void track_aa_lethal( game_object_script x, int position,
		unsigned long color_background, unsigned long color_text, unsigned long color_text_aa,
		bool only_if_x_aa, int i_aa_count );
	void track_zones( bool draw_zones, bool draw, TreeEntry* priority_list, int font_size, TreeEntry* x, TreeEntry* y, bool lock, bool hidevis );

	namespace hero_ranges
	{
		void load_menu( TreeTab* base_menu );
		void draw_ranges( bool is_env_draw );
	}

	namespace structures
	{
		void load_menu( TreeTab* inhibitor, TreeTab* turret );
		void unload( );

		void show_turret( );
		void show_inhibitor( );

		void on_inhibitor_death( uint32_t sender );
	}
}