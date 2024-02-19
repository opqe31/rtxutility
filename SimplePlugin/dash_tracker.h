#pragma once

#include "../plugin_sdk/plugin_sdk.hpp"

class c_dash_tracker
{
public:
	c_dash_tracker( );

	void on_process_spell_cast( game_object_script sender, spell_instance_script spell, std::map<std::uint32_t, TreeEntry*> color_map );
	void on_draw( bool draw_allies, bool draw_enemies, float max_duration, unsigned long clr_arc, float sz );
	void on_tick( float time );
private:
	struct s_dash 
	{
		vector v_start;
		vector v_end;
		float t_start;

		std::string display_name;
		bool is_enemy;

		uint32_t* icon_spell;
		uint32_t* icon_sender;
		float bounding_radius;
		unsigned long color;

		bool removal = false;
		float t_end_anim = -1.f;
		float t_last_anim_0 = 0.f;
		float t_last_anim_1 = 0.f;
		float t_last_anim_2 = 0.f;

		float t_last_anim_3_1 = -1.f;
		float t_last_anim_3 = -1.f;

		float t_end_anim_alpha;
		float f_current_alpha;
		bool size_set = false;
		vector current_size = vector::zero;
	};
	std::vector<s_dash> m_dashes;
	std::map<std::uint32_t, float> m_clamp_spells;
	std::map<spellslot, std::string> m_spell_slots;

	void clamp( vector& from, vector& to, std::uint32_t hash );
	std::string get_display_slot( spellslot spell_slot );
	unsigned long get_color( std::map<std::uint32_t, TreeEntry*> color_map, std::uint32_t hash );
	float animate_height( float start_t, float end_t, float dest_pos, float current_pos, float ct, s_dash& dash );
};

extern c_dash_tracker* g_dash_tracker;