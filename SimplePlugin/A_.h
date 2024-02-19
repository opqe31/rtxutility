#include "../plugin_sdk/plugin_sdk.hpp"

#include <optional>

#undef M_PI
#undef PI

#pragma once
namespace A_
{
	// AIO utilities
	//
	void on_load();
	bool utils_show_chat();
	bool utils_use_chinese();
	std::vector<std::string> explode_string( std::string input, char delimiter );
	bool str_replace( std::string& str, const std::string& from, const std::string& to );
	bool obs_safe( );

	bool can_ward();
	void cast_ward(vector pos);
	game_object_script get_nearby_ward(vector pos, float range);

	void draw_arc(const vector& pos, float radius, int start_angle, int end_angle, unsigned long color, float thickness);
	// Buff checks
	//
	bool has_unkillable_buff(game_object_script target);
	bool has_untargetable_buff(game_object_script target);
	bool has_crowd_control_buff(game_object_script target);
	bool has_Invincible_buff(game_object_script target);
	bool has_skill_protect_buff(game_object_script target);
	bool checkbox(std::map<std::uint32_t, TreeEntry*>& map, game_object_script target);
	bool checkbox( std::map<champion_id, TreeEntry*>& map, game_object_script target );
	int get_slider( std::map<std::uint32_t, TreeEntry*>& map, game_object_script target );

	//monsters
	bool is_epic_monster( std::string name_lower );
	bool is_monster( std::string name );
	bool is_monster( game_object_script x );
	bool is_jungle_buff( std::string name_lower );
	bool is_smite_monster( std::string name );
	bool is_jungler( game_object_script x );
	vector get_nexus_pos( bool enemy );

	//Ð¡±ø
	int32_t count_minions_in_range(float range);
	int32_t count_monsters_in_range(float range);
	std::vector<vector> arc_points( float percent, const vector center, float radius, std::optional<int> num_points = std::nullopt, float start_angle = -90.f );
	void draw_gradient_arc( vector center_point, float radius, float start_angle, float end_angle, unsigned long color1, unsigned long color2,
		float thickness = 1.f, vector v_from = vector::zero );
	vector randomize( const vector& current_position, float min, float max );
	float random_float( float min, float max );

	unsigned long get_alpha( unsigned long color );
	unsigned long set_alpha(unsigned long color, unsigned long alpha);
	unsigned long set_alpha_percent( unsigned long color, float alpha );
	unsigned long animate_color(unsigned long color, float min_alpha = 60.f, float max_alpha = 180.f, float duration = 2.f, float period = 1.f);
	vector animate_vector( vector min_vec, vector max_vec, float l, float period = 1.f );
	void draw_corner_box( vector min, vector max, unsigned long color, bool animate = true, float length = -1.f, float thickness  = 2.f);
	unsigned long get_color( float damage_pcent );
	void d3d_to_menu( unsigned long long color, float* rgba );

	bool is_spell_immune( game_object_script x );
	bool can_be_killed( game_object_script x, bool check_spellshield );
	bool is_invulnerable( game_object_script target, float delay, bool jax );
	bool is_on_screen( vector v );
	geometry::polygon arc_poly( vector start, vector direction, float angle, float radius, int quality = 20 );
};

namespace A_trans
{
	void loadTranslation( );
};

enum easing_functions
{
	EaseInSine,
	EaseOutSine,
	EaseInOutSine,
	EaseInQuad,
	EaseOutQuad,
	EaseInOutQuad,
	EaseInCubic,
	EaseOutCubic,
	EaseInOutCubic,
	EaseInQuart,
	EaseOutQuart,
	EaseInOutQuart,
	EaseInQuint,
	EaseOutQuint,
	EaseInOutQuint,
	EaseInExpo,
	EaseOutExpo,
	EaseInOutExpo,
	EaseInCirc,
	EaseOutCirc,
	EaseInOutCirc,
	EaseInBack,
	EaseOutBack,
	EaseInOutBack,
	EaseInElastic,
	EaseOutElastic,
	EaseInOutElastic,
	EaseInBounce,
	EaseOutBounce,
	EaseInOutBounce
};

typedef float( *easingFunction )( float );

easingFunction getEasingFunction( easing_functions function );
