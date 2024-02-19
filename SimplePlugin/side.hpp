#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

class c_side_hud 
{
public:
	void load( );
	void unload( );

	void on_draw( );
private:
	void create_menu( );
	TreeEntry* enabled;
	TreeEntry* sidehud_side;
	TreeEntry* hud_size;

	TreeEntry* ping_key;
	TreeEntry* spell_ready_ping_type;
	TreeEntry* spell_cooldown_type;
	TreeEntry* spellready_message_text;
	TreeEntry* spellcooldown_message_text;

	TreeEntry* button_champion_1; TreeEntry* button_spell_1;
	TreeEntry* button_champion_2; TreeEntry* button_spell_2; TreeEntry* button_seconds_2; TreeEntry* button_s_2; TreeEntry* button_m_2;

	TreeEntry* spellname_r;
	std::map<uint32_t, TreeEntry*> spellname_other;

	TreeEntry* show_elements;

	std::map<champion_id, TreeEntry*> cfg_champion_names;
	TreeEntry* pos_x; TreeEntry* pos_y;

	struct s_champion_data
	{
		float fl_last_visible = 0.f;
		float fl_last_health = 0.f;
		float fl_last_mana = 0.f;

		float fl_animvalue_dead = 0.001f;
		float t_last_animvalue_dead = -20.f;

		float fl_animvalue_vis = 0.001f;
		float t_last_animvalue_vis = -20.f;

		float fl_animvalue_spell [ 3 ] = { 0.001f, 0.001f, 0.001f };
		float t_last_animvalue_spell [ 3 ] = { -20.f, -20.f, -20.f };
	};

	const std::map<champion_id, const char*> m_champion_map = { };
	std::string get_champion_name( game_object_script x );

	float fl_last_ping = -20.f;
};

extern c_side_hud* g_side_hud;