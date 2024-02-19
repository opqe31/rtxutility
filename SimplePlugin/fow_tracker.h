#pragma once

#include "../plugin_sdk/plugin_sdk.hpp"
#include <optional>

class c_fow_tracker
{
public:
	void load( );
	void unload( );

	enum e_enabled_mode
	{
		all = 0,
		native = 1,
		minimap = 2
	};

	struct s_fow_tracker_entry 
	{
		float t_last_update = -600.f;
		float t_last_update_particle = -600.f;
		vector position = vector::zero;
	};

	std::map<uint32_t, s_fow_tracker_entry> tracker_data = { };

	bool is_enabled( e_enabled_mode mode = e_enabled_mode::all );

	void update_position( uint32_t network_id, vector position, bool p = false );
	void notify_objective_attack( game_object_script sender, uint32_t* opt_icon = nullptr );
	std::optional<float> get_last_damage_taken( int camp_id );
	s_fow_tracker_entry get_data( uint32_t network_id );

	TreeEntry* cfg_ref;
	TreeEntry* local_ping;
	TreeEntry* global_ping;

	TreeEntry* monster_glow;
	TreeEntry* monster_glow_color;
	TreeEntry* monster_glow_blur;
	TreeEntry* monster_glow_thickness;

	TreeEntry* update_minimap;
	TreeEntry* draw_3d_jg;
	TreeEntry* draw_3d_color_resparning;

	TreeEntry* draw_3d_clr_atk;
	TreeEntry* draw_3d_clr_anim;

	TreeEntry* draw_circle_epic;
	TreeEntry* draw_circle_radius;
	TreeEntry* aggro_circle_color;

	TreeEntry* monster_tracker_fow;

	TreeEntry* max_draw_time;

	bool is_objective_under_attack( int obj );

	int get_camp_id_mutual( int id );
	uint32_t* get_monster_icon( int camp );
	void update_monster_icon( int camp, game_object_script x );
	void update_camp_status( game_object_script x, bool notify = false );
	void handle_player_death( game_object_script sender );
	float camp_respawn_t( int camp, bool kill = false );
	std::optional<uint32_t> camp_id_from_string( game_object_script sender );
	//void handle_plant( game_object_script sender, uint32_t animation_name );
private:
	std::map<int, uint32_t*> m_monster_icons;
};

extern c_fow_tracker* fow_tracker;