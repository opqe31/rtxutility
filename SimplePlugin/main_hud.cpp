#include "main_hud.hpp"
#include "fow_tracker.h"
#include "A_.h"
#include "utility.h"

#include <iomanip>
#include <sstream>
#include "dash_tracker.h"
#include "map_data.hpp"

#include "cooldown_manager.hpp"

#define ALPHA_SPELL_NOT_READY 220
#define COLOR_NOT_ENOUGH_MANA_RECT MAKE_COLOR(  9, 132, 227, 220 )
//#define COLOR_NOT_ENOUGH_MANA_RECT MAKE_COLOR(  116, 185, 255, 220 )
#define COLOR_NOT_ENOUGH_MANA_DROP MAKE_COLOR( 116, 185, 255, 255 )
#define ANIM_DETECTION_T 0.5f

#define PADDING_TEXT_X(SCALE) 14 * SCALE
#define PADDING_TEXT_Y(SCALE) 8 * SCALE

namespace spell_tracker
{
	float clr_active_color [ 4 ] = { 47, 49, 165, 255 };
	float clr_background_color [ 4 ] = { 47, 49, 145, 95 };
	float clr_active_text_color [ 4 ] = { 179, 55, 113, 255 };
	float clr_text_color [ 4 ] = { 245, 246, 250, 255 };
	float clr_inactive_text_color [ 4 ] = { 255, 255, 255, 255 };

	float dclr_active_color [ 4 ] = { 52.f / 255.f, 73.f / 255.f, 94.f / 255.f, 1.f };
	float dclr_background_color [ 4 ] = { 44.f / 255.f, 62.f / 255.f, 80.f / 255.f, 120.f / 255.f };
	float dclr_inactive_text_color [ 4 ] = { 1.f, 1.f, 1.f, 1.f };
	float dclr_text_color [ 4 ] = { 245.f / 255.f, 246.f / 255.f, 250.f / 255.f, 1.f };
	float dclr_active_text_color [ 4 ] = { 29.f / 255.f, 209.f / 255.f, 161.f / 255.f, 1.f };

	/*constexpr const uint32_t m_plants[ ] = {
		buff_hash( "SRU_Plant_Vision" ),
		buff_hash( "SRU_Plant_Satchel" ),
		buff_hash( "SRU_Plant_Health" )
	};*/

	constexpr const uint32_t m_clone_models[ ] = {
		buff_hash( "Neeko" ),
		buff_hash( "Shaco" ),
		buff_hash( "LeBlanc" ),
		buff_hash( "MonkeyKing" ),
		buff_hash( "fiddlestickseffigy" )
	};

	enum e_teleport_type
	{
		at_nexus = 0,
		at_particle_end = 2,
		zed = 3,
		shen = 4,
		pantheon = 5
	};

	struct s_teleport
	{
		float t_end = 0.f;
		float duration = 0.f;

		uint32_t source_network_id = 0;
		std::optional< uint32_t > owner_network_id = 0;
		e_teleport_type type;

		vector last_position;
		vector last_direction;

		uint32_t u_spell_hash;

		float fl_alpha = 0.001f;
		float t_last_alpha_change = -20.f;

		bool is_ally = true;

		auto can_delete( ) -> bool
		{
			return gametime->get_time( ) > t_end && fl_alpha <= 0.01f;
		};

		auto is_expired( ) -> bool
		{
			return gametime->get_time( ) > t_end;
		};

		auto get_time_till_end( ) -> float
		{
			return std::max( 0.f, t_end - gametime->get_time( ) );
		}

		auto get_icon( ) -> uint32_t*
		{
			auto spell = database->get_spell_by_hash( u_spell_hash );
			return spell && spell->get_icon_texture_by_index( 0 ) ? spell->get_icon_texture_by_index( 0 ) : nullptr;
		}

		auto get_owner_icon( ) -> uint32_t*
		{
			if ( !owner_network_id.has_value( ) )
				return nullptr;

			auto owner = entitylist->get_object_by_network_id( owner_network_id.value( ) );

			if ( !owner || !owner->is_valid( ) )
				return nullptr;

			return owner->get_square_icon_portrait( );
		}

		auto get_position( ) -> vector
		{
			if ( type == e_teleport_type::at_nexus || type == e_teleport_type::shen )
			{
				auto particle = entitylist->get_object_by_network_id( source_network_id );
				if ( !particle || !particle->is_valid( ) )
					return last_position;

				auto owner = particle->get_emitter( );
				if ( !owner || !owner->is_valid( ) )
					return last_position;

				auto position = vector::zero;
				auto target = particle->get_particle_target_attachment_object( ) && particle->get_particle_target_attachment_object( )->is_valid( ) ? particle->get_particle_target_attachment_object( ) : particle->get_particle_attachment_object( );
				if ( !target || !target->is_valid( ) )
					return last_position;

				if ( type == e_teleport_type::at_nexus )
				{
					if ( particle->get_position( ).distance( target->get_position( ) ) <= 0 )
						position = target->get_position( ).extend( A_::get_nexus_pos( !particle->is_enemy( ) ), target->get_bounding_radius( ) + owner->get_bounding_radius( ) );
					else
						position = particle->get_position( ).extend( A_::get_nexus_pos( !particle->is_enemy( ) ), owner->get_bounding_radius( ) );
				}
				else if ( type == e_teleport_type::shen )
					position = target->get_position( ).extend( A_::get_nexus_pos( !particle->is_enemy( ) ), ( target->get_bounding_radius( ) + owner->get_bounding_radius( ) ) );

				if ( position.is_wall( ) || position.is_building( ) )
					position = particle->get_position( );

				last_position = position;
				return last_position;
			}
			else if ( type == e_teleport_type::at_particle_end )
			{
				auto particle = entitylist->get_object_by_network_id( source_network_id );

				if ( !particle || !particle->is_valid( ) )
					return last_position;

				last_position = particle->get_position( );
				return last_position;
			}
			else if ( type == e_teleport_type::zed )
			{
				auto particle = entitylist->get_object_by_network_id( source_network_id );
				if ( !particle || !particle->is_valid( ) )
					return last_position;

				auto owner = particle->get_emitter( );
				if ( !owner || !owner->is_valid( ) )
					return last_position;

				auto position = vector::zero;
				auto target = particle->get_particle_target_attachment_object( ) && particle->get_particle_target_attachment_object( )->is_valid( ) ? particle->get_particle_target_attachment_object( ) : particle->get_particle_attachment_object( );
				if ( !target || !target->is_valid( ) )
					return last_position;

				if ( !last_direction.is_valid( ) )
					last_direction = owner->get_direction( );

				position = target->get_position( ) + ( last_direction * ( target->get_bounding_radius( ) + owner->get_bounding_radius( ) ) );
				if ( position.is_wall( ) || position.is_building( ) )
					position = target->get_position( ) + ( last_direction * 1 );

				last_position = position;
				return last_position;
			}
			else if ( type == e_teleport_type::pantheon )
			{
				return last_position;
			}

			return vector::zero;
		}

		s_teleport( float flduration, uint32_t usource_network_id, e_teleport_type utype, std::optional<uint32_t> owner_nid, uint32_t spell_hash )
		{
			duration = flduration;
			t_end = gametime->get_time( ) + duration;

			source_network_id = usource_network_id;
			type = utype;
			u_spell_hash = spell_hash;

			owner_network_id = owner_nid;
			last_direction = last_position = vector::zero;

			auto particle = entitylist->get_object_by_network_id( source_network_id );
			if ( particle && particle->is_valid( ) )
			{
				auto emitter = particle->get_emitter( );
				if ( emitter && emitter->is_valid( ) )
					is_ally = emitter->is_ally( );
			}
		}
	};

	struct s_animdata {
		float fl_current_alpha = 0.001f;
		float t_last_animation = -20.f;
	};

	struct s_animdata_hud 
	{
		float fl_current_alpha = 0.001f;
		float t_last_animation = -20.f;

		//std::optional<vector4> v4_bar_pos = std::nullopt;
		std::optional<vector4> v4_hp_bar_pos = std::nullopt;
	};

	struct s_revive
	{
		vector position;
		float duration;
		float time_end;

		bool enemy = false;
		std::optional<uint32_t> owner_nid = std::nullopt;
		std::optional<ItemId> itemid = std::nullopt;
		std::optional<uint32_t> spellhash = std::nullopt;

		float fl_alpha = 0.001f;
		float t_last_alpha_change = -20.f;

		auto get_icon( ) -> std::optional< std::pair<uint32_t*, vector4> >
		{
			if ( itemid.has_value( ) )
			{
				auto item = database->get_item_by_id( itemid.value( ) );
				if ( item )
					return item->get_texture( 0 );
			}
			else if ( spellhash.has_value( ) )
			{
				auto spell = database->get_spell_by_hash( spellhash.value( ) );
				if ( spell && spell->get_icon_texture_by_index( 0 ) )
					return std::make_pair< uint32_t*, vector4>( spell->get_icon_texture_by_index( 0 ), { 0, 0, 1, 1 } );
			}

			return std::nullopt;
		}

		auto get_owner_icon( ) -> uint32_t*
		{
			if ( !owner_nid.has_value( ) )
				return nullptr;

			auto owner = entitylist->get_object_by_network_id( owner_nid.value( ) );
			return owner && owner->is_valid( ) ? owner->get_square_icon_portrait( ) : nullptr;
		}

		auto get_time_till_end( ) -> float
		{
			return std::max( 0.f, time_end - gametime->get_time( ) );
		}

		auto can_delete( ) -> bool
		{
			return gametime->get_time( ) > time_end && fl_alpha <= 0.01f;
		};

		auto is_expired( ) -> bool
		{
			return gametime->get_time( ) > time_end;
		};

		s_revive( vector vposition, float flduration, bool is_enemy, std::optional<uint32_t> opt_owner_id, std::optional<uint32_t> opt_spell_hash = std::nullopt, std::optional<ItemId> opt_item_id = std::nullopt )
		{
			position = vposition;
			duration = flduration;
			time_end = gametime->get_time( ) + duration;

			enemy = is_enemy;

			owner_nid = opt_owner_id;
			itemid = opt_item_id;
			spellhash = opt_spell_hash;
		}
	};

	struct s_average
	{
		float t_last_entry;
		std::vector<float> entries;
	};

	struct s_action_entry
	{
		std::vector<float> action_data = { };
		float t_last_invalid_cast = -1.f;

		float t_last_emote = -1.f;
		float t_last_cast = -1.f;

		float t_last_emote_cancel = -1.f;
	};

	struct s_draw_monster_entry
	{
		std::vector<std::pair<uint32_t, std::string>> text; //color; text
		std::vector<vector> sizes;
		vector line_len;

		uint32_t* texture = nullptr;
	};

	std::map<champion_id, s_action_entry> action_map;
	std::map<champion_id, float> peak_action_map;
	std::map<champion_id, s_average> action_map_avg;

	std::map<uint32_t, bool> m_glow_champions = { };
	std::vector<s_teleport> m_teleports = { };
	std::vector<s_revive> m_revives = { };

	namespace cfg
	{
		namespace lastseen
		{
			bool* b_lastseen_active = new bool( false );
			TreeEntry* enabled;
			TreeEntry* hide_visible;
			TreeEntry* zones;
			TreeEntry* text_size;
			TreeEntry* priority_list;

			TreeEntry* lock_position;
			TreeEntry* x;
			TreeEntry* y;
		}
		bool* b_mia_circles = new bool( false );
		bool* b_tp_active = new bool( false );
		TreeEntry* qwer_spells = nullptr;
		TreeEntry* draw_fow;
		TreeEntry* qwer_spells_rounding;
		TreeEntry* spell_lvl_style;
		TreeEntry* spell_lvl_base_rounding;
		TreeEntry* spell_lvl_point_rounding;
		TreeEntry* qwer_icons = nullptr;
		TreeEntry* qwer_icons_unleveled;
		TreeEntry* gold_tracker = nullptr;
		TreeEntry* s1s2_spells = nullptr;
		TreeEntry* xp_bar = nullptr;
		TreeEntry* xp_bar_rounding;
		TreeEntry* show_permanently = nullptr;
		TreeEntry* apm_enabled = nullptr;
		TreeEntry* apm_tracker_interval = nullptr;
		TreeEntry* apm_elements = nullptr;
		TreeEntry* trackers_me = nullptr;
		TreeEntry* trackers_allies = nullptr;
		TreeEntry* trackers_enemies = nullptr;
		TreeEntry* trackers_active_item = nullptr;
		TreeEntry* trackers_zhonya_item = nullptr;
		TreeEntry* trackers_cleanse_item = nullptr;
		TreeEntry* trackers_lifeline_item = nullptr;
		TreeEntry* trackers_yuumi_item_qwe = nullptr;
		TreeEntry* trackers_yuumi_item_r = nullptr;
		TreeEntry* trackers_yuumi_item_sums = nullptr;
		TreeEntry* trackerscolor_1 = nullptr;
		TreeEntry* trackerscolor_2 = nullptr;
		TreeEntry* trackers_outline_leveled = nullptr;
		//TreeEntry* trackers_outline_unleveled = nullptr;
		TreeEntry* spells_text_size = nullptr;
		//TreeEntry* xp_bar_y_offset = nullptr;
		//TreeEntry* xp_bar_y_size = nullptr;
		//TreeEntry* spells_y_offset = nullptr;
		//TreeEntry* hp_bar_x_offset = nullptr;
		//TreeEntry* hp_bar_y_offset = nullptr;

		TreeEntry* item_style;

		//TreeEntry* hp_auto_size_mode = nullptr;
		//TreeEntry* hp_width_offset_custom = nullptr;
		//TreeEntry* hp_height_offset_custom = nullptr;

		TreeEntry* monster_tracker = nullptr;
		TreeEntry* monster_tracker_text_color;
		TreeEntry* monster_tracker_textheader_color;
		TreeEntry* monster_tracker_text_active_color;
		TreeEntry* monster_tracker_header_color;
		TreeEntry* monster_tracker_background_color;
		TreeEntry* monster_text_header;

		TreeEntry* draw_3d_last_attack;
		TreeEntry* _3d_animate_attack;

		namespace glow
		{
			TreeEntry* enabled;
			TreeEntry* color;
			TreeEntry* blur;
			TreeEntry* th;
			TreeEntry* time;
		}

		namespace aatokill
		{
			TreeEntry* enabled;
			TreeEntry* only_if_x_aa;
			TreeEntry* aa_count;
			TreeEntry* position;

			TreeEntry* color_text;
			TreeEntry* color_text_aa;
			TreeEntry* color_background;
		}

		TreeEntry* fow_tracker_mode;
		TreeEntry* monster_tracker_x;
		TreeEntry* monster_tracker_y;
		bool monster_tracker_moving = false;
		vector monster_tracker_vdiff = vector::zero;

		TreeEntry* path_me;
		TreeEntry* path_ally;
		TreeEntry* path_enemy;
		TreeEntry* hide_visible;
		TreeEntry* text_inside_path;

		TreeEntry* path_me_color;
		TreeEntry* path_ally_color;
		TreeEntry* path_enemy_color;

		TreeEntry* spell_casts_ally;
		TreeEntry* spell_casts_enemy;
		TreeEntry* spell_casts_duration;
		TreeEntry* spell_casts_size;

		TreeEntry* minimap_allies;
		TreeEntry* minimap_self;
		TreeEntry* minimap_enemies;

		TreeEntry* map_icon_size;
		TreeEntry* map_icon_monster_size;
		TreeEntry* health_based_hp;
		TreeEntry* map_health_color;

		TreeEntry* map_draw_r;
		TreeEntry* map_draw_sums;
		TreeEntry* map_draw_monsters;
		TreeEntry* map_draw_monsters_time_attack;
		TreeEntry* map_draw_objective_time_attack;
		TreeEntry* map_draw_respawn_t;
		TreeEntry* map_circle_attack;
		TreeEntry* attack_circle_color;
		TreeEntry* map_draw_respawn_t_progressbar;
		TreeEntry* map_draw_respawn_t_progressbar_color;
		TreeEntry* map_draw_respawn_t_size;
		TreeEntry* map_draw_mini_monsters;
		//TreeEntry* map_draw_plants;

		TreeEntry* map_colors_sums;
		TreeEntry* map_r_color;
		TreeEntry* map_sums_color;
		TreeEntry* map_sums_color_text;

		TreeEntry* draw_fake;
		TreeEntry* draw_real;

		TreeEntry* animate_fake;
		TreeEntry* draw_fake_color;
		TreeEntry* draw_real_color;

		TreeEntry* mia_circles;
		TreeEntry* mia_circles_3d;

		TreeEntry* mia_circle_color;
		TreeEntry* mia_circle3d_color;
		TreeEntry* max_draw_time;

		TreeEntry* spell_casts_arc_clr;
		std::map<std::uint32_t, TreeEntry*> color_map;

		namespace xp
		{
			TreeEntry* shared_floating_text;
			TreeEntry* shared_xp_text;
		}

		namespace tp
		{
			TreeEntry* track_tp;
			TreeEntry* track_tp_ally;

			TreeEntry* color_enemy;
			TreeEntry* color_ally;
		}

		namespace revive
		{
			TreeEntry* track_revive;
			TreeEntry* track_revive_ally;

			TreeEntry* color_enemy;
			TreeEntry* color_ally;
		}

		namespace buffs
		{
			TreeEntry* track_buffs;
			TreeEntry* track_buffs_ally;
			TreeEntry* track_buffs_self;

			TreeEntry* immunity;
			TreeEntry* self_enchance;
			TreeEntry* enchanters;

			bool* b_active;
		}

#pragma region minion_ranges
		TreeEntry* ally_xp_range;
		TreeEntry* enemy_xp_range;
		TreeEntry* strict_inside_check;

		TreeEntry* thickness_xp;
		TreeEntry* ally_color;
		TreeEntry* ally_color_inside;
		TreeEntry* enemy_color;
#pragma endregion

		bool* path_tracker_active;
		bool* spell_tracker_active;
		bool* spell_tracker_casts_active;

		bool* monster_tracker_active;
		bool* map_tracker_active;

		bool* tracker_active_clone;

		bool* xp_ranges_active;
	}

	auto get_priority_list_zones( ) -> std::vector<ProrityCheckItem> {
		std::vector<ProrityCheckItem> m_priority_list = { };

		for ( auto&& x : entitylist->get_enemy_heroes( ) )
		{
			if ( !x || !x->is_valid( ) )
				continue;

			auto is_active = cfg::lastseen::enabled->get_int( ) == 2 || ( cfg::lastseen::enabled->get_int( ) == 1 && A_::is_jungler( x ) );
			auto display_name = x->get_model( );
			if ( A_::is_jungler( x ) )
				display_name += " (jungler)";

			m_priority_list.push_back( { x->get_network_id( ), display_name, is_active, nullptr } );
		}

		return m_priority_list;
	}

	void create_menu( )
	{
		auto base_menu = menu->get_tab( "utility" );
		auto hpbar_menu = base_menu->add_tab( "_hpbar_menu", "Tracker" );
		if ( hpbar_menu )
		{
			hpbar_menu->add_separator( "_tracker", " - - Tracker - - " );

			hpbar_menu->add_tab( "cd_mgr", "Cooldown tracker" ); { }

			auto show_turret = hpbar_menu->add_tab( "show_turret", "Show turret" );
			auto show_inhibitor = hpbar_menu->add_tab( "show_inhibitor", "Show inhibitor" );
			
			structures::load_menu( show_inhibitor, show_turret );

			auto show_teleport = hpbar_menu->add_tab( "show_teleport", "Teleport" );
			{
				show_teleport->add_separator( "tp_sep", " - -  Teleport  - - " );

				auto spell = database->get_spell_by_hash( spell_hash( "SummonerTeleport" ) );

				if ( spell && spell->get_icon_texture_by_index( 0 ) )
					show_teleport->set_texture( spell->get_icon_texture_by_index( 0 ) );

				cfg::tp::track_tp = show_teleport->add_checkbox( "track_tp", "Track teleport", true );
				cfg::tp::track_tp_ally = show_teleport->add_checkbox( "track_tp_ally", "Track teleport ally", false );

				show_teleport->add_separator( "tp_sep_clr", " - -  Colors  - - " );
				float clrenemy [ 4 ]; A_::d3d_to_menu( utility::COLOR_CORAL, clrenemy );
				float clrally [ 4 ]; A_::d3d_to_menu( utility::COLOR_LGREEN, clrally );

				cfg::tp::color_enemy = show_teleport->add_colorpick( "color_enemy", "Enemy", clrenemy );
				cfg::tp::color_ally = show_teleport->add_colorpick( "color_ally", "Ally", clrally );

				auto on_change = [ ]( TreeEntry* ) {
					*cfg::b_tp_active = cfg::tp::track_tp->get_bool( );
				};

				cfg::tp::track_tp->add_property_change_callback( on_change );
				show_teleport->set_assigned_active( cfg::tp::track_tp );
			}

			auto show_revive = hpbar_menu->add_tab( "show_revive", "Revive" );
			{
				show_revive->add_separator( "rev_sep", " - -  Revive  - - " );

				cfg::revive::track_revive = show_revive->add_checkbox( "track_tp", "Track revives", true );
				cfg::revive::track_revive_ally = show_revive->add_checkbox( "track_tp_ally", "Track revives ally", false );

				show_revive->add_separator( "tp_sep_clr", " - -  Colors  - - " );
				float clrenemy [ 4 ]; A_::d3d_to_menu( utility::COLOR_CORAL, clrenemy );
				float clrally [ 4 ]; A_::d3d_to_menu( utility::COLOR_LGREEN, clrally );

				cfg::revive::color_enemy = show_revive->add_colorpick( "color_enemy", "Enemy", clrenemy );
				cfg::revive::color_ally = show_revive->add_colorpick( "color_ally", "Ally", clrally );

				show_revive->set_assigned_active( cfg::revive::track_revive );

				auto item_ga = database->get_item_by_id( ItemId::Guardian_Angel );
				if ( item_ga )
				{
					auto texture = item_ga->get_texture( );
					if ( texture.first )
					{
						auto descriptor = create_texture_descriptor( texture.first, { texture.second.x, texture.second.y, texture.second.z, texture.second.w } );
						show_revive->set_texture( descriptor );
					}
				}
			}

			auto show_buffs = hpbar_menu->add_tab( "show_buffs", "Buffs" );
			{
				show_buffs->add_separator( "buffs_general_sep", " - - Track buffs - - " );
				cfg::buffs::track_buffs = show_buffs->add_checkbox( "track_buffs", "Track buffs", true );
				cfg::buffs::track_buffs_ally = show_buffs->add_checkbox( "track_buffs_ally", "Track buffs ally", false );
				cfg::buffs::track_buffs_self = show_buffs->add_checkbox( "track_buffs_self", "Track buffs self", false );

				show_buffs->add_separator( "buffs_wl_sep", " - - Whitelist - - " );
				cfg::buffs::immunity = show_buffs->add_checkbox( "immunity", "Immunity", true );
				cfg::buffs::self_enchance = show_buffs->add_checkbox( "self_enchance", "Self enhance", true );
				cfg::buffs::enchanters = show_buffs->add_checkbox( "enchanters", "Enchanter", false );

				cfg::buffs::b_active = new bool( false );

				auto click = [ ]( TreeEntry* s ) {
					*cfg::buffs::b_active = cfg::buffs::track_buffs->get_bool( ) || cfg::buffs::track_buffs_ally->get_bool( ) || cfg::buffs::track_buffs_self->get_bool( );
				};

				click( nullptr );
				cfg::buffs::track_buffs->add_property_change_callback( click );
				cfg::buffs::track_buffs_ally->add_property_change_callback( click );
				cfg::buffs::track_buffs_self->add_property_change_callback( click );
				show_buffs->set_assigned_active( cfg::buffs::b_active );
			}

			auto aa_to_kill_tracker = hpbar_menu->add_tab( "aa_to_kill_tracker", "AA to kill" );
			{
				aa_to_kill_tracker->add_separator( "aa_to_kill_tracker_sep", " - - AA to kill - - " );
				cfg::aatokill::enabled = aa_to_kill_tracker->add_checkbox( "enabled", "Enabled", false );
				cfg::aatokill::only_if_x_aa = aa_to_kill_tracker->add_checkbox( "only_if_x_aa", "Only if {x} AA lethal", false );
				cfg::aatokill::aa_count = aa_to_kill_tracker->add_slider( "aa_count", " ^ - AA count", 10, 3, 50 );

				aa_to_kill_tracker->add_separator( "aa_to_kill_trackerc_sep", " - - Customization - - " );
				cfg::aatokill::position = aa_to_kill_tracker->add_combobox( "position", "Position", { { "Right", nullptr }, { "Center", nullptr }, { "Left", nullptr } }, 0 );
				float def_clr_txet [ 4 ]; A_::d3d_to_menu( utility::COLOR_WHITE, def_clr_txet );
				float def_clr_txetaa [ 4 ]; A_::d3d_to_menu( utility::COLOR_RED, def_clr_txetaa );
				float def_clr_back [ 4 ] = { 0.f, 0.f, 0.f, 0.f };
				cfg::aatokill::color_text = aa_to_kill_tracker->add_colorpick( "color_text", "Text color", def_clr_txet );
				cfg::aatokill::color_text_aa = aa_to_kill_tracker->add_colorpick( "color_text_aa", "AA count color", def_clr_txetaa );
				cfg::aatokill::color_background = aa_to_kill_tracker->add_colorpick( "color_background", "Background color", def_clr_back );

				auto only_if_x_aa_click = [ ]( TreeEntry* s ) {
					cfg::aatokill::aa_count->is_hidden( ) = !s->get_bool( );
				};
				only_if_x_aa_click( cfg::aatokill::only_if_x_aa );
				cfg::aatokill::only_if_x_aa->add_property_change_callback( only_if_x_aa_click );
				aa_to_kill_tracker->set_assigned_active( cfg::aatokill::enabled );
			}

			auto aa_tracker = hpbar_menu->add_tab( "aa_tracker", "Hero ranges" );
			hero_ranges::load_menu( aa_tracker );

			auto m_tracker = hpbar_menu->add_tab( "m_tracker", "XP tracker" );
			{
				cfg::xp_ranges_active = new bool( false );
				m_tracker->add_separator( "xp_general_sep", " - - XP ranges - - " );
				cfg::ally_xp_range = m_tracker->add_checkbox( "ally_xp_range", "Ally XP range", false );
				cfg::enemy_xp_range = m_tracker->add_checkbox( "enemy_xp_range", "Enemy XP range", false );
				cfg::strict_inside_check = m_tracker->add_checkbox( "strict_inside_check", "Strict inside check", false );
				cfg::strict_inside_check->set_tooltip( "Check if you are outside of any minion's XP range, instead of checking if you are inside of any minion's XP range" );

				m_tracker->add_separator( "xp_colors_sep", " - - Colors - - " );
				float ally_color [ 4 ] = { 99.f / 255.f, 110.f / 255.f, 114.f / 255.f, 0.4f };
				float ally_color_inside [ 4 ] = { 116.f / 255.f, 185.f / 255.f, 255.f / 255.f, 0.7f };

				float enemy_color [ 4 ] = { 225.f / 255.f, 112.f / 255.f, 85.f / 255.f, 0.8f };

				cfg::thickness_xp = m_tracker->add_slider( "thickness_xp", "Thickness", 2, 1, 5 );
				cfg::ally_color = m_tracker->add_colorpick( "ally_color", "Ally color", ally_color );
				cfg::ally_color_inside = m_tracker->add_colorpick( "ally_color_inside", "Ally color (inside)", ally_color_inside );

				cfg::enemy_color = m_tracker->add_colorpick( "enemy_color", "Enemy color", enemy_color );

				m_tracker->add_separator( "xp_sharedgeneral_sep", " - - Shared XP tracker - - " );
				cfg::xp::shared_floating_text = m_tracker->add_checkbox( "shared_floating_text", "Floating text on shared XP", true );
				cfg::xp::shared_floating_text->set_tooltip( "Doesn't work with OBS bypass!" );
				cfg::xp::shared_xp_text = m_tracker->add_checkbox( "shared_xp_text", "Shared XP text", true );

#pragma region cb
				auto click_xp_range = [ ]( TreeEntry* ) {
					*cfg::xp_ranges_active = cfg::ally_xp_range->get_bool( ) || cfg::enemy_xp_range->get_bool( ) || cfg::xp::shared_floating_text->get_bool( );
				};

				click_xp_range( nullptr );
				cfg::ally_xp_range->add_property_change_callback( click_xp_range );
				cfg::enemy_xp_range->add_property_change_callback( click_xp_range );;

				m_tracker->set_assigned_active( cfg::xp_ranges_active );
#pragma endregion
			}

			auto clone_tracker = hpbar_menu->add_tab( "clone_tracker", "Clone Tracker" );
			{
				cfg::tracker_active_clone = new bool( false );
				clone_tracker->add_separator( "clone_tracker_sep", " - - Clone Tracker - - " );
				cfg::draw_real = clone_tracker->add_checkbox( "_draw_real", "Draw real", true );
				cfg::draw_fake = clone_tracker->add_checkbox( "_draw_fake", "Draw clone", true );

				//rgb(52, 152, 219)
				//fake rgb(255, 121, 121)
				clone_tracker->add_separator( "colors_sep", " - - Colors - - " );
				float def_fake_color [ 4 ] = { 255.f / 255.f, 121.f / 255.f, 121.f / 255.f, 0.12f };
				float def_real_color [ 4 ] = { 52.f / 255.f, 152.f / 255.f, 219.f / 255.f, 1.f };

				cfg::animate_fake = clone_tracker->add_checkbox( "animate_fake", "Animation", true );
				cfg::draw_real_color = clone_tracker->add_colorpick( "draw_real_color", "Real color", def_real_color );
				cfg::draw_fake_color = clone_tracker->add_colorpick( "draw_fake_color", "Fake color", def_fake_color );

#pragma region cb
				auto fake_real_click = [ ]( TreeEntry* )
				{
					*cfg::tracker_active_clone = cfg::draw_fake->get_bool( ) || cfg::draw_real->get_bool( );
				};
				fake_real_click( nullptr );
				cfg::draw_real->add_property_change_callback( fake_real_click ); cfg::draw_fake->add_property_change_callback( fake_real_click );
				clone_tracker->set_assigned_active( cfg::tracker_active_clone );
#pragma endregion cb
			}

			auto spell_tracker = hpbar_menu->add_tab( "_spell_tracker", "Spell, Item, Script HUD" );
			{
				spell_tracker->add_separator( "_draw_whitelist", " - - Spells - - " );
				cfg::qwer_spells = spell_tracker->add_checkbox( "_qwer_spells", "QWER spells", true );
				cfg::qwer_icons = spell_tracker->add_combobox( "_qwer_icons_mode", "QWER icons", { { "Disabled", nullptr }, { "Normal", nullptr }, { "Compact", nullptr } }, 2 );
				cfg::qwer_icons_unleveled = spell_tracker->add_checkbox( "qwer_icons_unleveled", "QWER icons unleveled", false );
				cfg::s1s2_spells = spell_tracker->add_combobox( "_s1s2_spells_mode", "Summoner spells", { { "Disabled", nullptr }, { "Normal", nullptr }, { "Compact", nullptr } }, 2 );
				cfg::qwer_spells_rounding = spell_tracker->add_slider( "qwer_spells_rounding", "Icons rounding", 90, 0, 90 );
				cfg::draw_fow = spell_tracker->add_checkbox( "draw_fow", "Draw in FoW", false );

#if !defined( __TEST )
				cfg::draw_fow->is_hidden( ) = true;
				cfg::draw_fow->set_bool( false );
#endif
				auto tab_spell_lvl = spell_tracker->add_tab( "spell_lvl_tab", "Spell level" );
				{
					tab_spell_lvl->add_separator( "sep_lvl", " - -  Spell level  - - " );
					cfg::spell_lvl_style = tab_spell_lvl->add_combobox( "spell_lvl_style", "Style", { { "Disabled", nullptr }, { "Bars", nullptr }, { "Side text", nullptr } }, 2 );
					cfg::spell_lvl_base_rounding = tab_spell_lvl->add_slider( "spell_lvl_base_rounding", "Base rounding", 90, 0, 90 );
					cfg::spell_lvl_point_rounding = tab_spell_lvl->add_slider( "spell_lvl_point_rounding", "Level point rounding", 90, 0, 90 );

					auto spell_lvl_style_change = [ ]( TreeEntry* s ) {
						cfg::spell_lvl_point_rounding->is_hidden( ) = s->get_int( ) != 1;
						cfg::spell_lvl_base_rounding->is_hidden( ) = s->get_int( ) == 0;
					};
					cfg::spell_lvl_style->add_property_change_callback( spell_lvl_style_change );
					spell_lvl_style_change( cfg::spell_lvl_style );
				}

				auto tab_xp = spell_tracker->add_tab( "tab_xp", "XP" );
				{
					tab_xp->add_separator( "tab_xp_sep", " - -  XP bar  - - " );
					cfg::xp_bar = tab_xp->add_checkbox( "_xp_bar", "Enabled", true );
					cfg::xp_bar_rounding = tab_xp->add_checkbox( "b_xp_bar_rounding", "Rounding", true );
				}

				auto tab_yuumi = spell_tracker->add_tab( "tab_yuumi", "Yuumi (attached)" );
				{
					tab_yuumi->add_separator( "tab_yuumi_sep", " - -  Yuumi  - - " );
					cfg::trackers_yuumi_item_qwe = tab_yuumi->add_checkbox( "_trackers_yuumi_item_qwe", "QWE", true );
					cfg::trackers_yuumi_item_r = tab_yuumi->add_checkbox( "_trackers_yuumi_item_r", "R", true );
					cfg::trackers_yuumi_item_sums = tab_yuumi->add_checkbox( "_trackers_yuumi_item_sums", "Summoner spells", true );
				}

				spell_tracker->add_separator( "_trackers_items", " - - Items - - " );
				{
					cfg::gold_tracker = spell_tracker->add_checkbox( "_gold_tracker", "Gold tracker", true );
#ifndef __TEST
					cfg::gold_tracker->is_hidden( ) = true;
					cfg::gold_tracker->set_bool( false );
#endif
					cfg::item_style = spell_tracker->add_combobox( "item_style_0", "Item style", { { "Normal", nullptr }, { "Compact", nullptr } }, 1 ); //
					auto items_wl_tab = spell_tracker->add_tab( "items_wl_tab", "Whitelist" );
					{
						items_wl_tab->add_separator( "items_wl_sep", " - -  Whitelist  - - " );
						{
							auto galeforce_item = database->get_item_by_id( ItemId::Galeforce )->get_texture( );
							auto galeforce_icon = create_texture_descriptor( galeforce_item.first, { galeforce_item.second.x, galeforce_item.second.y, galeforce_item.second.z, galeforce_item.second.w } );
							cfg::trackers_active_item = items_wl_tab->add_checkbox( "_trackers_active_item", "Active item", true );
							cfg::trackers_active_item->set_texture( galeforce_icon );
						}

						{
							auto stasis_item = database->get_item_by_id( ItemId::Commencing_Stopwatch )->get_texture( );
							auto stasis_icon = create_texture_descriptor( stasis_item.first, { stasis_item.second.x, stasis_item.second.y, stasis_item.second.z, stasis_item.second.w } );

							cfg::trackers_zhonya_item = items_wl_tab->add_checkbox( "_trackers_zhonya_item", "Stasis item", true );
							cfg::trackers_zhonya_item->set_texture( stasis_icon );
						}

						{
							auto qss_item = database->get_item_by_id( ItemId::Quicksilver_Sash )->get_texture( );
							auto qss_icon = create_texture_descriptor( qss_item.first, { qss_item.second.x, qss_item.second.y, qss_item.second.z, qss_item.second.w } );
							//auto qss_icon = database->get_spell_by_hash( spell_hash( "SummonerBoost" ) )->get_icon_texture_by_index( 0 );
							cfg::trackers_cleanse_item = items_wl_tab->add_checkbox( "_trackers_cleanse_item", "Cleanse item", true );
							cfg::trackers_cleanse_item->set_texture( qss_icon );
						}

						{
							auto shieldbow_item = database->get_item_by_id( ItemId::Immortal_Shieldbow )->get_texture( );
							auto shieldbow_icon = create_texture_descriptor( shieldbow_item.first, { shieldbow_item.second.x, shieldbow_item.second.y, shieldbow_item.second.z, shieldbow_item.second.w } );

							cfg::trackers_lifeline_item = items_wl_tab->add_checkbox( "_trackers_lifeline_item", "Shield item", true );
							cfg::trackers_lifeline_item->set_texture( shieldbow_icon );
						}
					}
				}

				auto sep = spell_tracker->add_separator( "_action_tracker", " - - Script tracker (?) - - " );
				{
					sep->set_tooltip( "Can be used to estimate if someone is scripting" );

					cfg::apm_enabled = spell_tracker->add_checkbox( "_apm_enabled", "Enabled", true, true );
					cfg::show_permanently = spell_tracker->add_checkbox( "_show_permanently", "Show permanently", true, true );
					cfg::apm_tracker_interval = spell_tracker->add_slider( +"_apm_tracker_interval", "Interval ( s )", 1, 1, 60 );
					cfg::apm_elements = spell_tracker->add_prority_list( "_action_tracker_elements", "Elements",
						{
							{ buff_hash( "Count per interval" ), "Count per interval", true, nullptr },
						{ buff_hash( "Peak" ), "Peak", true, nullptr },
						{ buff_hash( "Average" ), "Average", true, nullptr },
						{ buff_hash( "Animation cancels" ), "Animation cancels", true, nullptr },
						{ buff_hash( "Exploits detection" ), "Exploits detection", true, nullptr } },
						false );
				}

				static float clr_active_color [ 4 ] = { 52.f / 255.f, 73.f / 255.f, 94.f / 255.f, 1.f };
				static float clr_keybind_active_color [ 4 ] = { 29.f / 255.f, 209.f / 255.f, 161.f / 255.f, 1.f };
				spell_tracker->add_separator( "_trackers_colors", " - - Customization - - " );
				cfg::spells_text_size = spell_tracker->add_slider( "_spells_text_size", "Spells text size", renderer->screen_height( ) == 2160 ? 30 : 18, 0, 50 );
				cfg::trackerscolor_1 = spell_tracker->add_colorpick( "_hpbar_color1", "Color 1", clr_active_color );
				cfg::trackerscolor_2 = spell_tracker->add_colorpick( "_hpbar_color2", "Color 2", clr_keybind_active_color );
				cfg::trackers_outline_leveled = spell_tracker->add_checkbox( "trackers_outline_leveled", "Outline spells", true );
				//cfg::trackers_outline_unleveled = spell_tracker->add_checkbox( "trackers_outline_unleveled", "Outline unleveled spells", false );

				spell_tracker->add_button( "btn_preset_01", "Preset midnight blue" )->add_property_change_callback( [ ]( TreeEntry* ) {
					static float clr_active_color [ 4 ] = { 52.f / 255.f, 73.f / 255.f, 94.f / 255.f, 1.f };
					static float clr_keybind_active_color [ 4 ] = { 29.f / 255.f, 209.f / 255.f, 161.f / 255.f, 1.f };

					cfg::trackerscolor_1->set_color( clr_active_color );
					cfg::trackerscolor_2->set_color( clr_keybind_active_color );
				} );

				spell_tracker->add_button( "btn_preset_02", "Preset purple" )->add_property_change_callback( [ ]( TreeEntry* ) {
					static float clr_active_color [ 4 ] = { 47.f / 255.f, 49.f / 255.f, 145.f / 255.f, 1.f };
					static float clr_keybind_active_color [ 4 ] = { 179.f / 255.f, 55.f / 255.f, 113.f / 255.f, 1.f };

					cfg::trackerscolor_1->set_color( clr_active_color );
					cfg::trackerscolor_2->set_color( clr_keybind_active_color );
				} );

				spell_tracker->add_button( "btn_preset_03", "Old preset" )->add_property_change_callback( [ ]( TreeEntry* ) {
					static float clr_active_color [ 4 ] = { 204.f / 255.f, 200.f / 255.f, 177.f / 255.f, 1.f };
					static float clr_keybind_active_color [ 4 ] = { 69.f / 255.f, 65.f / 255.f, 56.f / 255.f, 1.f };

					cfg::trackerscolor_1->set_color( clr_active_color );
					cfg::trackerscolor_2->set_color( clr_keybind_active_color );
				} );

				spell_tracker->add_separator( "_trackers_whitelist", " - - Whitelist - - " );
				cfg::trackers_me = spell_tracker->add_checkbox( "_trackers_me", "Me", true );
				cfg::trackers_allies = spell_tracker->add_checkbox( "_trackers_allies", "Allies", true );
				cfg::trackers_enemies = spell_tracker->add_checkbox( "_trackers_enemies", "Enemies", true );

#pragma region callback_logic
				auto has_item_yuumi = false;
				auto yuumi_q_icon = ( uint32_t* )nullptr,
					yuumi_r_icon = ( uint32_t* )nullptr,
					yuumi_s1_icon = ( uint32_t* )nullptr;
				for ( auto&& x : entitylist->get_all_heroes( ) )
				{
					if ( !x )
						continue;

					if ( x->get_champion( ) == champion_id::Yuumi )
					{
						has_item_yuumi = true;
						yuumi_q_icon = x->get_spell( spellslot::q )->get_icon_texture( );
						yuumi_r_icon = x->get_spell( spellslot::r )->get_icon_texture( );
						yuumi_s1_icon = x->get_spell( spellslot::summoner1 )->get_icon_texture( );
						break;
					}
				}
				if ( !has_item_yuumi )
					cfg::trackers_yuumi_item_qwe->is_hidden( ) = cfg::trackers_yuumi_item_r->is_hidden( ) = cfg::trackers_yuumi_item_sums->is_hidden( ) = true;
				else
				{
					cfg::trackers_yuumi_item_qwe->set_texture( yuumi_q_icon );
					cfg::trackers_yuumi_item_r->set_texture( yuumi_r_icon );
					cfg::trackers_yuumi_item_sums->set_texture( yuumi_s1_icon );
				}

				cfg::spell_tracker_active = new bool( false );
				const auto on_spell_click = [ ]( TreeEntry* ) {
					*cfg::spell_tracker_active = cfg::qwer_spells->get_bool( ) || cfg::s1s2_spells->get_bool( ) || cfg::xp_bar->get_bool( ) ||
						( cfg::trackers_yuumi_item_qwe->get_bool( ) && !cfg::trackers_yuumi_item_qwe->is_hidden( ) ) ||
						( cfg::trackers_yuumi_item_r->get_bool( ) && !cfg::trackers_yuumi_item_r->is_hidden( ) ) ||
						( cfg::trackers_yuumi_item_sums->get_bool( ) && !cfg::trackers_yuumi_item_sums->is_hidden( ) ) ||
						cfg::apm_enabled->get_bool( ) || cfg::trackers_active_item->get_bool( ) || cfg::trackers_zhonya_item->get_bool( ) ||
						cfg::trackers_cleanse_item->get_bool( ) || cfg::trackers_lifeline_item->get_bool( );
				};
				on_spell_click( nullptr );
				cfg::qwer_spells->add_property_change_callback( on_spell_click );
				cfg::s1s2_spells->add_property_change_callback( on_spell_click );
				cfg::xp_bar->add_property_change_callback( on_spell_click );

				cfg::trackers_active_item->add_property_change_callback( on_spell_click );
				cfg::trackers_zhonya_item->add_property_change_callback( on_spell_click );
				cfg::trackers_cleanse_item->add_property_change_callback( on_spell_click );
				cfg::trackers_lifeline_item->add_property_change_callback( on_spell_click );

				cfg::trackers_yuumi_item_qwe->add_property_change_callback( on_spell_click );
				cfg::trackers_yuumi_item_r->add_property_change_callback( on_spell_click );
				cfg::trackers_yuumi_item_sums->add_property_change_callback( on_spell_click );
				cfg::apm_enabled->add_property_change_callback( on_spell_click );

				spell_tracker->set_assigned_active( cfg::spell_tracker_active );
#pragma endregion callback_logic
			}

			float clr_c [ 4 ] = { 250.f / 255.f, 177.f / 255.f, 160.f / 255.f, 1.f };
			auto minimap_trackers = hpbar_menu->add_tab( "_minimap_tracker", "Minimap" );
			{
				minimap_trackers->add_separator( "_minimap_trackers_whitelist", " - - Whitelist - - " );
				cfg::minimap_allies = minimap_trackers->add_checkbox( "minimap_allies", "Allies", false );
				cfg::minimap_self = minimap_trackers->add_checkbox( "minimap_self", "Me", false );
				cfg::minimap_enemies = minimap_trackers->add_checkbox( "minimap_enemies", "Enemies", true );

				minimap_trackers->add_separator( "_minimap_trackers_whitelist_jungle", " - - Monsters - - " );
				auto camptimers_tab = minimap_trackers->add_tab( "camptimers_tab", "Camp timers" );
				{
					camptimers_tab->add_separator( "sep_1", " - -  Camp timers  - - " );
					fow_tracker->update_minimap = camptimers_tab->add_checkbox( "update_minimap", "Update camp status", true );
					cfg::map_draw_respawn_t = camptimers_tab->add_checkbox( "map_draw_respawn_t", "Camp timers", true );
					cfg::map_draw_respawn_t_progressbar = camptimers_tab->add_prority_list( "map_draw_respawn_t_progressbar_t", "Progress bar", 
					{
						{ buff_hash( "Epic" ), "Epic monsters", true, nullptr },
						{ buff_hash( "Crabs" ), "Crabs", true, nullptr },
						{ buff_hash( "Buffs" ), "Buffs", true, nullptr },
						{ buff_hash( "Other" ), "Other monsters", false, nullptr },
					}, false );

					camptimers_tab->add_separator( "sep_2", " - -  Customization  - - " );
					cfg::map_draw_respawn_t_progressbar_color = camptimers_tab->add_colorpick( "map_draw_respawn_t_progressbar_color", "Progress bar color", clr_c );
					cfg::map_draw_respawn_t_size = camptimers_tab->add_slider( "map_draw_respawn_t_size", "Timers text size",
						static_cast< uint32_t >( std::clamp( 16.f * renderer->get_dpi_factor( ) / 1.8f, 8.f, 22.f ) ), 8, 22 );
				}
				
				cfg::map_draw_monsters = minimap_trackers->add_checkbox( "map_draw_monsters", "Icons", true );
				cfg::map_circle_attack = minimap_trackers->add_checkbox( "map_circle_attack", "Circles (attacked)", true );
				fow_tracker->draw_circle_epic = minimap_trackers->add_checkbox( "draw_circle_epic", "Circle (attacked) - objective", true );
				cfg::map_draw_monsters_time_attack = minimap_trackers->add_checkbox( "map_draw_monsters_time_attack", "Time since last attack", true );
				cfg::map_draw_objective_time_attack = minimap_trackers->add_checkbox( "map_draw_objective_time_attack", "Time since last attack (objective)", true );

				float clr_attack_c [ 4 ]; A_::d3d_to_menu( utility::COLOR_APPLE_GREEN, clr_attack_c );
				cfg::attack_circle_color = minimap_trackers->add_colorpick( "attack_circle_color", "Circle color (attacked)", clr_attack_c );
				cfg::map_draw_mini_monsters = minimap_trackers->add_checkbox( "map_draw_mini_monsters", "Small monsters icons", false );

				minimap_trackers->add_separator( "_minimap_trackers_extra", " - - Extra - - " );
				cfg::map_draw_r = minimap_trackers->add_combobox( "map_draw_r", "Draw R spell", { { "Disabled", nullptr }, { "Icon", nullptr }, { "Color", nullptr } }, 2 );
				cfg::map_draw_sums = minimap_trackers->add_combobox( "map_draw_sums", "Draw summoner spells", { { "Disabled", nullptr }, { "Icon", nullptr }, { "Color", nullptr } }, 1 );

				minimap_trackers->add_separator( "_minimap_trackers_colors", " - - Colors - - " );
				cfg::health_based_hp = minimap_trackers->add_checkbox( "health_based_hp", "Health-based hp color", true );

				float def_clr_hp [ 4 ] = { 39.f / 255.f, 174.f / 255.f, 96.f / 255.f, 1.f };
				float def_clr_r [ 4 ] = { 52.f / 255.f, 152.f / 255.f, 219.f / 255.f, 1.f };
				cfg::map_health_color = minimap_trackers->add_colorpick( "map_health_color", "Health color", def_clr_hp );
				cfg::map_r_color = minimap_trackers->add_colorpick( "map_r_color", "R color", def_clr_r );

				float def_clr_sum [ 4 ] = { 246.f / 255.f, 229.f / 255.f, 141.f / 255.f, 1.f };
				float def_clr_sum_t [ 4 ] = { 50.f / 255.f, 50.f / 255.f, 50.f / 255.f, 1.f };
				cfg::map_sums_color = minimap_trackers->add_colorpick( "map_sums_color", "Summoners colors", def_clr_sum );
				cfg::map_sums_color_text = minimap_trackers->add_colorpick( "map_sums_color_text", "Summoners text colors", def_clr_sum_t );

				cfg::map_icon_size = minimap_trackers->add_slider( "map_icon_size", "Icon size hero",
					static_cast< uint32_t >( 25.f * ( renderer->get_dpi_factor( ) ) ), 15, 100 );

				cfg::map_icon_monster_size = minimap_trackers->add_slider( "map_icon_monster_size", "Icon size monster",
					static_cast< uint32_t >( 20.f * ( renderer->get_dpi_factor( ) ) ), 15, 100 );

#pragma region cb
				const auto& health_based_hp_click = [ ]( TreeEntry* self ) {
					cfg::map_health_color->is_hidden( ) = self->get_bool( );
				};
				health_based_hp_click( cfg::health_based_hp );
				cfg::health_based_hp->add_property_change_callback( health_based_hp_click );

				const auto& map_draw_r_click = [ ]( TreeEntry* self ) {
					cfg::map_r_color->is_hidden( ) = self->get_int( ) != 2;
				};
				map_draw_r_click( cfg::map_draw_r );
				cfg::map_draw_r->add_property_change_callback( map_draw_r_click );

				const auto& map_draw_sums_click = [ ]( TreeEntry* self ) {
					cfg::map_sums_color->is_hidden( ) = cfg::map_sums_color_text->is_hidden( ) = self->get_int( ) != 2;
				};
				map_draw_sums_click( cfg::map_draw_sums );
				cfg::map_draw_sums->add_property_change_callback( map_draw_sums_click );

				cfg::map_tracker_active = new bool( false );
				const auto& minimap_element_click = [ ]( TreeEntry* )
				{
					*cfg::map_tracker_active = cfg::minimap_allies->get_bool( ) || cfg::minimap_self->get_bool( ) ||
						cfg::minimap_enemies->get_bool( ) || cfg::map_draw_monsters->get_bool( ) || cfg::map_draw_mini_monsters->get_bool( );
				};

				const auto& map_draw_monsters_click = [ ]( TreeEntry* s ) {
					cfg::map_draw_mini_monsters->is_hidden( ) = !s->get_bool( );
				};
				minimap_element_click( nullptr );

				map_draw_monsters_click( cfg::map_draw_monsters );
				cfg::map_draw_monsters->add_property_change_callback( map_draw_monsters_click );

				cfg::minimap_allies->add_property_change_callback( minimap_element_click );
				cfg::minimap_self->add_property_change_callback( minimap_element_click );
				cfg::minimap_enemies->add_property_change_callback( minimap_element_click );
				cfg::map_draw_monsters->add_property_change_callback( minimap_element_click );
				cfg::map_draw_mini_monsters->add_property_change_callback( minimap_element_click );
				minimap_trackers->set_assigned_active( cfg::map_tracker_active );
#pragma endregion cb
			}

			auto spell_tracker_casts = hpbar_menu->add_tab( "_spell_tracker_casts", "Spell, Blink Tracker" );
			{
				spell_tracker_casts->add_separator( "spell_tracker_casts_common0", " - - Common - - " );
				cfg::spell_tracker_casts_active = new bool( false );
				spell_tracker_casts->set_assigned_active( cfg::spell_tracker_casts_active );

				cfg::spell_casts_enemy = spell_tracker_casts->add_checkbox( "_castsenemy_enabled", "Enemies", true );
				cfg::spell_casts_ally = spell_tracker_casts->add_checkbox( "_casts_ally_enabled", "Allies", true );

				cfg::spell_casts_duration = spell_tracker_casts->add_slider( "spell_casts_duration", "Duration ( s )", 4, 1, 15 );
				cfg::spell_casts_size = spell_tracker_casts->add_slider( "spell_casts_size", "Size ( % )", 100, 60, 200 );

				spell_tracker_casts->add_separator( "spell_tracker_casts_colors0", " - - Colors - - " );
				float default_color_arc [ 4 ] = { 51.f / 255.f, 217.f / 255.f, 178.f / 255.f, 1.f };
				cfg::spell_casts_arc_clr = spell_tracker_casts->add_colorpick( "spell_casts_arc_clr", "Arc color", default_color_arc );

				spell_tracker_casts->add_separator( "spell_tracker_casts_colors1", " - - Colors spells - - " );

				float default_color_dash [ 4 ] = { 199.f / 255.f, 236.f / 255.f, 238.f / 255.f, 1.f };
				float default_color_flash [ 4 ] = { 246.f / 255.f, 229.f / 255.f, 141.f / 255.f, 1.f };
				float default_color_ghost [ 4 ] = { 52.f / 255.f, 152.f / 255.f, 219.f / 255.f, 1.f };
				float default_color_ignite [ 4 ] = { 255.f / 255.f, 121.f / 255.f, 121.f / 255.f, 1.f };
				float default_color_smite [ 4 ] = { 0.f, 148.f / 255.f, 50.f / 255.f, 1.f };
				float default_color_cleanse [ 4 ] = { 106.f / 255.f, 137.f / 255.f, 204.f / 255.f, 1.f };
				float default_color_exhaust [ 4 ] = { 248 / 255.f, 194 / 255.f, 145 / 255.f, 1.f };
				float default_color_heal [ 4 ] = { 46.f / 255.f, 213.f / 255.f, 115.f / 255.f };
				float default_color_barrier [ 4 ] = { 255.f / 255.f, 204.f / 255.f, 204.f / 255.f, 1.f };
				float default_color_snowball [ 4 ] = { 241.f / 255.f, 242.f / 255.f, 246.f / 255.f, 1.f };

				cfg::color_map =
				{
					{ 0, spell_tracker_casts->add_colorpick( "color_default", "Default color", default_color_dash ) },
					{ spell_hash( "summonerflash" ), spell_tracker_casts->add_colorpick( "color_flash", "Flash color", default_color_flash ) },
					{ spell_hash( "summonerhaste" ), spell_tracker_casts->add_colorpick( "color_ghost", "Ghost color", default_color_ghost ) },
					{ spell_hash( "summonerdot" ), spell_tracker_casts->add_colorpick( "color_ignite", "Ignite color", default_color_ignite ) },
					{ spell_hash( "summonersmite" ), spell_tracker_casts->add_colorpick( "color_smite", "Smite color", default_color_smite ) },
					{ spell_hash( "summonerboost" ), spell_tracker_casts->add_colorpick( "color_cleanse", "Cleanse color", default_color_cleanse ) },
					{ spell_hash( "summonerexhaust" ), spell_tracker_casts->add_colorpick( "color_exhaust", "Exhaust color", default_color_exhaust ) },
					{ spell_hash( "summonerheal" ), spell_tracker_casts->add_colorpick( "default_color_heal", "Heal color", default_color_heal ) },
					{ spell_hash( "summonerbarrier" ), spell_tracker_casts->add_colorpick( "default_color_barrier", "Barrier color", default_color_barrier ) },
					{ spell_hash( "summonersnowball" ), spell_tracker_casts->add_colorpick( "default_color_snowball", "Snowball color", default_color_snowball ) }
				};
#pragma region on_casts_change
				const auto on_casts_change = [ ]( TreeEntry* ) {
					*cfg::spell_tracker_casts_active = cfg::spell_casts_ally->get_bool( ) || cfg::spell_casts_enemy->get_bool( );
				};
				on_casts_change( nullptr );
				cfg::spell_casts_enemy->add_property_change_callback( on_casts_change );
				cfg::spell_casts_ally->add_property_change_callback( on_casts_change );

				auto icon_flash = database->get_spell_by_hash( spell_hash( "SummonerFlash" ) );
				if ( icon_flash && icon_flash->get_icon_texture_by_index( 0 ) )
					spell_tracker_casts->set_texture( icon_flash->get_icon_texture_by_index( 0 ) );
#pragma endregion on_casts_change
			}

			auto monster = hpbar_menu->add_tab( "_fow_tracker", "FoW Tracker" );
			{
				float def_miacircle_clr [ 4 ] = { 52.f / 255.f, 152.f / 255.f, 219.f / 255.f, 0.8f };
				monster->add_separator( "champion_tracker", " - - Champion tracker - - " );
				cfg::fow_tracker_mode = monster->add_prority_list( "fow_tracker", "FoW tracker", { { 1, "Native", true, nullptr }, { 2, "Minimap", true, nullptr } }, false, true );
				auto glowtab = monster->add_tab( "glowtab", "MIA glow" );
				{
					glowtab->add_separator( "glowtab_sep", " - -  Glow  - - " );
					cfg::glow::enabled = glowtab->add_checkbox( "enabled", "Enabled", true );
					cfg::glow::enabled->add_property_change_callback( [ ]( TreeEntry* s ) {
						if ( s->get_bool( ) )
							for ( auto&& x : entitylist->get_enemy_heroes( ) )
								if ( x && x->is_valid( ) && glow->is_glowed( x ) )
									glow->remove_glow( x );

						for ( auto&& x : m_glow_champions )
							x.second = false;
					} );

					float fowglowclr [ 4 ]; A_::d3d_to_menu( utility::COLOR_RED, fowglowclr );

					auto cb_change = [ ]( TreeEntry* s ) {
						for ( auto&& x : m_glow_champions )
							x.second = false;
					};

					cfg::glow::color = glowtab->add_colorpick( "color", "Color", fowglowclr ); cfg::glow::color->add_property_change_callback( cb_change );
					cfg::glow::blur = glowtab->add_slider( "blur", "Blur", 1, 1, 10 ); cfg::glow::blur->add_property_change_callback( cb_change );
					cfg::glow::th = glowtab->add_slider( "th", "Thickness", 1, 1, 10 ); cfg::glow::th->add_property_change_callback( cb_change );
					cfg::glow::time = glowtab->add_slider( "time", "Time", 8, 1, 60 ); cfg::glow::time->add_property_change_callback( cb_change );

					glowtab->set_assigned_active( cfg::glow::enabled );
				}

				auto miacircletab = monster->add_tab( "miacircletab", "MIA circles" );
				{
					miacircletab->add_separator( "mia_circle_sep_0", " - -  World  - - " );
					cfg::mia_circles_3d = miacircletab->add_checkbox( "mia_circles3d", "Enabled", false );
					cfg::mia_circle3d_color = miacircletab->add_colorpick( "mia_circle3d_color", "Color", def_miacircle_clr );

					miacircletab->add_separator( "mia_circle_sep_1", " - -  Minimap  - - " );
					cfg::mia_circles = miacircletab->add_checkbox( "mia_circles", "Enabled", false );
					cfg::mia_circle_color = miacircletab->add_colorpick( "mia_circle_color", "Color", def_miacircle_clr );

					auto option_click = [ ]( TreeEntry* ) {
						*cfg::b_mia_circles = cfg::mia_circles_3d->get_bool( ) || cfg::mia_circles->get_bool( );
					};
					cfg::mia_circles->add_property_change_callback( option_click );
					cfg::mia_circles_3d->add_property_change_callback( option_click );
					option_click( nullptr );

					miacircletab->set_assigned_active( cfg::b_mia_circles );
				}
				cfg::max_draw_time = monster->add_slider( "max_draw_time", "Max draw time ( s )", 18, 5, 30 );

				fow_tracker->max_draw_time = cfg::max_draw_time;
				//fow_tracker->mia_circle_color = cfg::mia_circle_color;

				monster->add_separator( "monster_tracker", " - - Monster tracker - - " );
				auto tab_monster_glow = monster->add_tab( "monster_glow", "Glow" );
				{
					tab_monster_glow->add_separator( "monster_glow_sep", "Monster glow" );
					fow_tracker->monster_glow = tab_monster_glow->add_checkbox( "monster_glow", "Enabled", false );

					float color_glow_def [ 4 ]; A_::d3d_to_menu( utility::COLOR_LPURPLE, color_glow_def );
					fow_tracker->monster_glow_color = tab_monster_glow->add_colorpick( "monster_glow_color", "Color", color_glow_def );
					fow_tracker->monster_glow_blur = tab_monster_glow->add_slider( "monster_glow_blur", "Blur", 1, 1, 10 );
					fow_tracker->monster_glow_thickness = tab_monster_glow->add_slider( "monster_glow_thickness", "Thickness", 1, 1, 10 );

					tab_monster_glow->set_assigned_active( fow_tracker->monster_glow );
				}

				auto tab_monster_world = monster->add_tab( "monster_world", "3D icons" );
				{
					tab_monster_world->add_separator( "sep1", " - -  3D icons  - - " );
					fow_tracker->draw_3d_jg = tab_monster_world->add_checkbox( "draw_3d_jg", "Enabled", true );
					cfg::_3d_animate_attack = tab_monster_world->add_checkbox( "3d_animate_attack", "Animate aggro", true );
					cfg::draw_3d_last_attack = tab_monster_world->add_checkbox( "draw_3d_last_attack", "Time since last attack", true );

					tab_monster_world->add_separator( "sep2", " - -  Colors  - - " );
					float def_circle_clr [ 4 ] = { 52.f / 255.f, 152.f / 255.f, 219.f / 255.f, 1.f };
					fow_tracker->aggro_circle_color = tab_monster_world->add_colorpick( "aggro_circle_color", "Circle color", def_circle_clr );
					//float clr_resp_def [ 4 ] = { 116.f / 255.f, 185.f / 255.f, 255.f / 255.f, 255.f / 255.f };
					fow_tracker->draw_3d_color_resparning = tab_monster_world->add_colorpick( "draw_3d_color_resparning", "Color respawning", clr_c );

					float clr_atk_def [ 4 ] = { 255.f / 255.f, 71.f / 255.f, 87.f / 255.f, 255.f / 255.f };
					fow_tracker->draw_3d_clr_atk = tab_monster_world->add_colorpick( "draw_3d_clr_atk", "Text color: under attack", clr_atk_def );

					clr_atk_def [ 3 ] = 120.f / 255.f;
					fow_tracker->draw_3d_clr_anim = tab_monster_world->add_colorpick( "draw_3d_clr_anim", "Animation color: under attack", clr_atk_def );

					tab_monster_world->set_assigned_active( fow_tracker->draw_3d_jg );
				}

				auto tab_objective_hud = monster->add_tab( "tab_objective_hud", "Objective HUD" );
				{
					tab_objective_hud->add_separator( "tab_objective_hud_sep", " - -  Objective HUD  - - " );
					cfg::monster_tracker = tab_objective_hud->add_checkbox( "objective_tracker", "Enabled", true );

					for ( auto i = 0; i <= 3; i++ )
					{
						clr_active_color [ i ] /= 255.f;
						clr_background_color [ i ] /= 255.f;
						clr_active_text_color [ i ] /= 255.f;
						clr_text_color [ i ] /= 255.f;
						clr_inactive_text_color [ i ] /= 255.f;
					}

					cfg::monster_tracker_text_color = tab_objective_hud->add_colorpick( "monster_tracker_text_color", "Text color", dclr_text_color );
					cfg::monster_tracker_text_active_color = tab_objective_hud->add_colorpick( "monster_tracker_text_active_color", "Text color active", dclr_active_text_color );
					cfg::monster_tracker_header_color = tab_objective_hud->add_colorpick( "monster_tracker_header_color", "Header color", dclr_active_color );
					cfg::monster_tracker_textheader_color = tab_objective_hud->add_colorpick( "monster_tracker_textheader_color", "Header text color", dclr_inactive_text_color );
					cfg::monster_tracker_background_color = tab_objective_hud->add_colorpick( "monster_tracker_background_color", "Background color", dclr_background_color );

					tab_objective_hud->add_button( "purple_clr_preset_btn", "Purple color preset" )->add_property_change_callback( [ ]( TreeEntry* ) {
						cfg::monster_tracker_text_color->set_color( clr_text_color );
						cfg::monster_tracker_text_active_color->set_color( clr_active_text_color );
						cfg::monster_tracker_header_color->set_color( clr_active_color );
						cfg::monster_tracker_textheader_color->set_color( clr_inactive_text_color );
						cfg::monster_tracker_background_color->set_color( clr_background_color );
					} );

					cfg::monster_tracker_x = tab_objective_hud->add_slider( "monster_tracker_x", "Position X", 50, 0, 9999 );
					cfg::monster_tracker_y = tab_objective_hud->add_slider( "monster_tracker_y", "Position Y", 50, 0, 9999 );

					tab_objective_hud->set_assigned_active( cfg::monster_tracker );
				}
				
				monster->add_separator( "other_sep", " - -  Other  - - " );
				fow_tracker->local_ping = monster->add_checkbox( "local_ping", "Local ping objective attack", true );

				cfg::monster_tracker_x->is_hidden( ) = cfg::monster_tracker_y->is_hidden( ) = true;

				fow_tracker->cfg_ref = cfg::fow_tracker_mode;

				cfg::monster_tracker_active = new bool( false );
				const auto& on_click_monster_stuff = [ ]( TreeEntry* )
				{
					auto fow_enabled = false;
					for ( auto&& x : cfg::fow_tracker_mode->get_prority_sorted_list( ) )
						if ( x.is_active )
							fow_enabled = true;

					*cfg::monster_tracker_active = cfg::monster_tracker->get_bool( ) || fow_enabled;
				};
				on_click_monster_stuff( nullptr );
				cfg::monster_tracker->add_property_change_callback( on_click_monster_stuff );
				cfg::fow_tracker_mode->add_property_change_callback( on_click_monster_stuff );
				monster->set_assigned_active( cfg::monster_tracker_active );
			}

			auto path_tracker = hpbar_menu->add_tab( "_path_tracker", "Path Tracker" );
			{
				path_tracker->add_separator( "_path_tracker_sep_wl", " - - Whitelist - - " );

				cfg::path_me = path_tracker->add_checkbox( "_path_me", "Me", false );
				cfg::path_ally = path_tracker->add_checkbox( "_path_ally", "Ally", false );
				cfg::path_enemy = path_tracker->add_checkbox( "_path_enemy", "Enemy", true );

				path_tracker->add_separator( "_path_tracker_sep_settings", " - - Customization - - " );
				cfg::text_inside_path = path_tracker->add_checkbox( "_text_inside_path", "Draw text inside icon", true );
				cfg::hide_visible = path_tracker->add_checkbox( "_hide_visible", "Hide for visible heroes", false );
				//rgb(116, 185, 255), rgb(255, 107, 107)
				static float default_ally_clr [ 4 ] = { 116.f / 255.f, 185.f / 255.f, 255.f / 255.f, 1.f };
				static float default_enemy_clr [ 4 ] = { 255 / 255.f, 107.f / 255.f, 107.f / 255.f, 1.f };

				cfg::path_me_color = path_tracker->add_colorpick( "path_me_color", "My path", default_ally_clr );
				cfg::path_ally_color = path_tracker->add_colorpick( "path_ally_color", "Ally path", default_ally_clr );
				cfg::path_enemy_color = path_tracker->add_colorpick( "path_enemy_color", "Enemy path", default_enemy_clr );

				cfg::path_tracker_active = new bool( false );
				path_tracker->set_assigned_active( cfg::path_tracker_active );
				const auto on_path_click = [ ]( TreeEntry* ) {
					*cfg::path_tracker_active = cfg::path_me->get_bool( ) || cfg::path_ally->get_bool( ) || cfg::path_enemy->get_bool( );
				};
				on_path_click( nullptr );
				cfg::path_me->add_property_change_callback( on_path_click );
				cfg::path_ally->add_property_change_callback( on_path_click );
				cfg::path_enemy->add_property_change_callback( on_path_click );
			}

			auto last_seen_tracker = hpbar_menu->add_tab( "_last_seen_tracker", "Last seen position" );
			{
				last_seen_tracker->add_separator( "sep", " - -  Last seen position  - - " );
				cfg::lastseen::enabled = last_seen_tracker->add_combobox( "enabled", "Mode", { { "Disabled", nullptr }, { "Jungler", nullptr }, { "All", nullptr } }, 0 );
				cfg::lastseen::hide_visible = last_seen_tracker->add_checkbox( "hide_visible", "Hide if visible", false );
				cfg::lastseen::text_size = last_seen_tracker->add_slider( "fontsz", "Font size", static_cast< int >( 16 * renderer->get_dpi_factor( ) ), 12, 50 );
				cfg::lastseen::lock_position = last_seen_tracker->add_checkbox( "lock_position", "Lock position", false );
				cfg::lastseen::x = last_seen_tracker->add_slider( "x", "_x", 100, 0, static_cast< int >( renderer->screen_width( ) ) );
				cfg::lastseen::y = last_seen_tracker->add_slider( "y", "_y", 100, 0, static_cast< int >( renderer->screen_height( ) ) );
				cfg::lastseen::zones = last_seen_tracker->add_checkbox( "zones", "[DBG]Draw all zones", false );

				auto m_priority_list = get_priority_list_zones( );
				cfg::lastseen::priority_list = last_seen_tracker->add_prority_list( "priority_list", "Whitelist", m_priority_list, false );

				auto enabled_click = [ ]( TreeEntry* ) {
					auto m_priority_list = get_priority_list_zones( );

					cfg::lastseen::priority_list->set_prority_list( m_priority_list );

					*cfg::lastseen::b_lastseen_active = cfg::lastseen::enabled->get_int( ) != 0;
				};
				enabled_click( nullptr );
				cfg::lastseen::enabled->add_property_change_callback( enabled_click );
#if !defined( __TEST )
				cfg::lastseen::zones->is_hidden( ) = true;
				cfg::lastseen::zones->set_bool( false );
#endif
				last_seen_tracker->set_assigned_active( cfg::lastseen::b_lastseen_active );
			}
			last_seen_tracker->is_hidden( ) = missioninfo->get_map_id( ) != game_map_id::SummonersRift;
		}
	}

	auto add_order_entry( champion_id id )
	{
		if ( id == champion_id::Yuumi ) //Skip item
			return;

		if ( action_map.find( id ) == action_map.end( ) )
		{
			action_map [ id ].action_data = std::vector<float >( );
			peak_action_map [ id ] = 0;

			action_map_avg [ id ] = s_average { -5.f };
		}

		if ( gametime->get_time( ) <= 15.f )
			return;

		auto time = gametime->get_time( );
		action_map [ id ].action_data.push_back( time );
	}

	void draw_paths( game_object_script x )
	{
		if ( x->is_dead( ) || x->get_real_path( ).size( ) <= 1 ) //
			return;

		if ( x->is_me( ) && !cfg::path_me->get_bool( ) )
			return;

		if ( x->is_ally( ) && !x->is_me( ) && !cfg::path_ally->get_bool( ) )
			return;

		if ( x->is_enemy( ) && !cfg::path_enemy->get_bool( ) )
			return;

		//auto has_invisible_node = false;
		if ( cfg::hide_visible->get_bool( ) && A_::is_on_screen( x->get_position( ) ) && !x->is_me( ) )
		{
			/*auto path = x->get_real_path( );
			if ( !path.empty( ) )
			{
				auto it_invis = std::find_if( path.begin( ), path.end( ), [ ]( vector a ) {
					return !a.is_on_screen( );
				} );
				has_invisible_node = it_invis != path.end( );
			}*/
			return;
		}
		/*else
			has_invisible_node = true;

		if ( !has_invisible_node )
			return;*/

		auto path = x->get_real_path( );
		auto color = ( x->is_enemy( ) ? cfg::path_enemy_color->get_color( ) : ( x->is_me( ) ? cfg::path_me_color->get_color( ) : cfg::path_ally_color->get_color( ) ) );

		auto path_length = 0.f;
		for ( auto i = 0; i < path.size( ) - 1; i++ )
		{
			auto v_end = path [ i + 1 ];
			auto v_start = i == 0 ? path [ i ].extend( v_end, x->get_bounding_radius( ) ) : path [ i ];

			if ( i == 0 && v_start.distance( v_end ) <= x->get_bounding_radius( ) * 2 )
				continue;

			path_length += v_start.distance( v_end );

			if ( !v_start.is_valid( ) || !v_end.is_valid( ) )
				continue;

			draw_manager->add_line( v_start, v_end, color, 2.f );
			if ( i > 0 )
				draw_manager->add_filled_circle( v_start, 6.f, MAKE_COLOR( 223, 249, 251, 255 ), v_start.z );
			if ( i == path.size( ) - 2 && x->get_square_icon_portrait( ) )
			{
				//arrow
				auto l_arrow = v_end + ( v_end - v_start ).rotated( degrees_to_radians( 180 - 45 ) ),
					r_arrow = v_end + ( v_end - v_start ).rotated( degrees_to_radians( 180 + 45 ) );
				draw_manager->add_line( v_end, v_end.extend( l_arrow, 45.f ), color, 2.f );
				draw_manager->add_line( v_end, v_end.extend( r_arrow, 45.f ), color, 2.f );

				auto w2s = vector::zero;
				renderer->world_to_screen( v_end.extend( v_start, -x->get_bounding_radius( ) * 2.f ), w2s );

				if ( !w2s.is_valid( ) )
					continue;

				auto v_start_icon_w = v_end.extend( v_start, -std::clamp( x->get_bounding_radius( ), 50.f, 85.f ) );
				auto v_start_icon_w2s = vector::zero;
				renderer->world_to_screen( v_start_icon_w, v_start_icon_w2s );

				if ( !v_start_icon_w2s.is_valid( ) || !x->get_square_icon_portrait( ) )
					continue;

				auto size = vector(
					20 * ( renderer->screen_height( ) > 1080.f ? renderer->screen_height( ) / 1080.f * 0.905f : 1.f ),
					20 * ( renderer->screen_height( ) > 1080.f ? renderer->screen_height( ) / 1080.f * 0.905f : 1.f ) );

				//auto size = vector( v_end_icon.distance( v_start_icon ), v_end_icon.distance( v_start_icon ) );
				//auto v_start_icon = w2s - size / 2;
				auto im_clr_rgb = cfg::text_inside_path->get_bool( ) ? 0.33f : 1.f;
				draw_manager->add_image( x->get_square_icon_portrait( ), v_start_icon_w2s - size / 2, size, 90.f, { 0, 0 }, { 1, 1 },
					{
						im_clr_rgb, im_clr_rgb, im_clr_rgb, 1.f
					} );
				draw_manager->add_circle_on_screen( v_start_icon_w2s, size.x / 2.f, utility::COLOR_DARK_GRAY, std::roundf( renderer->get_dpi_factor( ) ) );
				if ( x->get_move_speed( ) > 0.f )
				{
					auto t_reach = path_length / x->get_move_speed( );

					if ( !cfg::text_inside_path->get_bool( ) )
					{
						auto text_size = draw_manager->calc_text_size( 20, "%.1f s", t_reach );
						v_start_icon_w2s.y += size.y / 2;
						v_start_icon_w2s.x -= text_size.x / 2;

						draw_manager->add_text_on_screen( v_start_icon_w2s, utility::COLOR_WHITE, 20, "%.1f s", t_reach );
					}
					else
					{
						auto sz_font = static_cast< int >( size.x );
						auto text_size = draw_manager->calc_text_size( sz_font, t_reach < 9 ? "%.1f" : "%.0f", t_reach );
						draw_manager->add_text_on_screen( v_start_icon_w2s + size / 4.f - text_size / 2.f, utility::COLOR_WHITE, 20, t_reach < 9 ? "%.1f" : "%.0f", t_reach );
					}
				}
			}
		}
	}

	//check kindred mark
	auto draw_monsters_map( ) -> void
	{
		static std::map<int, s_animdata> m_anim_data = { };
		for ( int i = neutral_camp_id::Blue_Order; i < neutral_camp_id::Max_Camps; i++ )
		{
			auto it_anim = m_anim_data.find( i );
			if ( it_anim == m_anim_data.end( ) )
			{
				m_anim_data [ i ] = s_animdata { };
				continue;
			}

			auto monsters_nids = camp_manager->get_camp_minions( i );
			std::vector<game_object_script> v_monsters = { };

			for ( auto&& x : monsters_nids )
			{
				auto monster = entitylist->get_object_by_network_id( x );
				if ( !monster || !monster->is_valid( ) || monster->is_dead( ) || !monster->is_monster( ) )
					continue;

				v_monsters.push_back( monster );
			}

			auto draw_progressbar = false;
			auto last_attack = fow_tracker->get_last_damage_taken( i );
			switch ( i )
			{
				default:
				{
					draw_progressbar = cfg::map_draw_respawn_t_progressbar->get_prority( buff_hash( "Other" ) ).second;
					break;
				}
				case neutral_camp_id::Crab_Bottom:
				case neutral_camp_id::Crab_Top:
				{
					draw_progressbar = cfg::map_draw_respawn_t_progressbar->get_prority( buff_hash( "Crabs" ) ).second;
					break;
				}

				case neutral_camp_id::Red_Chaos:
				case neutral_camp_id::Red_Order:
				case neutral_camp_id::Blue_Chaos:
				case neutral_camp_id::Blue_Order:
				{
					draw_progressbar = cfg::map_draw_respawn_t_progressbar->get_prority( buff_hash( "Buffs" ) ).second;
					break;
				}

				case neutral_camp_id::Baron:
				case neutral_camp_id::Dragon:
				{
					draw_progressbar = cfg::map_draw_respawn_t_progressbar->get_prority( buff_hash( "Epic" ) ).second;
					break;
				}
			}

			if ( monsters_nids.empty( ) || v_monsters.empty( ) )
			{
				auto respawn_t = camp_manager->get_camp_respawn_time( i ) - gametime->get_time( );
				auto minutes = static_cast< int >( respawn_t / 60 ),
					seconds = static_cast< int >( respawn_t ) % 60;

				if ( respawn_t <= 0.f || respawn_t >= 5.f * 60.f )
					continue;

				/*auto icon = get_monster_icon( i );
				if ( icon )
				{
					draw_manager->add_image( icon, pos, icon_size, 90.f, vector::zero, { 1, 1 },
						{
							0.33f,
							0.33f,
							0.33f,
							1 } );
				}*/

				auto text_size = draw_manager->calc_text_size( cfg::map_draw_respawn_t_size->get_int( ), "%d:%02d", minutes, seconds );

				auto t_pos = camp_manager->get_camp_position( i );
				gui->get_tactical_map( )->to_map_coord( t_pos, t_pos );

				t_pos = t_pos - text_size / 2.f;

				if ( cfg::map_draw_respawn_t->get_bool( ) )
				{
					if ( draw_progressbar )
						draw_manager->add_filled_rect( t_pos - vector( 5, 4 ), t_pos + text_size + vector( 5, 4 ), MAKE_COLOR( 0, 0, 0, 180 ) );
					draw_manager->add_text_on_screen( t_pos, MAKE_COLOR( 255, 255, 255, 255 ), cfg::map_draw_respawn_t_size->get_int( ), "%d:%02d", minutes, seconds );

					if ( draw_progressbar )
					{
						auto pcent = 1.f - std::clamp( respawn_t / fow_tracker->camp_respawn_t( i ), 0.f, 1.f );
						draw_manager->add_filled_rect( t_pos - vector( 3, -text_size.y ), t_pos + vector( ( 3 + text_size.x ), 2 + text_size.y ), MAKE_COLOR( 53, 59, 72, 180 ) );
						draw_manager->add_filled_rect( t_pos - vector( 3, -text_size.y ), t_pos + vector( ( 3 + text_size.x ) * pcent, 2 + text_size.y ), cfg::map_draw_respawn_t_progressbar_color->get_color( ) );
					}
				}
				continue;
			}

			auto it_visible = std::find_if( v_monsters.begin( ), v_monsters.end( ), [ ]( game_object_script x ) {
				return x->is_visible( );
			} );

			if ( it_visible != v_monsters.end( ) )
			{
				static auto easing_fn = getEasingFunction( easing_functions::EaseOutSine );

				if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.02f )
				{
					it_anim->second.fl_current_alpha = std::clamp( easing_fn( it_anim->second.fl_current_alpha ), 0.001f, 0.999f );
					it_anim->second.t_last_animation = gametime->get_time( );
				}
			}
			else
			{
				static auto easing_fn = getEasingFunction( easing_functions::EaseInSine );
				if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.02f )
				{
					it_anim->second.fl_current_alpha = std::clamp( easing_fn( it_anim->second.fl_current_alpha ), 0.001f, 0.999f );
					it_anim->second.t_last_animation = gametime->get_time( );
				}
			}

			if ( it_anim->second.fl_current_alpha <= 0.01f )
				continue;

			auto _size = vector( static_cast< float >( cfg::map_icon_monster_size->get_int( ) ), static_cast< float >( cfg::map_icon_monster_size->get_int( ) ) );

			auto _position = camp_manager->get_camp_position( i );
			if ( !_position.is_valid( ) )
				continue;

			gui->get_tactical_map( )->to_map_coord( _position, _position );

			for ( auto&& monster : v_monsters )
			{
				auto size = _size;
				auto position = _position;

				auto name = monster->get_name( );
				std::transform( name.begin( ), name.end( ), name.begin( ),
					[ ]( unsigned char c ) { return std::tolower( c ); } );

				buff_instance_script mark_tracker = nullptr;
				auto is_mark_holder = false;

				for ( auto&& kindred : entitylist->get_all_heroes( ) )
				{
					//auto kindred = *kindred_it;
					if ( !kindred || !kindred->is_valid( ) || !kindred->has_buff( buff_hash( "kindredhitlistmonsteractivetracker" ) ) )
						continue;

					if ( kindred && kindred->is_valid( ) )
					{
						auto tmp_mark_tracker = kindred->get_buff( buff_hash( "kindredhitlistmonsteractivetracker" ) );
						if ( tmp_mark_tracker && tmp_mark_tracker->is_valid( ) )
						{
							mark_tracker = tmp_mark_tracker;
							is_mark_holder = mark_tracker->get_caster( ) && mark_tracker->get_caster( )->is_valid( ) && mark_tracker->get_caster( )->get_network_id( ) == monster->get_network_id( );
						}
					}
				}

				if ( !cfg::map_draw_mini_monsters->get_bool( ) && name.find( "mini" ) != std::string::npos )
					continue;

				if ( name.find( "mini" ) != std::string::npos || name.find( "sru_crab" ) != std::string::npos )
				{
					auto v_out = vector::zero;
					auto v_in = monster->get_position( );
					gui->get_tactical_map( )->to_map_coord( v_in, v_out );

					if ( !is_mark_holder )
						size = size / 1.75f;

					if ( v_out.is_valid( ) && name.find( "sru_crab" ) == std::string::npos )
						position = position.extend( v_out, size.x * 1.25f );
				}

				{
					auto draw_circle = cfg::map_circle_attack->get_bool( ) && name.find( "mini" ) == std::string::npos && last_attack.has_value( ) && gametime->get_time( ) - last_attack.value( ) <= 10.f &&
						i != neutral_camp_id::Dragon && i != neutral_camp_id::Baron && i != neutral_camp_id::Herlad;

					auto draw_text = (
						( cfg::map_draw_monsters_time_attack->get_bool( ) && i != neutral_camp_id::Dragon && i != neutral_camp_id::Baron && i != neutral_camp_id::Herlad ) ||
						( cfg::map_draw_objective_time_attack->get_bool( ) && ( i == neutral_camp_id::Dragon || i == neutral_camp_id::Baron || i == neutral_camp_id::Herlad ) )
						) &&
						name.find( "mini" ) == std::string::npos && last_attack.has_value( ) && gametime->get_time( ) - last_attack.value( ) <= 10.f;

					if ( cfg::map_draw_monsters->get_bool( ) )
					{
						auto im_rgb = draw_circle || draw_text ? 0.33f : 1.f;
						draw_manager->add_image( fow_tracker->get_monster_icon( i ), position - size / 2.f, size, 90.f, { 0, 0 }, { 1, 1 }, { im_rgb, im_rgb, im_rgb, it_anim->second.fl_current_alpha } );
						draw_manager->add_circle_on_screen( position, size.x / 2.f, A_::set_alpha( utility::COLOR_DARK_GRAY, static_cast< int >( 255.f * it_anim->second.fl_current_alpha ) ), 1.f );
					}

					if ( draw_circle )
						draw_manager->add_circle_on_screen( position, fmodf( size.x / 2.f * ( gametime->get_time( ) + 1.f ), size.x / 2.f ),
							A_::set_alpha( cfg::attack_circle_color->get_color( ), static_cast< unsigned long >( A_::get_alpha( cfg::attack_circle_color->get_color( ) ) * it_anim->second.fl_current_alpha ) ),
							1.f );

					if ( draw_text )
					{
						auto sz_font = static_cast< int >( size.x / 2.f );
						auto sz_time = draw_manager->calc_text_size( sz_font, "%.1f", gametime->get_time( ) - last_attack.value( ) );
						draw_manager->add_text_on_screen( position - sz_time / 2.f,
							A_::set_alpha( utility::COLOR_WHITE, static_cast< int >( 255.f * it_anim->second.fl_current_alpha ) ), sz_font, "%.1f", gametime->get_time( ) - last_attack.value( ) );
					}
				}

				if ( cfg::map_draw_monsters->get_bool( ) && is_mark_holder && mark_tracker->get_texture( ) )
				{
					auto size_mark = size * 0.75f;

					auto v_mark_pos = position + vector( size_mark.x * 0.5f, 0 ).rotated( degrees_to_radians( -30.f ) );
					draw_manager->add_image( mark_tracker->get_texture( ), v_mark_pos - size_mark / 2, size_mark, 90.f, vector::zero, { 1.f, 1.f }, { 1.f, 1.f, 1.f, it_anim->second.fl_current_alpha }, { 0, 0, 0, 0 } );

					auto fnt_sz = static_cast< int >( 12.f * ( renderer->screen_height( ) > 1080.f ? ( renderer->screen_height( ) / 1080.f * 0.6f ) : 1.f ) );
					auto text_size = draw_manager->calc_text_size( fnt_sz, "%.0fs", mark_tracker->get_remaining_time( ) );
					draw_manager->add_text_on_screen( v_mark_pos - vector( text_size.x / 2.f, -text_size.y / 2.f ), A_::set_alpha( utility::COLOR_WHITE, static_cast< int >( 255.f * it_anim->second.fl_current_alpha ) ), fnt_sz, "%.0fs", mark_tracker->get_remaining_time( ) );
				}
			}
		}
	}

	auto draw_map_portrait( game_object_script x, vector position_world, bool fow, float time_in_fow ) -> void
	{
		auto icon = x->get_square_icon_portrait( );
		if ( !icon )
			return;

		auto position = vector::zero;
		gui->get_tactical_map( )->to_map_coord( position_world, position );
		if ( !gui->get_tactical_map( )->is_coord_on_map( position ) )
			return;

		auto size = vector( static_cast< float >( cfg::map_icon_size->get_int( ) ), static_cast< float >( cfg::map_icon_size->get_int( ) ) );
		auto t_pcent = fow ? std::clamp( time_in_fow / fow_tracker->max_draw_time->get_int( ), 0.f, 1.f ) : 0.f;

		if ( fow && cfg::mia_circles->get_bool( ) )
		{
			auto distance_traveled = time_in_fow * x->get_move_speed( );
			auto points = geometry::geometry::circle_points( position_world, distance_traveled, 40 );
			for ( size_t i = 0; i < points.size( ); ++i )
			{
				auto start = points [ i ];
				auto end = points [ points.size( ) - 1 == i ? 0 : i + 1 ];

				gui->get_tactical_map( )->to_map_coord( start, start );
				gui->get_tactical_map( )->to_map_coord( end, end );

				if ( !gui->get_tactical_map( )->is_coord_on_map( start ) && !gui->get_tactical_map( )->is_coord_on_map( end ) )
					continue;

				draw_manager->add_line_on_screen( start, end, cfg::mia_circle_color->get_color( ), 1.f );
			}
		}

		draw_manager->add_image( icon, position - size / 2.f, size, 90.f, vector::zero, { 1.f, 1.f }, { fow ? 0.6f : 1.f, fow ? 0.6f : 1.f, fow ? 0.6f : 1.f, 1.f } );
		draw_manager->add_circle_on_screen( position, size.x / 2, MAKE_COLOR( 53, 59, 72, 255 ), 2.f * renderer->get_dpi_factor( ) );

		if ( fow )
		{
			auto text_size = static_cast< int >( size.x * 0.4f ); //rgb(178, 190, 195)
			auto sz = draw_manager->calc_text_size( text_size, "%.1f", time_in_fow );
			draw_manager->add_text_on_screen( position - sz / 2.f, MAKE_COLOR( 178, 190, 195, 220 ), text_size, "%.1f", time_in_fow );
		}

		if ( !fow )
		{
#pragma region hp
			auto hp = std::clamp( x->get_health_percent( ) / 100.f, 0.1f, 1.f );
			auto v_arc_points = A_::arc_points( hp, position, size.x / 2 );
			if ( v_arc_points.empty( ) )
				return;

			auto color = cfg::health_based_hp->get_bool( ) ? A_::get_color( hp * 100.f ) : cfg::map_health_color->get_color( );
			for ( auto i = 0; i < v_arc_points.size( ) - 1; i++ )
				draw_manager->add_line_on_screen( v_arc_points [ i ], v_arc_points [ i + 1 ], hp <= 0.25f ? A_::animate_color( color, 120.f, 255.f ) : color, 2.f * renderer->get_dpi_factor( ) );
#pragma endregion hp

#pragma region r
			vector point_r =
			{
				position.x + size.x / 2.f * std::cos( degrees_to_radians( 45.f ) ),
				position.y + size.y / 2.f * std::sin( degrees_to_radians( 45.f ) )
			};

			auto size_r = size / 2.25f;
			auto spell = x->get_spell( spellslot::r );

			if ( spell && spell->is_learned( ) )
			{
				auto opt_cd = g_cooldown->spell_cd( x, spellslot::r );
				auto cd = opt_cd.has_value( ) ? opt_cd.value( ) : spell->cooldown( );
				if ( cd <= 0.f )
				{
					if ( cfg::map_draw_r->get_int( ) == 1 )
					{
						auto img = spell->get_icon_texture_by_index( 0 );
						if ( img )
						{
							draw_manager->add_image( img, point_r - size_r / 2.f, size_r, 90.f );
							draw_manager->add_circle_on_screen( point_r, size_r.x / 2.f, MAKE_COLOR( 53, 59, 72, 255 ), 1.f );
						}
					}
					else if ( cfg::map_draw_r->get_int( ) == 2 )
					{
						draw_manager->add_filled_circle_on_screen( point_r, size_r.x / 2.f, cfg::map_r_color->get_color( ) );
						draw_manager->add_circle_on_screen( point_r, size_r.x / 2.f, MAKE_COLOR( 53, 59, 72, 255 ), 1.f );
					}
				}
			}
#pragma endregion r

#pragma region sums
			for ( auto i = 0; i <= 1; i++ )
			{
				auto spell = x->get_spell( ( spellslot )( ( int )spellslot::summoner1 + i ) );
				if ( spell && spell->is_learned( ) )
				{
					auto opt_cd = g_cooldown->spell_cd( x, static_cast<spellslot>( i ) );
					auto cd = opt_cd.has_value( ) ? opt_cd.value( ) : spell->cooldown( );

					if ( cd <= 0.f )
					{
						vector point_sum =
						{
							position.x + size.x / 2.f * std::cos( degrees_to_radians( -90.f - 45.f * ( i == 0 ? 1 : 3 ) ) ),
							position.y + size.y / 2.f * std::sin( degrees_to_radians( -90.f - 45.f * ( i == 0 ? 1 : 3 ) ) )
						};

						if ( cfg::map_draw_sums->get_int( ) == 1 )
						{
							auto img = spell->get_icon_texture_by_index( 0 );
							if ( img )
							{
								draw_manager->add_image( img, point_sum - size_r / 2.f, size_r, 90.f );
								draw_manager->add_circle_on_screen( point_sum, size_r.x / 2.f, MAKE_COLOR( 53, 59, 72, 255 ), 1.f );
							}
						}
						else if ( cfg::map_draw_sums->get_int( ) == 2 )
						{
							draw_manager->add_filled_circle_on_screen( point_sum, size_r.x / 2.f, cfg::map_sums_color->get_color( ) );
							draw_manager->add_circle_on_screen( point_sum, size_r.x / 2.f, MAKE_COLOR( 53, 59, 72, 255 ), 1.f );

							std::string spell_l = spell->get_name( ).find( "SummonerSmite" ) != std::string::npos ? "S" : "";

							if ( spell_l.empty( ) )
							{
								switch ( spell->get_name_hash( ) )
								{
									default:
										break;

									case spell_hash( "SummonerExhaust" ): spell_l = "E"; break;
									case spell_hash( "SummonerHaste" ): spell_l = "G"; break;
									case spell_hash( "SummonerBoost" ): spell_l = "C"; break;
									case spell_hash( "SummonerDot" ): spell_l = "I"; break;
									case spell_hash( "SummonerHeal" ): spell_l = "H"; break;
									case spell_hash( "SummonerBarrier" ): spell_l = "B"; break;
									case spell_hash( "SummonerFlash" ): spell_l = "F"; break;
									case spell_hash( "SummonerSnowball" ): spell_l = "S"; break;
								}
							}

							if ( spell_l.size( ) >= 1 )
							{
								auto text_size = draw_manager->calc_text_size( static_cast< int >( size_r.x * 0.75f ), spell_l.c_str( ) );
								draw_manager->add_text_on_screen( point_sum - text_size / 2, cfg::map_sums_color_text->get_color( ), static_cast< int >( size_r.x * 0.75f ), spell_l.c_str( ) );
							}
						}
					}
				}
			}
#pragma endregion sums
		}
		else
		{

		}
	}

	auto draw_map( game_object_script x ) -> void
	{
		if ( x->is_dead( ) ) //
			return;

		if ( x->is_me( ) && !cfg::minimap_self->get_bool( ) )
			return;

		if ( !x->is_me( ) && x->is_ally( ) && !cfg::minimap_allies->get_bool( ) )
			return;

		if ( x->is_enemy( ) && !cfg::minimap_enemies->get_bool( ) )
			return;

		auto position = x->get_position( );
		if ( !position.is_valid( ) )
			return;

		draw_map_portrait( x, position, false );
	}

#pragma region hud
	void draw_tracker( game_object_script x )
	{
		static std::map<uint32_t, s_animdata_hud> m_tracker_anim = { };
		static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
		static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

		auto it = m_tracker_anim.find( x->get_network_id( ) );
		if ( it == m_tracker_anim.end( ) )
		{
			m_tracker_anim [ x->get_network_id( ) ] = s_animdata_hud { };
			return;
		}		

		auto vis_data = fow_tracker->get_data( x->get_network_id( ) );
		auto can_draw_fow = gametime->get_time( ) - vis_data.t_last_update >= 10.f;
		if ( x->is_visible( ) )
		{
			if ( gametime->get_time( ) - it->second.t_last_animation >= 0.02f )
			{
				it->second.fl_current_alpha = std::clamp( easing_fn_in( it->second.fl_current_alpha ), 0.001f, 0.999f );
				it->second.t_last_animation = gametime->get_time( );
			}
		}
		else
		{
			auto can_draw_fow = gametime->get_time( ) - vis_data.t_last_update <= 10.f && cfg::draw_fow->get_bool( );
			if ( can_draw_fow )
			{
				if ( it->second.fl_current_alpha < 0.30f )
				{
					if ( gametime->get_time( ) - it->second.t_last_animation >= 0.02f )
					{
						it->second.fl_current_alpha = std::clamp( easing_fn_in( it->second.fl_current_alpha ), 0.001f, 0.999f );
						it->second.t_last_animation = gametime->get_time( );
					}
				}

				if ( it->second.fl_current_alpha > 0.35f )
				{
					if ( gametime->get_time( ) - it->second.t_last_animation >= 0.02f )
					{
						it->second.fl_current_alpha = std::clamp( easing_fn_out( it->second.fl_current_alpha ), 0.001f, 0.999f );
						it->second.t_last_animation = gametime->get_time( );
					}
				}
			}
			else
			{
				if ( gametime->get_time( ) - it->second.t_last_animation >= 0.02f )
				{
					it->second.fl_current_alpha = std::clamp( easing_fn_out( it->second.fl_current_alpha ), 0.001f, 0.999f );
					it->second.t_last_animation = gametime->get_time( );
				}
			}
		}

		auto&& fl_alpha = it->second.fl_current_alpha;
		if ( fl_alpha <= 0.01f )
			return;

		if ( x->is_me( ) && !cfg::trackers_me->get_bool( ) )
			return;

		if ( x->is_ally( ) && !x->is_me( ) && !cfg::trackers_allies->get_bool( ) )
			return;

		if ( x->is_enemy( ) && !cfg::trackers_enemies->get_bool( ) )
			return;

		if ( x->get_spell( spellslot::w ) && x->get_spell( spellslot::w )->get_name_hash( ) == spell_hash( "YuumiWEndWrapper" ) )
			return;

		vector4 v4_hp_bar_pos, _;
		if ( !x->get_health_bar_position( _, v4_hp_bar_pos ) )
		{
			if ( !x->is_visible( ) )
			{
				if ( !cfg::draw_fow->get_bool( ) )
					return;
				else
				{
					auto pos = vis_data.position;
					pos.z = navmesh->get_height_for_position( pos.x, pos.y ) + 65.f;
					renderer->world_to_screen( pos, pos );

					auto v4_alt = vector4( 0, 0, 105.f * renderer->get_dpi_factor( ), 11.5f * renderer->get_dpi_factor( ) );

					v4_hp_bar_pos = { pos.x, pos.y, it->second.v4_hp_bar_pos.value_or( v4_alt ).z,
						it->second.v4_hp_bar_pos.value_or( v4_alt ).w };
				}
			}
		}
		else
			it->second.v4_hp_bar_pos = v4_hp_bar_pos;

		auto bar_pos = vector( v4_hp_bar_pos.x,
			v4_hp_bar_pos.y );

		auto bar_width = v4_hp_bar_pos.z,
			bar_height = v4_hp_bar_pos.w;

		auto id = x->get_champion( );
		if ( action_map.find( id ) != action_map.end( ) && !action_map [ id ].action_data.empty( ) )
		{
			action_map [ id ].action_data.erase( std::remove_if( action_map [ id ].action_data.begin( ), action_map [ id ].action_data.end( ), [ ]( float f )
			{
				return gametime->get_time( ) - f > cfg::apm_tracker_interval->get_int( );
			} ), action_map [ id ].action_data.end( ) );
		}

		auto xp_bar_y_offset = bar_height / 1.5f;
		auto xp_bar_size = bar_height - xp_bar_y_offset;

		game_object_script yuumi = nullptr;
		std::vector<game_object_script> yuumies = { };
		for ( auto&& yuumi_buff : x->get_bufflist( ) )
		{
			if ( !yuumi_buff || !yuumi_buff->is_valid( ) )
				continue;

			if ( yuumi_buff->get_hash_name( ) != buff_hash( "YuumiWAlly" ) )
				continue;

			if ( !yuumi_buff->get_caster( ) )
				continue;

			if ( yuumi_buff->get_caster( )->get_network_id( ) != x->get_network_id( ) )
				yuumies.push_back( yuumi_buff->get_caster( ) );
		}

		if ( !yuumies.empty( ) )
		{
			std::sort( yuumies.begin( ), yuumies.end( ), [ ]( game_object_script a, game_object_script b )
			{
				auto buff_a = a->get_buff( buff_hash( "YuumiWAttach" ) ),
				buff_b = b->get_buff( buff_hash( "YuumiWAttach" ) );

			auto start_a = ( buff_a && buff_a->is_valid( ) ) ? buff_a->get_start( ) : -5.f,
				start_b = ( buff_b && buff_b->is_valid( ) ) ? buff_b->get_start( ) : -5.f;

			return start_a < start_b;
			} );

			yuumi = yuumies.front( );
		}

		//xp
		auto allow_xp_render = !x->has_buff_type(
			{
				buff_type::Knockup,
				buff_type::Knockback,
				buff_type::Blind,
				buff_type::Disarm,
				buff_type::Drowsy,
				buff_type::Berserk,
				buff_type::Charm,
				buff_type::Flee,
				buff_type::Grounded,
				buff_type::NearSight,
				buff_type::Polymorph,
				buff_type::Snare,
				buff_type::Silence,
				buff_type::Asleep,
				buff_type::Suppression
			} );
		if ( allow_xp_render && cfg::xp_bar->get_bool( ) )
		{
			auto xp_bar_width = std::min( 1.f, std::max( 0.05f, x->get_exp_percent( ) ) ) * bar_width;

			auto xp_color = x->get_exp_percent( ) >= 0.9f && x->get_level( ) < 18 ? A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 250.f * fl_alpha, 0, 0.5 ) :
				x->get_exp_percent( ) >= 0.8f && x->get_level( ) < 18 ?
				A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 250.f * fl_alpha ) :
				A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha );

			draw_manager->add_filled_rect( vector( bar_pos.x, bar_pos.y - xp_bar_y_offset ), bar_pos + vector( xp_bar_width, xp_bar_size - xp_bar_y_offset ), xp_color,
				cfg::xp_bar_rounding->get_bool( ) ? 25.f : 0.f );
			draw_manager->add_rect( vector( bar_pos.x, bar_pos.y - xp_bar_y_offset ), bar_pos + vector( bar_width, xp_bar_size - xp_bar_y_offset ),
				A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
				cfg::xp_bar_rounding->get_bool( ) ? 25.f : 0.f );
		}
		auto spells_y_offset = 0.f;

		switch ( id )
		{
			default:break;
			case champion_id::Ahri:
				if ( x->has_buff( buff_hash( "AhriR" ) ) )
					spells_y_offset += bar_height * 1.25f;
				break;
			case champion_id::Annie:
			case champion_id::Zeri:
			case champion_id::Aphelios:
			case champion_id::Belveth:
			case champion_id::Corki:
			case champion_id::Graves:
			case champion_id::Gwen:
			case champion_id::Irelia:
			case champion_id::Jhin:
			case champion_id::Mordekaiser:
			case champion_id::Pantheon:
			case champion_id::Rengar:
			case champion_id::Ryze:
			case champion_id::Samira:
			case champion_id::Seraphine:
			case champion_id::Sona:
			case champion_id::Syndra:
			case champion_id::Xayah:
			case champion_id::Gangplank:
				spells_y_offset += bar_height * 1.25f;
				break;
		}

		auto qwer_trackers_start = bar_pos;
		qwer_trackers_start.y += bar_height + bar_height * 0.9f + spells_y_offset;

		auto spells_height = bar_width / 4.f;
		auto spell_width = spells_height;

		if ( cfg::qwer_icons->get_int( ) == 2 )
			spell_width -= spell_width / 5.f;

		//qwer
		if ( cfg::qwer_spells->get_bool( ) )
		{
			for ( auto i = spellslot::q; i <= spellslot::r; i = static_cast< spellslot >( static_cast< int >( i ) + 1 ) )
			{
				auto spell = x->get_spell( i );
				if ( !spell )
					continue;

				auto is_enough_mana = x->get_mana_for_spell( i ) <= 0.f || spell->get_spell_data( )->mDoesNotConsumeMana( ) || x->get_mana( ) - x->get_mana_for_spell( i ) > 5; 
				{
#pragma region spell_icon
					if ( cfg::qwer_icons->get_int( ) > 0 && ( spell->is_learned( ) || cfg::qwer_icons_unleveled->get_bool( ) ) )
					{
						auto im_rgb = spell->is_learned( ) ? 1.f : 0.2f;
						draw_manager->add_image( spell->get_icon_texture( ), qwer_trackers_start + vector( 1 + spells_height * ( int )i, 1 ), vector( spells_height - 2, spell_width - 2 ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
							vector( 0.f, 0.f ), vector( 1.f, cfg::qwer_icons->get_int( ) == 2 ? 0.8f : 1.f ), { im_rgb, im_rgb, im_rgb, spell->is_learned( ) ? 1.f * fl_alpha : 0.66f * fl_alpha } );
					}
#pragma endregion

#pragma region cooldown_indicators
					if ( spell->is_learned( ) )
					{
						auto opt_cd = g_cooldown->spell_cd( x, i );
						auto cd = opt_cd.has_value( ) ? opt_cd.value( ) : spell->cooldown( );
						if ( cd > 0.f )
						{
							if ( !is_enough_mana )
							{
								if ( cfg::qwer_icons->get_int( ) > 0 )
									draw_manager->add_filled_rect(
										qwer_trackers_start + vector( spells_height * ( int )i, 0.f ),
										qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width ),
										A_::set_alpha( COLOR_NOT_ENOUGH_MANA_RECT, static_cast<unsigned long>( 180.f * fl_alpha ) ),
										static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
									);

								//draw_manager->add_path_filled_on_screen( drop, COLOR_NOT_ENOUGH_MANA_DROP );
							}
							else
							{
								if ( cfg::qwer_icons->get_int( ) > 0 )
									draw_manager->add_filled_rect(
										qwer_trackers_start + vector( spells_height * ( int )i, 0.f ),
										qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width ),
										( i == spellslot::r && cd < 10.f ) ?
										A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 200.f * fl_alpha )
										:
										A_::set_alpha( cfg::trackerscolor_1->get_color( ), static_cast<unsigned long>( ALPHA_SPELL_NOT_READY * fl_alpha ) ),
										static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
									);
							}

							//text
							auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), cd > 15.f ? "%.0f" : "%.1f", cd ); // > 0
							auto text_start = qwer_trackers_start +
								vector(
									spells_height * ( int )i + ( spells_height / 2 ) - text_size.x / 2,
									( cfg::qwer_icons->get_int( ) > 0 ? ( spell_width + spell_width / 5 ) / 2 : spell_width / 5 ) - ( cfg::qwer_icons->get_int( ) > 0 ? text_size.y / 2 : -5 ) );
							if ( cfg::qwer_icons->get_int( ) == 1 )
							{
								text_start = qwer_trackers_start +
									vector(
										spells_height * ( int )i + ( spells_height / 2 ) - text_size.x / 2,
										( ( spell_width ) / 2 ) - ( text_size.y / 2 ) );
							}
							else if ( cfg::qwer_icons->get_int( ) == 0 && cfg::spell_lvl_style->get_int( ) == 2 )
							{
								text_start = qwer_trackers_start +
									vector(
										spells_height * ( int )i + ( spells_height / 2 ) - text_size.x / 2,
										spell_width / 4.f * 1.25f + 3.f
									);
							}

							draw_manager->add_text_on_screen(
								text_start,
								cfg::qwer_icons->get_int( ) > 0 ? ( is_enough_mana ? cfg::trackerscolor_2->get_color( ) : cfg::trackerscolor_1->get_color( ) ) :
								A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
								//cfg::trackerscolor_2->get_color( ),
								cfg::spells_text_size->get_int( ), cd > 15.f ? "%.0f" : "%.1f", cd );
						}
						else
						{
							if ( !is_enough_mana )
							{
								if ( cfg::qwer_icons->get_int( ) > 0 )
									draw_manager->add_filled_rect(
										qwer_trackers_start + vector( spells_height * ( int )i, 0.f ),
										qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width ),
										A_::set_alpha( COLOR_NOT_ENOUGH_MANA_RECT, static_cast<unsigned long>( 180.f * fl_alpha ) ),
										static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
									);

								//draw_manager->add_path_filled_on_screen( drop, COLOR_NOT_ENOUGH_MANA_DROP );
							}
						}
					}
#pragma endregion
				}

#pragma region spell_icon_outline
				if ( cfg::trackers_outline_leveled->get_bool( ) && ( spell->is_learned( ) || cfg::qwer_icons_unleveled->get_bool( ) ) )
				{
					if ( cfg::qwer_icons->get_int( ) == 0 && cfg::spell_lvl_style->get_int( ) == 2 )
					{
						draw_manager->add_filled_rect(
							qwer_trackers_start +
							vector( spells_height * ( int )i, 0 ),
							cfg::qwer_icons->get_int( ) > 0 ?
							qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width )
							:
							qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width / ( cfg::spell_lvl_style->get_int( ) == 2 ? 4.f / 1.5f : 5.f ) + 1 ), 
							
							A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),

							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
					}
					else
					{
						draw_manager->add_rect(
							qwer_trackers_start +
							vector( spells_height * ( int )i, 0 ),
							cfg::qwer_icons->get_int( ) > 0 ?
							qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width )
							:
							qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width / ( cfg::spell_lvl_style->get_int( ) == 2 ? 4.f / 1.5f : 5.f ) + 1 ), 
							
							A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),

							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
					}
				}
#pragma endregion

				if ( spell->is_learned( ) )
				{
#pragma region spell_level_points_base
					switch ( cfg::spell_lvl_style->get_int( ) )
					{
						default:
							break;

						case 1: //bar
						{
							draw_manager->add_filled_rect(
								qwer_trackers_start + vector( spells_height * ( int )i, -( /*cfg::qwer_icons->get_bool( ) ? 1 :*/ 0 ) ),
								qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width / 5 + 1 ),
								A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
								static_cast< float >( cfg::spell_lvl_base_rounding->get_int( ) ) );
							break;
						}
						case 2: //text
						{
							draw_manager->add_filled_rect(
								qwer_trackers_start + vector( spells_height * ( int )i, 0.f ),
								qwer_trackers_start + vector( spells_height * ( int )i + ( spells_height / 4.f ) * 1.5f, ( spells_height / 4.f ) * 1.5f ),
								A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
								static_cast< float >( cfg::spell_lvl_base_rounding->get_int( ) ) );

							draw_manager->add_rect(
								qwer_trackers_start + vector( spells_height * ( int )i, 0.f ),
								qwer_trackers_start + vector( spells_height * ( int )i + ( spells_height / 4.f ) * 1.5f, ( spells_height / 4.f ) * 1.5f ),
								A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ),fl_alpha ),
								static_cast< float >( cfg::spell_lvl_base_rounding->get_int( ) ) );
							break;
						}
					}
#pragma endregion
				}

#pragma region spell_level_points
				switch ( cfg::spell_lvl_style->get_int( ) )
				{
					default:
						break;

					case 1: //bar
					{
						auto max_level = i == spellslot::r ? 3.f : 5.f;
						auto current_level = std::min( max_level, ( float )spell->level( ) );

						auto base_start = qwer_trackers_start + vector( spells_height * ( int )i, 0 );
						auto base_end = qwer_trackers_start + vector( spells_height * ( ( int )i + 1 ), spell_width / 5 );

						for ( auto x = 1; x <= max_level; x++ )
						{
							auto delta = ( base_end.x - base_start.x ) / max_level;

							auto offset = cfg::spell_lvl_point_rounding->get_int( ) > 0 ? vector( 1.f, 1.f ) : vector( 0.f, 0.f );

							if ( current_level >= x )
								draw_manager->add_filled_rect(
									base_start + vector( delta * ( x - 1 ), 0 ) + offset,
									vector( base_start.x + delta * x, base_end.y ) - vector( offset.x, 0.f ),
									cfg::qwer_icons->get_int( ) > 0 ? cfg::trackerscolor_2->get_color( ) : ( is_enough_mana ? cfg::trackerscolor_2->get_color( ) :
										A_::animate_color( COLOR_NOT_ENOUGH_MANA_RECT, 120 * fl_alpha, 220 * fl_alpha ) ),
									static_cast< float >( cfg::spell_lvl_point_rounding->get_int( ) ) );
						}
						break;
					}

					case 2: //text
					{
						auto max_level = i == spellslot::r ? 3.f : 5.f;
						auto current_level = std::min( max_level, ( float )spell->level( ) );

						if ( current_level > 0.f )
						{
							auto text_start = qwer_trackers_start + vector( spells_height * ( int )i, 0.f ) + vector( ( spells_height / 4.f ) * 1.5f, ( spells_height / 4.f ) * 1.5f ) / 2.f;
							auto sz_font = static_cast< int >( ( ( spells_height / 4.f ) * 1.5f ) );

							auto sz_text = draw_manager->calc_text_size( sz_font, "%.0f", current_level );
							draw_manager->add_text_on_screen( text_start - sz_text / 2.f, 
								A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
									sz_font, "%.0f", current_level );
						}
						break;
					}
				}
#pragma endregion
			}

			if ( yuumi && yuumi->is_valid( ) )
			{
				auto size_yuumi = vector( spells_height / 2.f, spell_width / 2.f );
				auto start_yuumi = qwer_trackers_start + vector( spells_height * 4 - size_yuumi.x, spells_height );
				if ( cfg::qwer_icons->get_int( ) == 2 )
					start_yuumi.y -= spell_width / 5.f;
				for ( auto i = spellslot::r; i >= spellslot::q; i = static_cast< spellslot >( static_cast< int >( i ) - 1 ) )
				{
					if ( i == spellslot::r )
					{
						if ( !cfg::trackers_yuumi_item_r->get_bool( ) )
							continue;
					}
					else
						if ( !cfg::trackers_yuumi_item_qwe->get_bool( ) )
							continue;

					auto spell_yuumi = yuumi->get_spell( i );
					if ( !spell_yuumi )
						continue;

					if ( spell_yuumi->is_learned( ) )
					{
						draw_manager->add_image( spell_yuumi->get_icon_texture( ), start_yuumi + vector( 1, 1 ), size_yuumi - vector( 2, 2 ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );

						auto opt_cd = g_cooldown->spell_cd( yuumi, i );
						auto cd = opt_cd.has_value( ) ? opt_cd.value( ) : spell_yuumi->cooldown( );
						if ( cd > 0.f )
						{
							draw_manager->add_filled_rect(
								start_yuumi,
								start_yuumi + size_yuumi,
								( i == spellslot::r && cd < 10.f ) ?
								A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f, 200.f )
								:
								A_::set_alpha( cfg::trackerscolor_1->get_color( ), static_cast<unsigned long>( ALPHA_SPELL_NOT_READY * fl_alpha ) ),
								static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
							);

							auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ) / 2, cd > 15.f ? "%.0f" : "%.1f", cd );
							draw_manager->add_text_on_screen(
								start_yuumi +
								size_yuumi / 2.f - text_size / 2.f,
								A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
								cfg::spells_text_size->get_int( ) / 2, cd > 15.f ? "%.0f" : "%.1f", cd );
						}
					}

					draw_manager->add_rect( start_yuumi,
						start_yuumi + size_yuumi,
						A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
						static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );

					start_yuumi = start_yuumi - vector( size_yuumi.x, 0 );
				}
			}
		}

		//sums
		spells_height -= 2;
		auto start = bar_pos + vector( bar_width + xp_bar_y_offset - 5, -xp_bar_y_offset + xp_bar_size - 3 ); // 
		auto sum_1 = x->get_spell( spellslot::summoner1 );
		auto sum_2 = x->get_spell( spellslot::summoner2 );

#pragma region summoner_spells & items

#pragma region summoners
		if ( cfg::s1s2_spells->get_int( ) > 0 )
		{
			if ( sum_1 && sum_2 && sum_2->get_spell_data( )->get_name_hash( ) == spell_hash( "SummonerFlash" ) )
				std::swap( sum_1, sum_2 );

			if ( sum_1 && sum_2 )
			{
				auto sum_w = cfg::s1s2_spells->get_int( ) == 2 ? spells_height - ( spells_height / 5 * 2 ) : spells_height;
				if ( cfg::s1s2_spells->get_int( ) == 2 )
					start.y = start.y + spells_height / 5;

				auto x_1920_offset = renderer->screen_height( ) == 1920 ? 0 : 0; //?
				auto y_1080_offset = renderer->screen_height( ) == 1080 ? 3 : 0;

				if ( sum_1->get_icon_texture( ) )
					draw_manager->add_image( sum_1->get_icon_texture( ), start + vector( 1, 1 ), vector( spells_height - 2, sum_w - 2 ),
						static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
						vector( 0.f, cfg::s1s2_spells->get_int( ) == 2 ? 0.2f : 0.f ), vector( 1, cfg::s1s2_spells->get_int( ) == 2 ? 0.8f : 1.f ), { 1.f, 1.f, 1.f, fl_alpha } );

				if ( sum_2->get_icon_texture( ) )
					draw_manager->add_image( sum_2->get_icon_texture( ), start + vector( spells_height + 1, 1 ), vector( spells_height - 2, sum_w - 2 ),
						static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
						vector( 0.f, cfg::s1s2_spells->get_int( ) == 2 ? 0.2f : 0.f ), vector( 1, cfg::s1s2_spells->get_int( ) == 2 ? 0.8f : 1.f ), { 1.f, 1.f, 1.f, fl_alpha } );

				auto opt_sum1_cd = g_cooldown->sum_cd( x, sum_1->get_name_hash( ) );
				auto opt_sum2_cd = g_cooldown->sum_cd( x, sum_2->get_name_hash( ) );

				auto sum1_cooldown = opt_sum1_cd.has_value( ) ? opt_sum1_cd.value( ) : sum_1->cooldown( );
				auto sum2_cooldown = opt_sum2_cd.has_value( ) ? opt_sum2_cd.value( ) : sum_2->cooldown( );

#pragma region cooldowns
				if ( sum1_cooldown > 0 )
				{
					draw_manager->add_filled_rect( start, start + vector( spells_height, sum_w ),
						sum1_cooldown > 20 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), ALPHA_SPELL_NOT_READY ) : A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f, 200.f ),
						static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
					);
					auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), sum1_cooldown >= 10 ? "%.0f" : "%.1f", sum1_cooldown );
					draw_manager->add_text_on_screen(
						start +
						vector(
							( spells_height / 2 ) - text_size.x / 2 + x_1920_offset,
							( sum_w / 2 ) - text_size.y / 2 + y_1080_offset ),
						cfg::trackerscolor_2->get_color( ),
						cfg::spells_text_size->get_int( ), sum1_cooldown >= 10 ? "%.0f" : "%.1f", sum1_cooldown );
				}
				if ( sum2_cooldown > 0 )
				{
					draw_manager->add_filled_rect( start + vector( spells_height, 0 ), start + vector( spells_height * 2, sum_w ),
						sum2_cooldown > 20 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), ALPHA_SPELL_NOT_READY ) : A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f, 200.f ),
						static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
					);
					auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), sum2_cooldown > 10 ? "%.0f" : "%.1f", sum2_cooldown );
					draw_manager->add_text_on_screen(
						start +
						vector(
							spells_height + ( spells_height / 2 ) - text_size.x / 2 + x_1920_offset,
							( sum_w / 2 ) - text_size.y / 2 + y_1080_offset ),
						cfg::trackerscolor_2->get_color( ),
						cfg::spells_text_size->get_int( ), sum2_cooldown >= 10 ? "%.0f" : "%.1f", sum2_cooldown );
				}
#pragma endregion
				draw_manager->add_rect( start, start + vector( spells_height, sum_w ), cfg::trackerscolor_1->get_color( ),
					static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
				draw_manager->add_rect( start + vector( spells_height, 0 ), start + vector( spells_height * 2, sum_w ), cfg::trackerscolor_1->get_color( ),
					static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
			}

			if ( yuumi && cfg::trackers_yuumi_item_sums->get_bool( ) )
			{
				auto sum_1_yuumi = yuumi->get_spell( spellslot::summoner1 ),
					sum_2_yuumi = yuumi->get_spell( spellslot::summoner2 );

				if ( sum_2_yuumi->get_spell_data( )->get_name_hash( ) == spell_hash( "SummonerFlash" ) )
					std::swap( sum_1_yuumi, sum_2_yuumi );

				if ( sum_1_yuumi && sum_2_yuumi )
				{
					auto _size_yuumi = vector( spells_height / 2.f, spell_width / 2.f );
					auto start_yuumi_ = qwer_trackers_start + vector( bar_width, _size_yuumi.y + 2 );

					if ( sum_1_yuumi->get_icon_texture( ) )
						draw_manager->add_image( sum_1_yuumi->get_icon_texture( ), start_yuumi_, _size_yuumi, static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ), { 0, 0 }, { 1, 1 }, { 1.f, 1.f, 1.f, fl_alpha } );

					if ( sum_2_yuumi->get_icon_texture( ) )
						draw_manager->add_image( sum_2_yuumi->get_icon_texture( ), start_yuumi_ + vector( 0, _size_yuumi.y ), _size_yuumi, static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ), { 0, 0 }, { 1, 1 }, { 1.f, 1.f, 1.f, fl_alpha } );

					auto opt_sum1_cd = g_cooldown->sum_cd( yuumi, sum_1_yuumi->get_name_hash( ) );
					auto opt_sum2_cd = g_cooldown->sum_cd( yuumi, sum_2_yuumi->get_name_hash( ) );

					auto sum1_cooldown_y = opt_sum1_cd.has_value( ) ? opt_sum1_cd.value( ) : sum_1_yuumi->cooldown( );
					auto sum2_cooldown_y = opt_sum2_cd.has_value( ) ? opt_sum2_cd.value( ) : sum_2_yuumi->cooldown( );

					if ( sum1_cooldown_y > 0 )
					{
						draw_manager->add_filled_rect( start_yuumi_, start_yuumi_ + _size_yuumi,
							sum1_cooldown_y > 15 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), static_cast<unsigned long>( ALPHA_SPELL_NOT_READY * fl_alpha ) ) : A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 200.f * fl_alpha ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
						);
						auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ) / 2, sum1_cooldown_y > 15 ? "%.0f" : "%.1f", sum1_cooldown_y );
						draw_manager->add_text_on_screen(
							start_yuumi_ + vector( _size_yuumi.x / 2 - text_size.x / 2, _size_yuumi.y / 2 - text_size.y / 2 ),
							A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
							cfg::spells_text_size->get_int( ) / 2, sum1_cooldown_y > 15 ? "%.0f" : "%.1f", sum1_cooldown_y );
					}
					if ( sum2_cooldown_y > 0 )
					{
						draw_manager->add_filled_rect( start_yuumi_ + vector( 0, _size_yuumi.y ), start_yuumi_ + vector( _size_yuumi.x, _size_yuumi.y * 2 + 1 ),
							sum2_cooldown_y > 15 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), static_cast<unsigned long>( ALPHA_SPELL_NOT_READY * fl_alpha ) ) : A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 200.f * fl_alpha ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
						);
						auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ) / 2, sum2_cooldown_y > 15 ? "%.0f" : "%.1f", sum2_cooldown_y );
						draw_manager->add_text_on_screen(
							start_yuumi_ + vector( 0, _size_yuumi.y ) + vector( _size_yuumi.x / 2 - text_size.x / 2, _size_yuumi.y / 2 - text_size.y / 2 ),
							A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
							cfg::spells_text_size->get_int( ) / 2, sum2_cooldown_y > 15 ? "%.0f" : "%.1f", sum2_cooldown_y );
					}

					draw_manager->add_rect( start_yuumi_, start_yuumi_ + _size_yuumi, 
						A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
						static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
					draw_manager->add_rect( start_yuumi_ + vector( 0, _size_yuumi.y ), start_yuumi_ + vector( _size_yuumi.x, _size_yuumi.y * 2 + 1 ), 
						A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
						static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
				}
			}
		}
#pragma endregion

		//items
		auto has_core = x->has_item( //220000 + x
			{
				ItemId::Galeforce, ItemId::Typhoon,
				ItemId::Galeforce_Arena, ItemId::Typhoon_Arena,

				ItemId::Ironspike_Whip,
				//static_cast<ItemId>( 220000 + static_cast<int>( ItemId::Ironspike_Whip ) ), 

			ItemId::Goredrinker, ItemId::Dreamshatter,
			ItemId::Goredrinker_Arena, ItemId::Dreamshatter_Arena,

			ItemId::Youmuus_Ghostblade, ItemId::Youmuus_Wake,
			ItemId::Youmuus_Ghostblade_Arena, ItemId::Youmuus_Wake_Arena,

			ItemId::Everfrost, ItemId::Eternal_Winter,
			ItemId::Everfrost_Arena, ItemId::Eternal_Winter_Arena,

			ItemId::Hextech_Rocketbelt, ItemId::Upgraded_Aeropack,
			ItemId::Hextech_Rocketbelt_Arena, ItemId::Upgraded_Aeropack_Arena,

			ItemId::Shurelyas_Battlesong, ItemId::Shurelyas_Requiem,
			ItemId::Shurelyas_Battlesong_Arena, ItemId::Shurelyas_Requiem_Arena,

			ItemId::Locket_of_the_Iron_Solari, ItemId::Reliquary_of_the_Golden_Dawn,
			ItemId::Locket_of_the_Iron_Solari_Arena, ItemId::Reliquary_of_the_Golden_Dawn_Arena
			//, ItemId::Immortal_Shieldbow
			} ) != spellslot::invalid;

		auto has_zhonya = x->has_item(
			{
				ItemId::Zhonyas_Hourglass,
				ItemId::Zhonyas_Hourglass_Arena,

				ItemId::Stopwatch,
				//static_cast< ItemId >( 220000 + static_cast< int >( ItemId::Stopwatch ) ),

			ItemId::Perfectly_Timed_Stopwatch,
			//static_cast< ItemId >( 220000 + static_cast< int >( ItemId::Perfectly_Timed_Stopwatch ) ),

			ItemId::Guardian_Angel,
			ItemId::Guardian_Angel_Arena
			} ) != spellslot::invalid;

		auto has_lifeline = x->has_item(
			{
				ItemId::Immortal_Shieldbow,
				ItemId::Immortal_Shieldbow_Arena,

				ItemId::Maw_of_Malmortius, ItemId::Hexdrinker,
				ItemId::Maw_of_Malmortius_Arena,

				ItemId::Steraks_Gage,
				ItemId::Steraks_Gage_Arena,

				ItemId::Seraphs_Embrace,
				ItemId::Seraphs_Embrace_Arena
			} ) != spellslot::invalid;

		auto start_items = bar_pos + vector( -xp_bar_y_offset / 2 - spells_height, -xp_bar_y_offset + xp_bar_size );
		auto start_items_zhonya = start_items - vector( spells_height, 0 );
		auto start_items_lifeline = start_items - vector( spells_height * 2, 0 );
		auto start_items_cleanse = start_items - vector( spells_height * 3, 0 );

		if ( !has_core || !cfg::trackers_active_item->get_bool( ) )
		{
			start_items.x += spells_height;
			start_items_zhonya.x += spells_height;
			start_items_lifeline.x += spells_height;
			start_items_cleanse.x += spells_height;
		}
		if ( !has_lifeline || !cfg::trackers_lifeline_item->get_bool( ) )
		{
			start_items_cleanse.x += spells_height;
		}
		if ( !has_zhonya || !cfg::trackers_zhonya_item->get_bool( ) )
		{
			start_items_lifeline.x += spells_height;
			start_items_cleanse.x += spells_height;
		}

		auto item_h = cfg::item_style->get_int( ) == 1 ? spells_height - ( spells_height / 5 * 2 ) : spells_height;
		for ( spellslot slot = spellslot::item_1; slot <= spellslot::item_6; slot = static_cast< spellslot >( static_cast< int >( slot ) + 1 ) )
		{
			auto item = x->get_item( slot );
			if ( !item )
				continue;

			auto item_sp_slot = x->get_spell( slot );
			if ( item && item_sp_slot )
			{
				auto i_item_id = item->get_item_id( );

				auto item_id = static_cast< ItemId >( i_item_id );

				auto opt_cooldown = g_cooldown->item_cd( x, item_id );
				auto cooldown = opt_cooldown.has_value( ) ? opt_cooldown.value( ) : item_sp_slot->cooldown( );

				switch ( item_id )
				{
					default:
						break;
					case ItemId::Galeforce:
					case ItemId::Typhoon:

					case ItemId::Ironspike_Whip:

					case ItemId::Stridebreaker:
					case ItemId::Dreamshatter:

					case ItemId::Goredrinker:
					case ItemId::Ceaseless_Hunger:

					case ItemId::Youmuus_Ghostblade:
					case ItemId::Youmuus_Wake:

					case ItemId::Everfrost:
					case ItemId::Eternal_Winter:

					case ItemId::Hextech_Rocketbelt:
					case ItemId::Upgraded_Aeropack:

					case ItemId::Shurelyas_Battlesong:
					case ItemId::Shurelyas_Requiem:

					case ItemId::Locket_of_the_Iron_Solari:
					case ItemId::Reliquary_of_the_Golden_Dawn:
					{
						if ( !cfg::trackers_active_item->get_bool( ) )
							break;

						auto spell = x->get_spell( slot );
						if ( spell )
						{
							auto texture = item->get_texture( );

							if ( texture.first )
								draw_manager->add_image( texture.first, start_items + vector( -spells_height + 1, -item_h + 1 ), vector( spells_height - 2, item_h - 2 ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
									{
										texture.second.x,
										texture.second.y + ( cfg::item_style->get_int( ) == 1 ? texture.second.w / 24.f : 0.f )
									},
									{
										texture.second.z,
										texture.second.w - ( cfg::item_style->get_int( ) == 1 ? texture.second.w / 24.f : 0.f )
									},
									{ 1.f, 1.f, 1.f, fl_alpha } );

							if ( cooldown > 0.0f )
							{
								draw_manager->add_filled_rect( start_items - vector( spells_height, item_h ), start_items,
									cooldown > 15 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), static_cast<unsigned long>( ALPHA_SPELL_NOT_READY * fl_alpha ) ) : 
									A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 200.f * fl_alpha ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
								);
								auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
								draw_manager->add_text_on_screen(
									start_items - vector( spells_height - spells_height / 2 + text_size.x / 2, item_h - item_h / 2 + text_size.y / 2 ),
									A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
									cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
							}
						}
						draw_manager->add_rect( start_items - vector( spells_height, item_h ), start_items, 
							A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
						break;
					}
					//盾弓
					case ItemId::Immortal_Shieldbow:
						//飲魔刀
					case ItemId::Maw_of_Malmortius:
					case ItemId::Hexdrinker:
						//手套
					case ItemId::Steraks_Gage:
						//大天使
					case ItemId::Seraphs_Embrace:
					{
						if ( !cfg::trackers_lifeline_item->get_bool( ) )
							break;

						auto spell = x->get_spell( slot );
						if ( spell )
						{
							auto texture = item_id == ItemId::Seraphs_Embrace ? database->get_item_by_id( ItemId::Immortal_Shieldbow )->get_texture( ) : item->get_texture( );

							if ( texture.first )
								draw_manager->add_image( texture.first, start_items_lifeline + vector( -spells_height + 1, -item_h + 1 ), vector( spells_height - 2, item_h - 2 ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
									{
										texture.second.x,
										texture.second.y + ( cfg::item_style->get_int( ) == 1 ? texture.second.w / 24.f : 0.f )
									},
									{
										texture.second.z,
										texture.second.w - ( cfg::item_style->get_int( ) == 1 ? texture.second.w / 24.f : 0.f )
									}, { 1.f, 1.f, 1.f, fl_alpha } );
							if ( cooldown > 0.0f )
							{
								draw_manager->add_filled_rect( start_items_lifeline - vector( spells_height, item_h ), start_items_lifeline,
									cooldown > 15 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), static_cast<unsigned long>( ALPHA_SPELL_NOT_READY * fl_alpha ) ) : A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 200.f * fl_alpha ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
								);
								auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
								draw_manager->add_text_on_screen(
									start_items_lifeline - vector( spells_height - spells_height / 2 + text_size.x / 2, item_h - item_h / 2 + text_size.y / 2 ),
									A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
									cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
							}
						}
						draw_manager->add_rect( start_items_lifeline - vector( spells_height, item_h ), start_items_lifeline, 
							A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
						break;
					}
					//金人
					case ItemId::Zhonyas_Hourglass:
					case ItemId::Stopwatch:
					//case ItemId::Perfectly_Timed_Stopwatch:
					case ItemId::Guardian_Angel:
					{
						if ( !cfg::trackers_zhonya_item->get_bool( ) )
							break;

						auto spell = x->get_spell( slot );
						if ( spell )
						{
							auto texture = item->get_texture( );
							if ( item_id == ItemId::Guardian_Angel && texture.first )
								draw_manager->add_image( texture.first, start_items_zhonya + vector( -spells_height + 1, -item_h + 1 ), vector( spells_height - 2, item_h - 2 ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
									{
										texture.second.x,
										texture.second.y + ( cfg::item_style->get_int( ) == 1 ? texture.second.w / 24.f : 0.f )
									},
									{
										texture.second.z,
										texture.second.w - ( cfg::item_style->get_int( ) == 1 ? texture.second.w / 24.f : 0.f )
									}, { 1.f, 1.f, 1.f, fl_alpha } );
							else
							{
								// ItemId::Stopwatch doesn't work :o
								auto zhonya = database->get_item_by_id( item_id == ItemId::Zhonyas_Hourglass ? ItemId::Zhonyas_Hourglass : ItemId::Commencing_Stopwatch )->get_texture( );
								//auto zhonya_icon = create_texture_descriptor( zhonya.first, { zhonya.second.x, zhonya.second.y, zhonya.second.z, zhonya.second.w } );

								if ( zhonya.first )
									draw_manager->add_image( zhonya.first, start_items_zhonya + vector( -spells_height + 1, -item_h + 1 ), vector( spells_height - 2, item_h - 2 ),
										static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
										{
											zhonya.second.x,
											zhonya.second.y + ( cfg::item_style->get_int( ) == 1 ? zhonya.second.w / 24.f : 0.f )
										},
										{
											zhonya.second.z,
											zhonya.second.w - ( cfg::item_style->get_int( ) == 1 ? zhonya.second.w / 24.f : 0.f )
										}, { 1.f, 1.f, 1.f, fl_alpha } ); //, 0.f,
							}
							if ( cooldown > 0.0f )
							{
								draw_manager->add_filled_rect( start_items_zhonya - vector( spells_height, item_h ), start_items_zhonya,
									cooldown > 15 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), static_cast<unsigned long>( ALPHA_SPELL_NOT_READY * fl_alpha ) ) : A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f * fl_alpha, 200.f * fl_alpha ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
								);
								auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
								draw_manager->add_text_on_screen(
									start_items_zhonya - vector( spells_height - spells_height / 2 + text_size.x / 2, item_h - item_h / 2 + text_size.y / 2 ),
									A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
									cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
							}
						}
						draw_manager->add_rect( start_items_zhonya - vector( spells_height, item_h ), start_items_zhonya, 
							A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
						break;
					}
					//水銀
					case ItemId::Quicksilver_Sash:
					case ItemId::Silvermere_Dawn:
					case ItemId::Mercurial_Scimitar:
					case ItemId::Mikaels_Blessing:
					{
						if ( !cfg::trackers_cleanse_item->get_bool( ) )
							break;

						auto spell = x->get_spell( slot );
						if ( spell )
						{
							bool mikael = myhero->has_item( ItemId::Mikaels_Blessing ) != spellslot::invalid;
							auto qss_item_texture = mikael ?
								database->get_item_by_id( ItemId::Mikaels_Blessing )->get_texture( ) :
								database->get_item_by_id( ItemId::Quicksilver_Sash )->get_texture( );

							if ( qss_item_texture.first )
								draw_manager->add_image( qss_item_texture.first, start_items_cleanse + vector( -spells_height + 1, -item_h + 1 ), vector( spells_height - 2, item_h - 2 ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ),
									{
										qss_item_texture.second.x,
										qss_item_texture.second.y + ( cfg::item_style->get_int( ) == 1 ? qss_item_texture.second.w / 24.f : 0.f )
									},
									{
										qss_item_texture.second.z,
										qss_item_texture.second.w - ( cfg::item_style->get_int( ) == 1 ? qss_item_texture.second.w / 24.f : 0.f )
									}, { 1.f, 1.f, 1.f, fl_alpha } ); //, 0.f,
							if ( cooldown > 0.0f )
							{
								draw_manager->add_filled_rect( start_items_cleanse - vector( spells_height, item_h ), start_items_cleanse,
									cooldown > 15 ? A_::set_alpha( cfg::trackerscolor_1->get_color( ), ALPHA_SPELL_NOT_READY ) : A_::animate_color( cfg::trackerscolor_1->get_color( ), 120.f, 200.f ),
									static_cast< float >( cfg::qwer_spells_rounding->get_int( ) )
								);
								auto text_size = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
								draw_manager->add_text_on_screen(
									start_items_cleanse - vector( spells_height - spells_height / 2 + text_size.x / 2, item_h - item_h / 2 + text_size.y / 2 ),
									A_::set_alpha_percent( cfg::trackerscolor_2->get_color( ), fl_alpha ),
									cfg::spells_text_size->get_int( ), cooldown > 15 ? "%.0f" : "%.1f", cooldown );
							}
						}
						draw_manager->add_rect( start_items_cleanse - vector( spells_height, item_h ), start_items_cleanse, 
							A_::set_alpha_percent( cfg::trackerscolor_1->get_color( ), fl_alpha ),
							static_cast< float >( cfg::qwer_spells_rounding->get_int( ) ) );
						break;
					}
				}
			}
		}
#pragma endregion

		if ( cfg::gold_tracker->get_bool( ) )
		{
			auto base_pos_gold = start + vector( 0, spells_height + 2 );

			auto text_count = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "%.0f ", x->get_gold( ) );
			draw_manager->add_text_on_screen( base_pos_gold, cfg::trackerscolor_1->get_color( ), cfg::spells_text_size->get_int( ), "%.0f ", x->get_gold( ) );
			base_pos_gold.x += text_count.x;

			draw_manager->add_text_on_screen( base_pos_gold, MAKE_COLOR( 223, 249, 251, 255 ), cfg::spells_text_size->get_int( ), "Gold" );
		}

		if ( cfg::apm_enabled->get_bool( ) &&
			action_map.find( id ) != action_map.end( ) &&
			peak_action_map.find( id ) != peak_action_map.end( ) &&
			action_map_avg.find( id ) != action_map_avg.end( ) )
		{

			auto count = action_map [ x->get_champion( ) ].action_data.empty( ) ? 0.f : static_cast< float >( action_map [ x->get_champion( ) ].action_data.size( ) );
			if ( count > 0 || cfg::show_permanently->get_bool( ) )
			{
				if ( gametime->get_time( ) - action_map_avg [ id ].t_last_entry > cfg::apm_tracker_interval->get_int( ) )
				{
					action_map_avg [ id ].t_last_entry = gametime->get_time( );
					action_map_avg [ id ].entries.push_back( count );
				}

				if ( count > peak_action_map [ id ] )
					peak_action_map [ id ] = count;

				auto _elements = cfg::apm_elements->get_prority_sorted_list( );
				_elements.erase( std::remove_if( _elements.begin( ), _elements.end( ), [ ]( ProrityCheckItem x )
				{
					return !x.is_active;
				} ), _elements.end( ) );

				if ( action_map_avg [ id ].entries.size( ) > 50 )
					action_map_avg [ id ].entries.erase( action_map_avg [ id ].entries.begin( ), action_map_avg [ id ].entries.begin( ) + ( action_map_avg [ id ].entries.size( ) - 50 ) );

				if ( _elements.size( ) > 0 )
				{

					auto anim_cancels = std::find_if( _elements.begin( ), _elements.end( ), [ ]( ProrityCheckItem x )
					{
						return x.key == buff_hash( "Animation cancels" ) && x.is_active;
					} ) != _elements.end( );
					auto exploits = std::find_if( _elements.begin( ), _elements.end( ), [ ]( ProrityCheckItem x )
					{
						return x.key == buff_hash( "Exploits detection" ) && x.is_active;
					} ) != _elements.end( );

					int menu = 0;
					if ( anim_cancels )
						menu++;
					if ( exploits )
						menu++;
					auto base_pos = start - vector( 0, ( draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "Y" ).y + 2 ) * ( _elements.size( ) - menu ) ); //
					auto base_pos_x_1 = start - vector( 0, ( draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "Y" ).y + 2 ) * ( _elements.size( ) - menu ) );

					for ( auto&& e : _elements )
					{
						if ( !e.is_active )
							continue;

						if ( e.key == buff_hash( "Count per interval" ) )
						{
							auto text_count = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "%.0f", roundf( count ) );
							auto text_count_int = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), cfg::apm_tracker_interval->get_int( ) == 1 ? " /s" : " /%ds",
								cfg::apm_tracker_interval->get_int( ) );
							auto text_size_s = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), " | ", count );

							auto color = MAKE_COLOR( 223, 249, 251, static_cast< int >( 255.f * fl_alpha ) );
							if ( count / cfg::apm_tracker_interval->get_int( ) >= 8 )
								color = A_::animate_color( MAKE_COLOR( 235, 77, 75, static_cast< int >( 255.f * fl_alpha ) ), 120.f * fl_alpha, 255.f * fl_alpha );
							else if ( count / cfg::apm_tracker_interval->get_int( ) >= 5 )
								color = MAKE_COLOR( 255, 190, 118, static_cast< int >( 255.f * fl_alpha ) );

							draw_manager->add_text_on_screen( base_pos, color, cfg::spells_text_size->get_int( ), "%.0f", roundf( count ) );
							base_pos_x_1.x += text_count.x;
							draw_manager->add_text_on_screen( base_pos_x_1, MAKE_COLOR( 245, 246, 250, static_cast<int>( 255.f*fl_alpha ) ), cfg::spells_text_size->get_int( ),
								cfg::apm_tracker_interval->get_int( ) == 1 ? " /s" : " /%ds",
								cfg::apm_tracker_interval->get_int( ) );
							base_pos_x_1.x += text_count_int.x;


							if ( anim_cancels && action_map [ id ].t_last_emote_cancel > 0.f && gametime->get_time( ) - action_map [ id ].t_last_emote_cancel <= 15.f )
							{
								draw_manager->add_text_on_screen( base_pos_x_1, MAKE_COLOR( 245, 246, 250, static_cast< int >( 255.f * fl_alpha ) ), cfg::spells_text_size->get_int( ), " | " );
								base_pos_x_1.x += text_size_s.x;

								auto text_size_anim = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "ANIM", count );
								draw_manager->add_text_on_screen( base_pos_x_1, A_::animate_color( MAKE_COLOR( 235, 77, 75, 255 ), 120.f * fl_alpha, 255.f * fl_alpha ), cfg::spells_text_size->get_int( ), "ANIM" );
								base_pos_x_1.x += text_size_anim.x;
							}

							if ( exploits && action_map [ id ].t_last_invalid_cast > 0.f && gametime->get_time( ) - action_map [ id ].t_last_invalid_cast <= 300.f )
							{
								draw_manager->add_text_on_screen( base_pos_x_1, MAKE_COLOR( 245, 246, 250, static_cast< int >( 255.f * fl_alpha ) ), cfg::spells_text_size->get_int( ), " | " );
								base_pos_x_1.x += text_size_s.x;

								auto text_size_expl = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "EXPLOIT", count );
								draw_manager->add_text_on_screen( base_pos_x_1, A_::animate_color( MAKE_COLOR( 235, 77, 75, 255 ), 120.f * fl_alpha, 255.f * fl_alpha ), cfg::spells_text_size->get_int( ), "EXPLOIT" );
								base_pos_x_1.x += text_size_expl.x;
							}

							base_pos.y += text_count.y + 2;
						}
						else if ( e.key == buff_hash( "Peak" ) )
						{
							auto text_peak = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "PEAK: " ),
								text_count_peak = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "%.0f", peak_action_map [ id ] );

							auto color_peak = MAKE_COLOR( 223, 249, 251, static_cast< int >( 255.f * fl_alpha ) );
							if ( peak_action_map [ id ] / cfg::apm_tracker_interval->get_int( ) >= 8.f )
								color_peak = A_::animate_color( MAKE_COLOR( 235, 77, 75, 255 ), 120.f * fl_alpha, 255.f );
							else if ( peak_action_map [ id ] / cfg::apm_tracker_interval->get_int( ) >= 5.f )
								color_peak = MAKE_COLOR( 255, 190, 118, static_cast< int >( 255.f * fl_alpha ) );

							draw_manager->add_text_on_screen( base_pos, MAKE_COLOR( 245, 246, 250, static_cast< int >( 255.f * fl_alpha ) ), cfg::spells_text_size->get_int( ), "PEAK: " );
							draw_manager->add_text_on_screen( base_pos + vector( text_peak.x, 0 ), color_peak, cfg::spells_text_size->get_int( ), "%.0f", peak_action_map [ id ] );
							draw_manager->add_text_on_screen( base_pos + vector( text_peak.x + text_count_peak.x, 0 ), MAKE_COLOR( 245, 246, 250, static_cast< int >( 255.f * fl_alpha ) ), cfg::spells_text_size->get_int( ),
								cfg::apm_tracker_interval->get_int( ) == 1 ? " /s" : " /%ds",
								cfg::apm_tracker_interval->get_int( ) );

							base_pos.y += text_count_peak.y + 2;
						}
						else if ( e.key == buff_hash( "Average" ) )
						{
							auto count = 0.0f;
							for ( auto x : action_map_avg [ id ].entries )
								count += x;
							count /= static_cast< float >( action_map_avg [ id ].entries.size( ) );

							auto text_avg = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "AVG: " ); // ,
							//text_count_avg = draw_manager->calc_text_size( cfg::spells_text_size->get_int( ), "%.1f", count );

							auto color_avg = MAKE_COLOR( 245, 246, 250, static_cast< int >( 255.f * fl_alpha ) );
							if ( count / cfg::apm_tracker_interval->get_int( ) >= 7.f )
								color_avg = A_::animate_color( MAKE_COLOR( 235, 77, 75, 255 ), 120.f * fl_alpha, 255.f * fl_alpha );
							else if ( count / cfg::apm_tracker_interval->get_int( ) >= 4.f )
								color_avg = MAKE_COLOR( 255, 190, 118, static_cast< int >( 255.f * fl_alpha ) );

							draw_manager->add_text_on_screen( base_pos, MAKE_COLOR( 245, 246, 250, static_cast< int >( 255.f * fl_alpha ) ), cfg::spells_text_size->get_int( ), "AVG: " );
							draw_manager->add_text_on_screen( base_pos + vector( text_avg.x, 0 ), color_avg, cfg::spells_text_size->get_int( ), "%.1f", count );
							base_pos.y += text_avg.y + 2;
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region apm_tracker
	auto sp_on_new_path( game_object_script sender, const std::vector<vector>& path, bool is_dash, float dash_speed ) -> void
	{
		if ( !sender || is_dash || !sender->is_ai_hero( ) )
			return;

		auto id = sender->get_champion( );
		add_order_entry( id );
	}

	auto sp_on_play_animation( game_object_script sender, const char* name, bool* ) -> void
	{
		if ( !sender || !sender->is_ai_hero( ) || sender->get_champion( ) == champion_id::Yuumi )
			return;

		auto id = sender->get_champion( );
		//Joke, Taunt, Dance, Laugh, Toggle
		if ( strcmp( name, "Joke" ) == 0 ||
			strcmp( name, "Taunt" ) == 0 ||
			strcmp( name, "Dance" ) == 0 ||
			strcmp( name, "Laugh" ) == 0 ||
			strcmp( name, "Toggle" ) == 0 )
		{
			if ( action_map.find( id ) != action_map.end( ) )
			{
				action_map [ id ].t_last_emote = gametime->get_time( );
				if ( action_map [ id ].t_last_cast > 0.f )
				{
					auto emote_diff = fabs( action_map [ id ].t_last_emote - action_map [ id ].t_last_cast );
					if ( emote_diff <= ANIM_DETECTION_T + ( ping->get_ping( ) / 1000.f ) )
						action_map [ id ].t_last_emote_cancel = gametime->get_time( );
				}
			}
		}
	}

	auto sp_do_cast( game_object_script sender, spell_instance_script spell ) -> void
	{
		if ( !sender || !sender->is_valid( ) || !sender->is_ai_hero( ) || !spell )
			return;

		auto id = sender->get_champion( );
		if ( spell->is_auto_attack( ) )
		{
			add_order_entry( id );

			if ( action_map.find( id ) != action_map.end( ) )
			{
				action_map [ id ].t_last_cast = gametime->get_time( );
				if ( action_map [ id ].t_last_emote > 0.f )
				{
					auto emote_diff = fabs( action_map [ id ].t_last_emote - action_map [ id ].t_last_cast );
					if ( emote_diff <= ANIM_DETECTION_T + ( ping->get_ping( ) / 1000.f ) )
						action_map [ id ].t_last_emote_cancel = gametime->get_time( );
				}
			}
		}
		else
		{
			if ( action_map.find( id ) != action_map.end( ) )
				action_map [ id ].t_last_cast = gametime->get_time( );
		}
		auto v_end = spell->get_end_position( );
		if ( spell->get_spellslot( ) >= spellslot::q && spell->get_spellslot( ) <= spellslot::r )
		{
			if ( !v_end.is_valid( ) || v_end.length_sqr( ) >= FLT_MAX )
			{
				//myhero->print_chat( 0x3, "[!]invalid cast detected: [%s:%d] (%.1f;%.1f;%.1f:%.1f)", spell->get_spell_data( )->get_name_cstr( ), v_end.x, v_end.y, v_end.z, v_end.length( ) );
				if ( action_map.find( id ) != action_map.end( ) )
					action_map [ id ].t_last_invalid_cast = gametime->get_time( );
			}
			else
			{
				//myhero->print_chat( 0x3, "[!]cast: [%s:%d] (%.1f;%.1f;%.1f:%.1f)", spell->get_spell_data( )->get_name_cstr( ), v_end.x, v_end.y, v_end.z, v_end.length( ) );
			}
		}
#ifdef __TEST

#endif
	}

	auto sp_stop_cast( game_object_script sender, spell_instance_script spell ) -> void
	{
		if ( !sender || !sender->is_ai_hero( ) || !spell || !spell->is_auto_attack( ) )
			return;

		auto id = sender->get_champion( );
		add_order_entry( id );
	}
#pragma endregion

	void monster_tracker_draw( )
	{
		if ( !cfg::monster_tracker->get_bool( ) || missioninfo->get_map_id( ) == game_map_id::HowlingAbyss )
			return;

		float dragon = camp_manager->get_camp_respawn_time( neutral_camp_id::Dragon ) - gametime->get_time( ),
			baron = camp_manager->get_camp_respawn_time( neutral_camp_id::Baron ) - gametime->get_time( ),
			herlad = camp_manager->get_camp_respawn_time( neutral_camp_id::Herlad ) - gametime->get_time( );

		auto cursor_pos = vector( static_cast< float >( game_input->get_cursor_pos( ).x ), static_cast< float >( game_input->get_cursor_pos( ).y ) ),
			ui_position = vector( static_cast< float >( cfg::monster_tracker_x->get_int( ) ), static_cast< float >( cfg::monster_tracker_y->get_int( ) ) );

		auto image_size = vector( 28 * renderer->get_dpi_factor( ), 28 * renderer->get_dpi_factor( ) );
		auto max_size = image_size + vector( 10, 0 ) * 2;

		std::vector<std::pair<game_object_script, std::string>> v_draw_monsters = { };
		std::vector< s_draw_monster_entry > v_draw_entries = { };

#pragma region process_alive_monsters
		auto txt_size_scaled = static_cast< int >( 17.f * renderer->get_dpi_factor( ) );
		auto max_size_temp = vector::zero;
		for ( int i = neutral_camp_id::Blue_Order; i < neutral_camp_id::Max_Camps; i++ )
		{
			auto camp_monsters_nids = camp_manager->get_camp_minions( i );
			if ( camp_monsters_nids.empty( ) )
				continue;

			std::vector< game_object_script > camp_monsters = { };
			for ( auto&& nid : camp_monsters_nids )
			{
				auto monster = entitylist->get_object_by_network_id( nid );
				if ( !monster || !monster->is_valid( ) )
					continue;

				camp_monsters.push_back( monster );
			}

			if ( camp_monsters.empty( ) )
				continue;

			std::sort( camp_monsters.begin( ), camp_monsters.end( ), [ ]( game_object_script a, game_object_script b )
			{
				return a->get_bounding_radius( ) > b->get_bounding_radius( );
			} );

			static auto color_attack = MAKE_COLOR( 250, 177, 160, 255 );

			auto&& monster = camp_monsters.front( );
			auto name = monster->get_name( );
			std::transform( name.begin( ), name.end( ), name.begin( ),
				[ ]( unsigned char c ) { return std::tolower( c ); } );

			if ( name.find( "sru_riftherald" ) != std::string::npos && herlad < 60 )
			{
				auto color_herald = herlad > 30 ? cfg::monster_tracker_text_color->get_color( ) :
					herlad > 0 ? MAKE_COLOR( 255, ( int )herlad * 8, ( int )herlad * 8, 255 ) :
					A_::animate_color( cfg::monster_tracker_text_active_color->get_color( ), 120.f, 250.f );

				if ( fow_tracker->is_objective_under_attack( neutral_camp_id::Herlad ) && !monster->is_visible( ) )
					color_herald = A_::animate_color( color_attack, 120.f, 250.f );

				std::string text2 = fow_tracker->is_objective_under_attack( neutral_camp_id::Herlad ) ? "under attack" : "alive";
				if ( herlad > 0 )
				{
					std::stringstream stream;
					stream << std::fixed << std::setprecision( 1 ) << herlad << "s";
					text2 = stream.str( );
				}
				auto text_size_1 = draw_manager->calc_text_size( txt_size_scaled, "Herald: " );
				auto text_size_2 = draw_manager->calc_text_size( txt_size_scaled, text2.c_str( ) );
				v_draw_entries.push_back( s_draw_monster_entry
					{
						{ { cfg::monster_tracker_text_color->get_color( ), "Herald: " }, { color_herald, text2 } },
					{ text_size_1, text_size_2 },
					text_size_1 + text_size_2,
					monster->get_square_icon_portrait( )
					} );
				v_draw_monsters.push_back( std::pair< game_object_script, std::string>( monster, name ) );
				max_size_temp.x = std::max( max_size_temp.x, text_size_1.x + text_size_2.x );
			}
			else if ( name.find( "sru_baron" ) != std::string::npos && baron < 60 )
			{
				auto color_baron = baron > 30 ? cfg::monster_tracker_text_color->get_color( ) :
					baron > 0 ? MAKE_COLOR( 255, ( int )baron * 8, ( int )baron * 8, 255 ) :
					A_::animate_color( cfg::monster_tracker_text_active_color->get_color( ), 120.f, 250.f );

				if ( fow_tracker->is_objective_under_attack( neutral_camp_id::Baron ) && !monster->is_visible( ) )
					color_baron = A_::animate_color( color_attack, 120.f, 250.f );

				std::string text2 = fow_tracker->is_objective_under_attack( neutral_camp_id::Baron ) ? "under attack" : "alive";
				if ( baron > 0 )
				{
					std::stringstream stream;
					stream << std::fixed << std::setprecision( 1 ) << baron << "s";
					text2 = stream.str( );
				}
				auto text_size_1 = draw_manager->calc_text_size( txt_size_scaled, "Baron: " ),
					text_size_2 = draw_manager->calc_text_size( txt_size_scaled, text2.c_str( ) );

				v_draw_entries.push_back( s_draw_monster_entry
					{
						{ { cfg::monster_tracker_text_color->get_color( ), "Baron: " }, { color_baron, text2 } },
					{ text_size_1, text_size_2 },
					text_size_1 + text_size_2,
					monster->get_square_icon_portrait( )
					} );
				v_draw_monsters.push_back( std::pair< game_object_script, std::string>( monster, name ) );
				max_size_temp.x = std::max( max_size_temp.x, text_size_1.x + text_size_2.x );
			}
			if ( name.find( "sru_dragon" ) != std::string::npos && dragon < 60 )
			{
				int color_dragon = dragon > 30 ? cfg::monster_tracker_text_color->get_color( ) :
					dragon > 0 ? MAKE_COLOR( 255, ( int )dragon * 8, ( int )dragon * 8, 255 ) :
					A_::animate_color( cfg::monster_tracker_text_active_color->get_color( ), 120.f, 250.f );

				if ( fow_tracker->is_objective_under_attack( neutral_camp_id::Dragon ) && !monster->is_visible( ) )
					color_dragon = A_::animate_color( color_attack, 120.f, 250.f );

				std::string text2 = fow_tracker->is_objective_under_attack( neutral_camp_id::Dragon ) ? "under attack" : "alive";
				if ( dragon > 0 )
				{
					std::stringstream stream;
					stream << std::fixed << std::setprecision( 1 ) << dragon << "s";
					text2 = stream.str( );
				}

				auto text_size_1 = draw_manager->calc_text_size( txt_size_scaled, "Dragon: " ),
					text_size_2 = draw_manager->calc_text_size( txt_size_scaled, text2.c_str( ) );

				v_draw_entries.push_back( s_draw_monster_entry
					{
						{ { cfg::monster_tracker_text_color->get_color( ), "Dragon: " }, { color_dragon, text2 } },
					{ text_size_1, text_size_2 },
					text_size_1 + text_size_2,
					monster->get_square_icon_portrait( )
					} );
				v_draw_monsters.push_back( std::pair< game_object_script, std::string>( monster, name ) );
				max_size_temp.x = std::max( max_size_temp.x, text_size_1.x + text_size_2.x );

				max_size = max_size + vector( 5, 0 ) + vector( 0, image_size.y + 10 );
			}
		}
		max_size = max_size + max_size_temp + vector( 10, 0 );
#pragma endregion

		auto text_size = draw_manager->calc_text_size( static_cast< int >( 13.f * renderer->get_dpi_factor( ) ), "Objectives" );
		auto height = text_size.y + ( PADDING_TEXT_Y( renderer->get_dpi_factor( ) ) ) * 2;
		auto width = max_size.x + ( PADDING_TEXT_X( renderer->get_dpi_factor( ) ) ) * 2;

#pragma region move_logic
		if ( keyboard_state->is_pressed( keyboard_game::mouse1 ) && !v_draw_entries.empty( ) )
		{
			auto hover_header = cursor_pos.x >= ui_position.x &&
				cursor_pos.x <= ( ui_position.x + width ) &&
				cursor_pos.y >= ui_position.y &&
				cursor_pos.y <= ( ui_position.y + height );

			if ( hover_header || cfg::monster_tracker_moving )
			{
				if ( !cfg::monster_tracker_vdiff.is_valid( ) )
					cfg::monster_tracker_vdiff = vector( cursor_pos.x - ui_position.x, cursor_pos.y - ui_position.y );

				auto new_safe_pos = vector(
					std::min( std::max( 0.f, cursor_pos.x - cfg::monster_tracker_vdiff.x ), renderer->screen_width( ) - max_size.x ),
					std::min( std::max( 0.f, cursor_pos.y - cfg::monster_tracker_vdiff.y ), renderer->screen_height( ) - max_size.y )
				);

				cfg::monster_tracker_x->set_int( static_cast< uint32_t >( new_safe_pos.x ) );
				cfg::monster_tracker_y->set_int( static_cast< uint32_t >( new_safe_pos.y ) );

				cfg::monster_tracker_moving = true;
			}
		}
		else
		{
			cfg::monster_tracker_moving = false;
			cfg::monster_tracker_vdiff = vector::zero;
		}

		auto safe_pos = vector(
			std::clamp( cfg::monster_tracker_x->get_int( ) * 1.f, 0.f, renderer->screen_width( ) - max_size.x ),
			std::clamp( cfg::monster_tracker_y->get_int( ) * 1.f, 0.f, renderer->screen_height( ) - max_size.y )
		);
		cfg::monster_tracker_x->set_int( static_cast< uint32_t >( safe_pos.x ) );
		cfg::monster_tracker_y->set_int( static_cast< uint32_t >( safe_pos.y ) );
#pragma endregion

#pragma region draw
		if ( !v_draw_entries.empty( ) )
		{
			auto base_position = ui_position;

			//header
			draw_manager->add_filled_rect( base_position, base_position + vector( width, height ), cfg::monster_tracker_header_color->get_color( ) );
			draw_manager->add_text_on_screen( base_position + vector(
				PADDING_TEXT_X( renderer->get_dpi_factor( ) ),
				PADDING_TEXT_Y( renderer->get_dpi_factor( ) ) ), cfg::monster_tracker_textheader_color->get_color( ), static_cast< int >( 13.f * renderer->get_dpi_factor( ) ), "Objectives" );

			base_position.y += height;

			//background
			auto background_y = ( image_size.y + PADDING_TEXT_Y( renderer->get_dpi_factor( ) ) ) * ( v_draw_entries.size( ) );
			background_y += PADDING_TEXT_Y( renderer->get_dpi_factor( ) );

			draw_manager->add_filled_rect( base_position, base_position + vector( width, background_y ), cfg::monster_tracker_background_color->get_color( ) );

			base_position.y += PADDING_TEXT_Y( renderer->get_dpi_factor( ) );

			for ( auto i = 0; i < v_draw_entries.size( ); i++ )
			{
				auto&& entry = v_draw_entries [ i ];
				auto text_position = base_position;

				if ( v_draw_entries [ i ].texture )
				{
					draw_manager->add_image( v_draw_entries [ i ].texture, text_position + vector( 7, 0 ), image_size + vector( 5, 0 ), 15.f );
					text_position = text_position + vector( 20 + image_size.x, image_size.y / 2 );
				}

				for ( auto x = 0; x < entry.text.size( ); x++ )
				{
					draw_manager->add_text_on_screen( text_position - vector( 0, entry.sizes [ x ].y / 2 ), entry.text [ x ].first, txt_size_scaled, entry.text [ x ].second.c_str( ) );
					text_position.x += entry.sizes [ x ].x + 5;
				}

				base_position.y += image_size.y + 10;
			}
		}
#pragma endregion
	}

	auto clone_tracker( ) -> void
	{
		if ( !cfg::draw_real->get_bool( ) && !cfg::draw_fake->get_bool( ) )
			return;

#ifdef __TEST
		auto champions = entitylist->get_all_heroes( );
#else
		auto champions = entitylist->get_enemy_heroes( );
#endif
		for ( auto&& real : champions )
		{
			if ( !real || !real->is_valid( ) || real->is_dead( ) || !real->is_visible( ) )
				continue;

			auto p_spell = real->get_spell( ( spellslot )63 );
			if ( !p_spell )
				continue;

			if ( p_spell->get_name_hash( ) != spell_hash( "NeekoPassive" ) )
				continue;

			auto name = real->get_base_skin_name( );
			std::transform( name.begin( ), name.end( ), name.begin( ),
				[ ]( unsigned char c ) { return std::tolower( c ); } );

			if ( name.find( "neeko" ) != std::string::npos /*&& !real->has_buff( buff_hash( "neekowhaste" ) )*/ )
				continue;

			auto min = real->get_bbox_min( ),
				max = real->get_bbox_max( );

			if ( !min.is_valid( ) || !max.is_valid( ) )
				continue;

			renderer->world_to_screen( min, min );
			renderer->world_to_screen( max, max );

			if ( min.x > max.x )
				std::swap( min.x, max.x );

			//draw_manager->add_rect( min, max, MAKE_COLOR( 255, 255, 255, 255 ) );
			auto l = std::abs( max.x - min.x ) / 3.f;

			if ( cfg::animate_fake->get_bool( ) )
			{
				min = A_::animate_vector( min, max, l / 2.f, 2.f );
				max = A_::animate_vector( max, min, l / 2.f, 2.f );
			}

			//R_CORNER_TOP
			draw_manager->add_line_on_screen( max, vector( max.x - l, max.y ), cfg::draw_real_color->get_color( ), 2.f );
			draw_manager->add_line_on_screen( max, vector( max.x, max.y + l ), cfg::draw_real_color->get_color( ), 2.f );

			//L_CORNER_TOP
			draw_manager->add_line_on_screen( vector( min.x, max.y ), vector( min.x + l, max.y ), cfg::draw_real_color->get_color( ), 2.f );
			draw_manager->add_line_on_screen( vector( min.x, max.y ), vector( min.x, max.y + l ), cfg::draw_real_color->get_color( ), 2.f );

			//L_CORNER_BOT
			draw_manager->add_line_on_screen( vector( min.x, min.y ), vector( min.x + l, min.y ), cfg::draw_real_color->get_color( ), 2.f );
			draw_manager->add_line_on_screen( vector( min.x, min.y ), vector( min.x, min.y - l ), cfg::draw_real_color->get_color( ), 2.f );

			//R_CORNER_BOT
			draw_manager->add_line_on_screen( vector( max.x, min.y ), vector( max.x - l, min.y ), cfg::draw_real_color->get_color( ), 2.f );
			draw_manager->add_line_on_screen( vector( max.x, min.y ), vector( max.x, min.y - l ), cfg::draw_real_color->get_color( ), 2.f );
		}

		//auto enemies = entitylist->get_other_minion_objects( );
		for ( auto&& x : entitylist->get_all_minions( ) )
		{
			if ( !x || !x->is_valid( ) || !x->is_ai_base( ) || x->is_ai_hero( ) || x->is_dead( ) || x->get_health( ) <= 0.f || !x->get_owner( ) /*|| x->get_type( )->get_id( ) != 3399847090*/ )
				continue;

			if ( !x->get_owner( )->is_ai_hero( ) )
				continue;

#ifndef __TEST
			if ( x->get_owner( )->is_ally( ) )
				continue;
#endif

			//myhero->print_chat( 0x3, "%s|%s", x->get_model_cstr( ), x->get_owner( )->get_model_cstr( ) );

			auto model_hash = buff_hash_real( x->get_model_cstr( ) );
			if ( std::find( std::begin( m_clone_models ), std::end( m_clone_models ), model_hash ) == std::end( m_clone_models ) )
				continue;


			//myhero->print_chat( 0x3, "%.1f|%.1f", x->get_health( ), x->get_attack_range( ) );
			if ( x->get_health( ) <= 0.f /*|| x->get_attack_range( ) <= 0.f*/ )
				continue;

			auto real = x->get_owner( );
			if ( real && real->is_valid( ) && real->is_visible( ) && cfg::draw_real->get_bool( ) && !real->is_dead( ) )
			{
				auto min = real->get_bbox_min( ),
					max = real->get_bbox_max( );

				if ( !min.is_valid( ) || !max.is_valid( ) )
					continue;

				renderer->world_to_screen( min, min );
				renderer->world_to_screen( max, max );

				A_::draw_corner_box( min, max, cfg::draw_real_color->get_color( ), true, -1.f, 2.f );
			}

			if ( cfg::draw_fake->get_bool( ) && x->is_visible( ) )
			{
				auto min = x->get_bbox_min( ),
					max = x->get_bbox_max( );

				if ( !min.is_valid( ) || !max.is_valid( ) )
					continue;

				renderer->world_to_screen( min, min );
				renderer->world_to_screen( max, max );

				A_::draw_corner_box( min, max, cfg::draw_fake_color->get_color( ), true, -1.f, 2.f );
			}
		}
	}

	auto on_notify_tracker( std::uint32_t hash_name, const char* event_name, global_event_params_script params ) -> void
	{
		if ( !params )
			return;

		auto event_id = params->get_argument( 0 ),
			sender = params->get_argument( 1 ),
			param_3 = params->get_argument( 3 );

		auto name = ( std::string )event_name;
		std::transform( name.begin( ), name.end( ), name.begin( ),
			[ ]( unsigned char c ) { return std::tolower( c ); } );


		auto ent1 = entitylist->get_object_by_network_id( sender ),
			ent2 = entitylist->get_object_by_network_id( param_3 );

		if ( name == "ondie" && ent1 && ent1->is_valid( ) && ent1->is_inhibitor( ) )
		{
			structures::on_inhibitor_death( sender );
		}

#ifdef __TEST
		//myhero->print_chat( 0x3, "%s: %zu | %s | %s:%s", name.c_str( ), sender, std::to_string( param_3 ).c_str( ), ent1 ? ent1->get_name_cstr( ) : "?", ent2 ? ent2->get_name_cstr( ) : "?" );
#endif
	}

#pragma region exp_ranges
	constexpr const auto EXP_RANGE = 1400.f;

	auto is_inside_path( vector position, ClipperLib::Paths solution, std::vector<vector> v_centers )
	{
		auto pos = ClipperLib::IntPoint( static_cast< ClipperLib::cInt >( position.x ), static_cast< ClipperLib::cInt >( position.y ) );
		if ( cfg::strict_inside_check->get_bool( ) )
		{
			for ( auto&& c : v_centers )
			{
				if ( position.distance( c ) > EXP_RANGE )
					return false;
			}

			return true;
		}
		else
		{
			for ( auto&& path : solution )
			{
				if ( ClipperLib::PointInPolygon( pos, path ) )
					return true;
			}

			return false;
		}
	}

	auto draw_exp_ranges( bool ally, int step = 5 ) -> void
	{
		std::vector<vector> v_centers = { };
		ClipperLib::Paths solution;
		ClipperLib::Paths circles;

		for ( auto x : ally ? entitylist->get_enemy_minions( ) : entitylist->get_ally_minions( ) )
		{
			if ( !x || !x->is_valid( ) || !x->is_visible( ) || x->is_dead( ) || x->get_distance( myhero ) > EXP_RANGE * 1.5f )
				continue;

			v_centers.push_back( x->get_position( ) );
		}

		if ( v_centers.empty( ) )
			return;

		for ( auto center : v_centers )
		{
			ClipperLib::Path circle;
			for ( auto i = 0; i < 360; i += step )
			{
				auto x = center.x + ( EXP_RANGE * cos( degrees_to_radians( static_cast< float >( i ) ) ) ),
					y = center.y + ( EXP_RANGE * sin( degrees_to_radians( static_cast< float >( i ) ) ) );

				circle.push_back( ClipperLib::IntPoint( static_cast< ClipperLib::cInt >( x ), static_cast< ClipperLib::cInt >( y ) ) );
			}
			circles.push_back( circle );
		}

		ClipperLib::Clipper c;
		c.AddPaths( circles, ClipperLib::ptSubject, true );
		c.Execute( ClipperLib::ctUnion, solution, ClipperLib::pftPositive, ClipperLib::pftPositive );

		if ( solution.empty( ) )
			return;

		auto b_inside = ally ? is_inside_path( myhero->get_position( ), solution, v_centers ) : false;
		for ( auto&& path : solution )
		{
			for ( auto i = 0; i < path.size( ); i++ )
			{
				auto&& start = path [ i ];
				auto&& end = i == path.size( ) - 1 ? path [ 0 ] : path [ i + 1 ];

				if ( ally )
				{
					auto color = b_inside ? cfg::ally_color_inside->get_color( ) : cfg::ally_color->get_color( );
					draw_manager->add_line( vector( static_cast< float >( start.X ), static_cast< float >( start.Y ) ),
						vector( static_cast< float >( end.X ), static_cast< float >( end.Y ) ), color, static_cast< float >( cfg::thickness_xp->get_int( ) ) );
				}
				else
					draw_manager->add_line( vector( static_cast< float >( start.X ), static_cast< float >( start.Y ) ),
						vector( static_cast< float >( end.X ), static_cast< float >( end.Y ) ), cfg::enemy_color->get_color( ), static_cast< float >( cfg::thickness_xp->get_int( ) ) );
			}
		}
	}
#pragma endregion

	auto draw_monsters_world( ) -> void
	{
		static std::map<int, s_animdata> m_anim_data = { };

		for ( auto i = 0; i < neutral_camp_id::Max_Camps; i++ )
		{
			if ( gametime->get_time( ) >= 20.f * 60.f && i == neutral_camp_id::Herlad )
				continue;

			if ( gametime->get_time( ) <= 15.f * 60.f && i == neutral_camp_id::Baron )
				continue;

			auto it_anim = m_anim_data.find( i );
			if ( it_anim == m_anim_data.end( ) )
			{
				m_anim_data [ i ] = s_animdata { };
				continue;
			}

			auto monster_nids = camp_manager->get_camp_minions( i );
			auto pos = camp_manager->get_camp_position( i );
			renderer->world_to_screen( pos, pos );

			auto icon_size = vector( 40 * renderer->get_dpi_factor( ), 40 * renderer->get_dpi_factor( ) );
			pos.x -= icon_size.x / 2;

			if ( i == neutral_camp_id::Baron && camp_manager->get_camp_alive_status( neutral_camp_id::Herlad ) )
				pos.y -= icon_size.y;

			if ( monster_nids.empty( ) ) //dead
			{
				if ( fow_tracker->draw_3d_jg->get_bool( ) )
				{
					auto respawn_t = camp_manager->get_camp_respawn_time( i ) - gametime->get_time( );
					auto minutes = static_cast< int >( respawn_t / 60 ),
						seconds = static_cast< int >( respawn_t ) % 60;

					if ( respawn_t <= 0.f || respawn_t >= 5.f * 60.f )
						continue;

					auto icon = fow_tracker->get_monster_icon( i );
					if ( icon )
						draw_manager->add_image( icon, pos, icon_size, 90.f, vector::zero, { 1, 1 },
						{
							1.f - 0.77f,
							1.f - 0.77f,
							1.f - 0.77f,
							1 } );

					auto text_size = draw_manager->calc_text_size( static_cast< int >( icon_size.x / 3.f ), "%d:%02d", minutes, seconds );

					auto t_pos = pos + icon_size / 2 - text_size / 2;

					draw_manager->add_text_on_screen( t_pos, utility::COLOR_WHITE, static_cast< int >( icon_size.x / 3.f ), "%d:%02d", minutes, seconds );
					auto pcent = std::clamp( respawn_t / fow_tracker->camp_respawn_t( i ), 0.f, 1.f );
					//myhero->print_chat( 0x3, "%.2f", respawn_t );

					auto v_arc_points = A_::arc_points( pcent, pos + icon_size / 2, icon_size.x / 2 );

					draw_manager->add_circle_on_screen( pos + icon_size / 2, icon_size.x / 2, utility::COLOR_DARK_GRAY, 3.f );
					if ( v_arc_points.size( ) > 2 )
					{
						for ( auto i = 0; i < v_arc_points.size( ) - 1; i++ )
							draw_manager->add_line_on_screen( v_arc_points [ i ], v_arc_points [ i + 1 ], fow_tracker->draw_3d_color_resparning->get_color( ), 3.f );
					}
				}
				continue;
			}

			std::vector<game_object_script> monsters = { };
			for ( auto&& nid : monster_nids )
			{
				auto x = entitylist->get_object_by_network_id( nid );
				if ( !x || !x->is_valid( ) )
					continue;

				monsters.push_back( x );
			}

			auto b_visible = std::find_if( monsters.begin( ), monsters.end( ), [ ]( game_object_script x ) {
				return x->is_visible( );
			} ) != monsters.end( );

			if ( b_visible )
			{
				static auto easing_fn = getEasingFunction( easing_functions::EaseOutSine );

				if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.02f )
				{
					it_anim->second.fl_current_alpha = std::clamp( easing_fn( it_anim->second.fl_current_alpha ), 0.001f, 0.999f );
					it_anim->second.t_last_animation = gametime->get_time( );
				}
			}
			else
			{
				static auto easing_fn = getEasingFunction( easing_functions::EaseInSine );
				if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.02f )
				{
					it_anim->second.fl_current_alpha = std::clamp( easing_fn( it_anim->second.fl_current_alpha ), 0.001f, 0.999f );
					it_anim->second.t_last_animation = gametime->get_time( );
				}
			}

			if ( it_anim->second.fl_current_alpha <= 0.01f )
				continue;

			for ( auto&& x : monsters )
			{
				auto name = x->get_name( );
				std::transform( name.begin( ), name.end( ), name.begin( ),
					[ ]( unsigned char c ) { return std::tolower( c ); } );

				if ( !A_::is_smite_monster( name ) )
					continue;

				fow_tracker->update_monster_icon( i, x );

				if ( !fow_tracker->draw_3d_jg->get_bool( ) )
					continue;

				//pos = x->get_position( );
				//renderer->world_to_screen( pos, pos );

				auto t_attack = fow_tracker->get_last_damage_taken( i );
				auto im_rgba =
					cfg::draw_3d_last_attack->get_bool( ) && (
						( t_attack.has_value( ) && gametime->get_time( ) - t_attack.value( ) <= 10.f ) ||
						( ( i == neutral_camp_id::Dragon || i == neutral_camp_id::Baron || i == neutral_camp_id::Herlad ) && fow_tracker->is_objective_under_attack( i ) )
						) ? 0.33f : 1.f;

				draw_manager->add_image( x->get_square_icon_portrait( ), pos, icon_size, 90.f, vector::zero, { 1, 1 }, { im_rgba, im_rgba, im_rgba, it_anim->second.fl_current_alpha } );
				draw_manager->add_circle_on_screen( pos + icon_size / 2, icon_size.x / 2, MAKE_COLOR( 53, 59, 72, static_cast< int >( 255.f * it_anim->second.fl_current_alpha ) ), 3.f );

				if ( ( i == neutral_camp_id::Dragon || i == neutral_camp_id::Baron || i == neutral_camp_id::Herlad ) && fow_tracker->is_objective_under_attack( i ) )
				{
					auto text_size = draw_manager->calc_text_size( static_cast< int >( icon_size.x / 3.f ), "under attack!" );
					auto t_pos2 = pos;
					t_pos2.x += icon_size.x / 2.f - text_size.x / 2.f;
					t_pos2.y += icon_size.y + 5.f;

					draw_manager->add_text_on_screen( t_pos2, A_::animate_color( fow_tracker->draw_3d_clr_atk->get_color( ), 60.f, 255.f ), static_cast< int >( icon_size.x / 3.f ), "under attack!" );
					draw_manager->add_circle_on_screen( pos + icon_size / 2.f, icon_size.x / 2.f, fow_tracker->draw_3d_clr_atk->get_color( ), 3.f );

					draw_manager->add_circle_on_screen( pos + icon_size / 2.f, fmod( icon_size.x / 2.f * ( gametime->get_time( ) + 1.f ), icon_size.x / 2.f ),
						fow_tracker->draw_3d_clr_anim->get_color( ), 2.f );

					if ( cfg::draw_3d_last_attack->get_bool( ) && t_attack.has_value( ) && gametime->get_time( ) - t_attack.value( ) <= 10.f )
					{
						auto font_sz = static_cast< int >( icon_size.x / 2 );
						auto sz_time = draw_manager->calc_text_size( font_sz, "%.1f", gametime->get_time( ) - t_attack.value( ) );
						draw_manager->add_text_on_screen( pos + icon_size / 2.f - sz_time / 2.f, A_::set_alpha( utility::COLOR_WHITE, static_cast< int >( 255.f * it_anim->second.fl_current_alpha ) ), font_sz, "%.1f", gametime->get_time( ) - t_attack.value( ) );
					}
				}
				else
				{
					if ( t_attack.has_value( ) && gametime->get_time( ) - t_attack.value( ) <= 10.f )
					{
						if ( cfg::_3d_animate_attack->get_bool( ) )
							draw_manager->add_circle_on_screen( pos + icon_size / 2.f, fmod( icon_size.x / 2.f * ( gametime->get_time( ) + 1.f ), icon_size.x / 2.f ),
								fow_tracker->draw_3d_clr_anim->get_color( ), 2.f );

						if ( cfg::draw_3d_last_attack->get_bool( ) )
						{
							auto font_sz = static_cast< int >( icon_size.x / 2 );
							auto sz_time = draw_manager->calc_text_size( font_sz, "%.1f", gametime->get_time( ) - t_attack.value( ) );
							draw_manager->add_text_on_screen( pos + icon_size / 2.f - sz_time / 2.f, A_::set_alpha( utility::COLOR_WHITE, static_cast< int >( 255.f * it_anim->second.fl_current_alpha ) ), font_sz, "%.1f", gametime->get_time( ) - t_attack.value( ) );
						}
					}
				}
			}
		}
	}

	auto draw_player_mia( ) -> void
	{
		if ( !cfg::mia_circles_3d->get_bool( ) )
			return;

		static std::map<uint32_t, s_animdata> m_mia_anim = { };
		static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
		static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

		for ( auto&& x : entitylist->get_enemy_heroes( ) )
		{
			if ( !x || !x->is_valid( ) || x->is_visible( ) )
				continue;

			auto it = m_mia_anim.find( x->get_network_id( ) );
			if ( it == m_mia_anim.end( ) )
			{
				m_mia_anim [ x->get_network_id( ) ] = s_animdata { };
				continue;
			}

			auto data = fow_tracker->get_data( x->get_network_id( ) );
			auto time_in_fow = gametime->get_time( ) - data.t_last_update;

			if ( time_in_fow <= static_cast< float >( cfg::max_draw_time->get_int( ) ) )
			{
				if ( gametime->get_time( ) - it->second.t_last_animation >= 0.02f )
				{
					it->second.fl_current_alpha = std::clamp( easing_fn_in( it->second.fl_current_alpha ), 0.001f, 0.999f );
					it->second.t_last_animation = gametime->get_time( );
				}
			}
			else
			{
				if ( gametime->get_time( ) - it->second.t_last_animation >= 0.02f )
				{
					it->second.fl_current_alpha = std::clamp( easing_fn_out( it->second.fl_current_alpha ), 0.001f, 0.999f );
					it->second.t_last_animation = gametime->get_time( );
				}
			}

			if ( it->second.fl_current_alpha <= 0.01f )
				continue;

			auto distance_traveled = time_in_fow * x->get_move_speed( );
			draw_manager->add_circle( data.position, distance_traveled,
				A_::set_alpha_percent( cfg::mia_circle3d_color->get_color( ), it->second.fl_current_alpha ),
				2.f );
		}
	}

	auto sp_tracker_on_env_draw( )
	{
		if ( !myhero )
			return;

		if ( keyboard_state->is_pressed( keyboard_game::tab ) )
			return;

		structures::show_turret( );
		structures::show_inhibitor( );

		if ( cfg::ally_xp_range->get_bool( ) )
			draw_exp_ranges( true );

		if ( cfg::enemy_xp_range->get_bool( ) )
			draw_exp_ranges( false );

		draw_player_mia( );

		hero_ranges::draw_ranges( true );
	}

#pragma region shared_xp
	struct s_exp_tracker
	{
		float xp = 0.f;
		bool last_visible = false;

		int last_shared_count = 0;
		float t_last_shared_update = -20.f;

		float t_last_shared_notify = -20.f;

		float fl_alpha = 0.001f;
		float t_last_alpha_change = -20.f;
	};
	std::map<uint32_t, s_exp_tracker> m_experience;

	auto get_shared_count( float fl_exp_received_raw, game_object_script src = nullptr ) -> std::optional<int>
	{
		return std::nullopt;
	}

	auto track_shared_xp( game_object_script x ) -> void
	{
		static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
		static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

		static auto is_rift = missioninfo->get_map_id( ) == game_map_id::SummonersRift;
		if ( !is_rift )
			return;

		if ( !x->is_enemy( ) )
			return;

		auto draw_text_count = [ & ]( int num, float fl_alpha ) {
			vector4 v4_bar_pos;
			vector4 v4_hp_bar_pos;
			if ( cfg::xp::shared_xp_text->get_bool( ) && x->get_health_bar_position( v4_bar_pos, v4_hp_bar_pos ) )
			{
				auto bar_width = v4_hp_bar_pos.z,
					bar_height = v4_hp_bar_pos.w;

				auto v_start = vector( v4_hp_bar_pos.x + bar_width / 2.f, v4_hp_bar_pos.y - bar_height * 2.f );
				auto half_line_w = bar_width / 2.f;
				for ( auto i = 0; i <= static_cast< int >( half_line_w ); i++ )
				{
					auto alpha = static_cast< unsigned long >( std::clamp( 255.f - 255.f * ( static_cast< float >( i ) * 1.25f / half_line_w ) * fl_alpha, 0.f, 255.f ) );

					draw_manager->add_line_on_screen( v_start, v_start + vector( static_cast< float >( i ), 0.f ), A_::set_alpha( utility::COLOR_WHITE, alpha ), 1.f ); //C->R
					draw_manager->add_line_on_screen( v_start - vector( static_cast< float >( i ), 0.f ), v_start, A_::set_alpha( utility::COLOR_WHITE, alpha ), 1.f ); //L->C
				}

				auto font_sz = static_cast< int >( bar_height /** 0.75f*/ );
				auto base_text_sz = draw_manager->calc_text_size( font_sz, "Enemies around: " );
				auto count_text_sz = draw_manager->calc_text_size( font_sz, "%d", num );
				base_text_sz.y = count_text_sz.y = std::max( base_text_sz.y, count_text_sz.y );

				auto start_text = v_start - vector( ( base_text_sz.x + count_text_sz.x ) / 2.f, base_text_sz.y + 5.f );
				draw_manager->add_text_on_screen( start_text, A_::set_alpha( utility::COLOR_WHITE, static_cast< unsigned long >( 255.f * fl_alpha ) ), font_sz, "Enemies around: " );

				start_text.x += base_text_sz.x;
				draw_manager->add_text_on_screen( start_text, A_::set_alpha( utility::COLOR_RED, static_cast< unsigned long >( 255.f * fl_alpha ) ), font_sz, "%d", num );
			}
		};

		auto it_exp = m_experience.find( x->get_network_id( ) );
		if ( it_exp == m_experience.end( ) )
		{
			m_experience [ x->get_network_id( ) ] = s_exp_tracker { };
			return;
		}

		if ( x->is_visible( ) && !it_exp->second.last_visible )
		{
			it_exp->second.last_visible = true;
			it_exp->second.xp = x->get_exp( );
		}

		if ( !x->is_visible( ) )
			it_exp->second.last_visible = false;
		else
		{
			if ( it_exp->second.xp != x->get_exp( ) )
			{
				auto xp_diff = std::abs( x->get_exp( ) - it_exp->second.xp );
				auto opt_shared_count = get_shared_count( xp_diff, x );
#if defined( __TEST )
				//myhero->print_chat( 0x3, "gained: %.2f xp | count: %d", xp_diff, opt_shared_count.has_value( ) ? opt_shared_count.value( ) : -1 );
#endif

				if ( opt_shared_count.has_value( ) && x->count_allies_in_range( EXP_RANGE ) < opt_shared_count.value( ) )
				{
#if defined( __TEST )
					//myhero->print_chat( 0x3, "hidden_shared_detected: %d", opt_shared_count.value( ) );
#endif
					if ( A_::obs_safe( ) && gametime->get_time( ) - it_exp->second.t_last_shared_notify >= 0.5f )
					{
						x->add_floating_line( float_text_type_enum::Countdown, "Hidden enemies: %d", opt_shared_count.value( ) );
						it_exp->second.t_last_shared_notify = gametime->get_time( );
					}

					it_exp->second.t_last_shared_update = gametime->get_time( );
					it_exp->second.last_shared_count = opt_shared_count.value( );
				}

				it_exp->second.xp = x->get_exp( );
			}
		}

		if ( x->is_visible( ) )
		{
			auto should_draw = ( x->count_allies_in_range( EXP_RANGE ) < it_exp->second.last_shared_count && gametime->get_time( ) - it_exp->second.t_last_shared_update <= 1.f );

			if ( should_draw )
			{
				if ( gametime->get_time( ) - it_exp->second.t_last_alpha_change >= 0.02f )
				{
					it_exp->second.fl_alpha = std::clamp( easing_fn_in( it_exp->second.fl_alpha ), 0.001f, 0.999f );
					it_exp->second.t_last_alpha_change = gametime->get_time( );
				}
			}
			else
			{
				if ( gametime->get_time( ) - it_exp->second.t_last_alpha_change >= 0.02f )
				{
					it_exp->second.fl_alpha = std::clamp( easing_fn_out( it_exp->second.fl_alpha ), 0.001f, 0.999f );
					it_exp->second.t_last_alpha_change = gametime->get_time( );
				}
			}

			if ( should_draw || it_exp->second.fl_alpha > 0.01f )
				draw_text_count( it_exp->second.last_shared_count, it_exp->second.fl_alpha );
		}
	}
#pragma endregion
	enum e_trackable_buff_type {
		immunity,
		self,
		enchanters
	};

#pragma region buffs
	auto track_buffs( game_object_script x ) -> void
	{
		auto draw_active_buff = [ ]( game_object_script x, float remaining_time, float duration, std::optional< std::pair<bool, uint32_t> > opt_texture_info_pair ) {
			auto max = x->get_bbox_max( ),
				min = x->get_bbox_min( );

			renderer->world_to_screen( max, max );
			renderer->world_to_screen( min, min );

			auto v_position = vector( std::max( max.x, min.x ), std::min( max.y, min.y ) );
			auto v_size = vector( 30.f * renderer->get_dpi_factor( ), 30.f * renderer->get_dpi_factor( ) );

			auto v_arc_points = A_::arc_points( 0.5f, v_position, v_size.x / 8.f, std::nullopt, 180.f + 45.f );
			if ( v_arc_points.empty( ) )
				return;

			for ( auto i = 0; i < v_arc_points.size( ) - 1; i++ )
				draw_manager->add_line_on_screen( v_arc_points [ i ], v_arc_points [ i + 1 ], utility::COLOR_WHITE );

			auto&& v_center_arc = v_arc_points [ static_cast< int >( v_arc_points.size( ) / 2 ) ];
			auto v_end_line_1 = v_center_arc + vector( v_size.x / 2.f, -v_size.y / 2.f );
			auto v_end_line_2 = v_end_line_1 + vector( v_size.x * 2.f, 0.f );

			draw_manager->add_line_on_screen( v_center_arc, v_end_line_1, utility::COLOR_WHITE );
			draw_manager->add_line_on_screen( v_end_line_1, v_end_line_2, utility::COLOR_WHITE );

			if ( opt_texture_info_pair.has_value( ) )
			{
				uint32_t* texture = nullptr;
				auto xy = vector::zero;
				auto zw = vector( 1, 1 );
				if ( !opt_texture_info_pair.value( ).first )
				{
					auto spell = database->get_spell_by_hash( opt_texture_info_pair.value( ).second );
					if ( spell && spell->get_icon_texture_by_index( 0 ) )
						texture = spell->get_icon_texture_by_index( 0 );
				}
				else
				{
					auto item = database->get_item_by_id( static_cast< ItemId >( opt_texture_info_pair.value( ).second ) );
					if ( item && item->get_texture( 0 ).first )
					{
						auto p_texture = item->get_texture( 0 );
						texture = p_texture.first;
						xy = { p_texture.second.x, p_texture.second.y };
						zw = { p_texture.second.z, p_texture.second.w };
					}
				}

				if ( texture )
					draw_manager->add_image( texture, v_end_line_2 - vector( 0.f, v_size.y / 2.f ), v_size, 90.f, xy, zw, { 0.33f, 0.33f, 0.33f, 1.f } );
			}
			draw_manager->add_circle_on_screen( v_end_line_2 + vector( v_size.x / 2.f, 0.f ), v_size.x / 2.f, utility::COLOR_DARK_GRAY, renderer->get_dpi_factor( ) );

			auto fl_remaining_time = std::max( 0.f, remaining_time );
			auto v_arc_percent = A_::arc_points( fl_remaining_time / ( duration ), v_end_line_2 + vector( v_size.x / 2.f, 0.f ), v_size.x / 2.f );
			if ( !v_arc_percent.empty( ) )
			{
				auto color = x->is_ally( ) ? utility::COLOR_LGREEN : utility::COLOR_RED;
				for ( auto i = 0; i < v_arc_percent.size( ) - 1; i++ )
					draw_manager->add_line_on_screen( v_arc_percent [ i ], v_arc_percent [ i + 1 ], color, renderer->get_dpi_factor( ) );
			}

			auto sz_font = static_cast< int >( v_size.x / 2.f );
			auto sz_text = draw_manager->calc_text_size( sz_font, "%.1f", fl_remaining_time );
			draw_manager->add_text_on_screen( v_end_line_2 + vector( v_size.x / 2.f, 0.f ) - sz_text / 2.f, utility::COLOR_WHITE, sz_font, "%.1f", fl_remaining_time );
		};

		static std::map<uint32_t, std::pair<bool, uint32_t>> m_buff_hashes_all = { };
		if ( m_buff_hashes_all.empty( ) )
		{
			m_buff_hashes_all.insert( m_buff_hashes_immunity.begin( ), m_buff_hashes_immunity.end( ) );
			m_buff_hashes_all.insert( m_buff_hashes_self.begin( ), m_buff_hashes_self.end( ) );
			m_buff_hashes_all.insert( m_buff_hashes_enchanters.begin( ), m_buff_hashes_enchanters.end( ) );
		}

		if ( !cfg::buffs::track_buffs->get_bool( ) && x->is_enemy( ) )
			return;

		if ( !cfg::buffs::track_buffs_ally->get_bool( ) && !x->is_enemy( ) )
			return;

		if ( !cfg::buffs::track_buffs_self->get_bool( ) && x->is_me( ) )
			return;

		if ( cfg::buffs::self_enchance->get_bool( ) )
		{
			auto active_spell = x->get_active_spell( );
			if ( active_spell && active_spell->get_spell_data( ) && active_spell->get_spell_data( )->get_name_hash( ) == spell_hash( "NunuR" ) )
			{
				draw_active_buff( x, std::clamp( 3.f - ( gametime->get_time( ) - active_spell->cast_start_time( ) ), 0.f, 3.f ), 3.f,
					std::make_pair( false, active_spell->get_spell_data( )->get_name_hash( ) ) );
				return;
			}
		}

		std::vector<buff_instance_script> m_buffs = { };
		std::vector<buff_instance_script> m_buffs2 = { };
		std::vector<buff_instance_script> m_buffs3 = { };
		for ( auto&& b : x->get_bufflist( ) )
		{
			if ( !b || !b->is_valid( ) )
				continue;

			if ( m_buff_hashes_immunity.find( b->get_hash_name( ) ) != m_buff_hashes_immunity.end( ) && cfg::buffs::immunity->get_bool( ) )
				m_buffs.push_back( b );

			if ( m_buff_hashes_self.find( b->get_hash_name( ) ) != m_buff_hashes_self.end( ) && cfg::buffs::self_enchance->get_bool( ) )
				m_buffs2.push_back( b );

			if ( m_buff_hashes_enchanters.find( b->get_hash_name( ) ) != m_buff_hashes_enchanters.end( ) && cfg::buffs::enchanters->get_bool( ) )
				m_buffs3.push_back( b );
		}

		if ( m_buffs.empty( ) && !m_buffs2.empty( ) )
			m_buffs.insert( m_buffs.begin( ), m_buffs2.begin( ), m_buffs2.end( ) );

		if ( m_buffs.empty( ) && !m_buffs3.empty( ) )
			m_buffs.insert( m_buffs.begin( ), m_buffs3.begin( ), m_buffs3.end( ) );

		if ( m_buffs.empty( ) )
			return;

		std::sort( m_buffs.begin( ), m_buffs.end( ), [ ]( buff_instance_script b1, buff_instance_script b2 ) {
			return b1->get_remaining_time( ) > b2->get_remaining_time( );
		} );

		auto&& buff = m_buffs.front( );
		auto it = m_buff_hashes_all.find( buff->get_hash_name( ) );
		if ( it != m_buff_hashes_all.end( ) )
			draw_active_buff( x, buff->get_remaining_time( ), buff->get_end( ) - buff->get_start( ), 
				std::make_pair( it->second.first, it->second.second ) );
		else
			draw_active_buff( x, buff->get_remaining_time( ), buff->get_end( ) - buff->get_start( ), std::nullopt );
	}
#pragma endregion

	auto sp_trackers_on_draw( ) -> void
	{
		if ( !myhero )
			return;

		if ( keyboard_state->is_pressed( keyboard_game::tab ) )
			return;

#pragma region tp & revive
		static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
		static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

		m_teleports.erase( std::remove_if( m_teleports.begin( ), m_teleports.end( ), [ ]( s_teleport t ) {
			return t.can_delete( );
		} ), m_teleports.end( ) );

		m_revives.erase( std::remove_if( m_revives.begin( ), m_revives.end( ), [ ]( s_revive t ) {
			return t.can_delete( );
		} ), m_revives.end( ) );

		if ( cfg::tp::track_tp->get_bool( ) )
		{
			for ( auto&& tp : m_teleports )
			{
				if ( tp.is_ally && !cfg::tp::track_tp_ally->get_bool( ) )
					continue;

				if ( !tp.is_expired( ) )
				{
					if ( gametime->get_time( ) - tp.t_last_alpha_change >= 0.02f )
					{
						tp.fl_alpha = std::clamp( easing_fn_in( tp.fl_alpha ), 0.001f, 0.999f );
						tp.t_last_alpha_change = gametime->get_time( );
					}
				}
				else
				{
					if ( gametime->get_time( ) - tp.t_last_alpha_change >= 0.02f )
					{
						tp.fl_alpha = std::clamp( easing_fn_out( tp.fl_alpha ), 0.001f, 0.999f );
						tp.t_last_alpha_change = gametime->get_time( );
					}
				}

				auto position = tp.get_position( );
				if ( !position.is_valid( ) )
					continue;

				renderer->world_to_screen( position, position );

				auto sz = vector( 40.f * renderer->get_dpi_factor( ), 40.f * renderer->get_dpi_factor( ) );
				draw_manager->add_image( tp.get_icon( ), position - sz / 2.f, sz, 90.f, { 0, 0 }, { 1, 1 }, { 0.33f, 0.33f, 0.33f, tp.fl_alpha } );
				draw_manager->add_circle_on_screen( position, sz.x / 2.f, A_::set_alpha( utility::COLOR_DARK_GRAY, static_cast< unsigned long >( tp.fl_alpha * 255.f ) ), 2.f * renderer->get_dpi_factor( ) );

				auto font_sz = static_cast< int >( sz.x / 2.f );
				auto text_sz = draw_manager->calc_text_size( font_sz, "%.1f", tp.get_time_till_end( ) );
				draw_manager->add_text_on_screen( position - text_sz / 2.f, A_::set_alpha( utility::COLOR_WHITE, static_cast< unsigned long >( tp.fl_alpha * 255.f ) ), font_sz, "%.1f", tp.get_time_till_end( ) );

				auto v_arc_points = A_::arc_points( std::clamp( 1.f - tp.get_time_till_end( ) / tp.duration, 0.f, 1.f ), position, sz.x / 2.f, 40 );
				if ( !v_arc_points.empty( ) )
				{
					for ( auto i = 0; i < v_arc_points.size( ) - 1; i++ )
						draw_manager->add_line_on_screen( v_arc_points [ i ], v_arc_points [ i + 1 ],
							A_::set_alpha( tp.is_ally ? cfg::tp::color_ally->get_color( ) : cfg::tp::color_enemy->get_color( ),
								static_cast< unsigned long >( A_::get_alpha( tp.is_ally ? cfg::tp::color_ally->get_color( ) : cfg::tp::color_enemy->get_color( ) ) * tp.fl_alpha ) ),
							2.f * renderer->get_dpi_factor( ) );
				}

				if ( tp.get_owner_icon( ) )
				{
					draw_manager->add_image( tp.get_owner_icon( ), position + vector( sz.x / 4.f, -sz.x / 2.f ), sz / 2.f, 90.f, { 0.f, 0.f }, { 1.f, 1.f }, { 1.f, 1.f, 1.f, tp.fl_alpha } );
					draw_manager->add_circle_on_screen( position + vector( sz.x / 4.f, -sz.x / 2.f ) + sz / 4.f, sz.x / 4.f, A_::set_alpha( utility::COLOR_DARK_GRAY, static_cast< int >( 255 * tp.fl_alpha ) ), 2.f );
				}
			}
		}

		if ( cfg::revive::track_revive->get_bool( ) )
		{
			for ( auto&& tp : m_revives )
			{
				if ( !tp.enemy && !cfg::revive::track_revive_ally->get_bool( ) )
					continue;

				if ( !tp.is_expired( ) )
				{
					if ( gametime->get_time( ) - tp.t_last_alpha_change >= 0.02f )
					{
						tp.fl_alpha = std::clamp( easing_fn_in( tp.fl_alpha ), 0.001f, 0.999f );
						tp.t_last_alpha_change = gametime->get_time( );
					}
				}
				else
				{
					if ( gametime->get_time( ) - tp.t_last_alpha_change >= 0.02f )
					{
						tp.fl_alpha = std::clamp( easing_fn_out( tp.fl_alpha ), 0.001f, 0.999f );
						tp.t_last_alpha_change = gametime->get_time( );
					}
				}

				auto position = tp.position;
				if ( !position.is_valid( ) )
					continue;

				renderer->world_to_screen( position, position );

				auto sz = vector( 40.f * renderer->get_dpi_factor( ), 40.f * renderer->get_dpi_factor( ) );
				auto opt_icon = tp.get_icon( );
				if ( opt_icon.has_value( ) )
				{
					draw_manager->add_image( opt_icon.value( ).first, position - sz / 2.f, sz, 90.f,
						{ opt_icon.value( ).second.x, opt_icon.value( ).second.y },
						{ opt_icon.value( ).second.z, opt_icon.value( ).second.w },
						{ 0.33f, 0.33f, 0.33f, tp.fl_alpha } );
				}
				draw_manager->add_circle_on_screen( position, sz.x / 2.f, A_::set_alpha( utility::COLOR_DARK_GRAY, static_cast< unsigned long >( tp.fl_alpha * 255.f ) ), 2.f * renderer->get_dpi_factor( ) );

				auto font_sz = static_cast< int >( sz.x / 2.f );
				auto text_sz = draw_manager->calc_text_size( font_sz, "%.1f", tp.get_time_till_end( ) );
				draw_manager->add_text_on_screen( position - text_sz / 2.f, A_::set_alpha( utility::COLOR_WHITE, static_cast< unsigned long >( tp.fl_alpha * 255.f ) ), font_sz, "%.1f", tp.get_time_till_end( ) );

				auto v_arc_points = A_::arc_points( std::clamp( 1.f - tp.get_time_till_end( ) / tp.duration, 0.f, 1.f ), position, sz.x / 2.f, 40 );
				if ( !v_arc_points.empty( ) )
				{
					for ( auto i = 0; i < v_arc_points.size( ) - 1; i++ )
						draw_manager->add_line_on_screen( v_arc_points [ i ], v_arc_points [ i + 1 ],
							A_::set_alpha( !tp.enemy ? cfg::revive::color_ally->get_color( ) : cfg::revive::color_enemy->get_color( ),
								static_cast< unsigned long >( A_::get_alpha( !tp.enemy ? cfg::revive::color_ally->get_color( ) : cfg::revive::color_enemy->get_color( ) ) * tp.fl_alpha ) ),
							2.f * renderer->get_dpi_factor( ) );
				}

				if ( tp.get_owner_icon( ) )
				{
					draw_manager->add_image( tp.get_owner_icon( ), position + vector( sz.x / 4.f, -sz.x / 2.f ), sz / 2.f, 90.f, { 0.f, 0.f }, { 1.f, 1.f }, { 1.f, 1.f, 1.f, tp.fl_alpha } );
					draw_manager->add_circle_on_screen( position + vector( sz.x / 4.f, -sz.x / 2.f ) + sz / 4.f, sz.x / 4.f, A_::set_alpha( utility::COLOR_DARK_GRAY, static_cast< int >( 255 * tp.fl_alpha ) ), 2.f );
				}
			}
		}
#pragma endregion

		hero_ranges::draw_ranges( false );

		track_zones( cfg::lastseen::zones->get_bool( ), cfg::lastseen::enabled->get_int( ) > 0, cfg::lastseen::priority_list, cfg::lastseen::text_size->get_int( ),
			cfg::lastseen::x, cfg::lastseen::y, cfg::lastseen::lock_position->get_bool( ), cfg::lastseen::hide_visible->get_bool( ) );
		clone_tracker( );
		monster_tracker_draw( );
		g_dash_tracker->on_draw( cfg::spell_casts_ally->get_bool( ), cfg::spell_casts_enemy->get_bool( ),
			static_cast< float >( cfg::spell_casts_duration->get_int( ) ),
			cfg::spell_casts_arc_clr->get_color( ),
			static_cast< float >( cfg::spell_casts_size->get_int( ) ) );
		draw_monsters_map( );
		draw_monsters_world( );

		for ( auto&& x : entitylist->get_all_heroes( ) )
		{
			if ( !x || !x->is_valid( ) || x->is_dead( ) )
				continue;

			track_shared_xp( x );

			auto it_glow = m_glow_champions.find( x->get_network_id( ) );
			if ( it_glow == m_glow_champions.end( ) )
				m_glow_champions [ x->get_network_id( ) ] = false;
			else
			{
				auto should_glow = cfg::glow::enabled->get_bool( ) && gametime->get_time( ) - fow_tracker->get_data( x->get_network_id( ) ).t_last_update <= cfg::glow::time->get_int( ) &&
					!x->is_visible( ) && !x->is_dead( ) && x->is_enemy( );

				if ( should_glow != it_glow->second )
				{
					if ( should_glow )
						glow->apply_glow( x, cfg::glow::color->get_color( ), cfg::glow::th->get_int( ), cfg::glow::blur->get_int( ) );
					else
						glow->remove_glow( x );

					it_glow->second = should_glow;
				}
			}

			draw_tracker( x );

			if ( !x->is_visible( ) )
				continue;

			if ( cfg::aatokill::enabled->get_bool( ) )
				track_aa_lethal( x, cfg::aatokill::position->get_int( ), 
					cfg::aatokill::color_background->get_color( ), cfg::aatokill::color_text->get_int( ), cfg::aatokill::color_text_aa->get_color( ),
					cfg::aatokill::only_if_x_aa->get_bool( ), cfg::aatokill::aa_count->get_int( ) );

			track_buffs( x );

			fow_tracker->update_position( x->get_network_id( ), x->get_position( ) );
			draw_paths( x );
			draw_map( x );
		}

		static auto interval = cfg::apm_tracker_interval->get_int( );
		if ( interval != cfg::apm_tracker_interval->get_int( ) )
		{
			interval = cfg::apm_tracker_interval->get_int( );
			action_map.clear( );
			action_map_avg.clear( );
			peak_action_map.clear( );
		}
	}

	void sp_on_process_spell_cast( game_object_script sender, spell_instance_script spell )
	{
		if ( !sender || !sender->is_valid( ) || !spell )
			return;

		g_dash_tracker->on_process_spell_cast( sender, spell, cfg::color_map );
	}

	auto sp_on_tick( ) -> void
	{
		g_dash_tracker->on_tick( static_cast< float >( cfg::spell_casts_duration->get_int( ) ) );

		if ( myhero->is_dead( ) )
			return;
	}

	auto on_create_object( game_object_script sender ) -> void
	{
		if ( !sender || !sender->is_valid( ) )
			return;

		if ( sender->is_general_particle_emitter( ) )
		{
			//myhero->print_chat( 0x3, "%s", sender->get_name_cstr( ) );
			switch ( sender->get_emitter_resources_hash( ) )
			{
				default:
					break;

				case buff_hash( "Shen_R_tar_Shield" ):
				{
					std::optional<uint32_t> owner = std::nullopt;
					if ( sender->get_emitter( ) && sender->get_emitter( )->is_valid( ) )
						owner = sender->get_emitter( )->get_network_id( );
					m_teleports.push_back( s_teleport( 3.f, sender->get_network_id( ), e_teleport_type::shen, owner, spell_hash( "ZedR" ) ) );
					break;
				}


				case buff_hash( "TwistedFate_R_Gatemarker_Red" ):
				case buff_hash( "TwistedFate_R_Gatemarker_Blue" ):
				case buff_hash( "TwistedFate_R_Gatemarker_Green" ):
				{
					std::optional<uint32_t> owner = std::nullopt;
					if ( sender->get_emitter( ) && sender->get_emitter( )->is_valid( ) )
						owner = sender->get_emitter( )->get_network_id( );
					m_teleports.push_back( s_teleport( 1.5f, sender->get_network_id( ), e_teleport_type::at_particle_end, owner, spell_hash( "Destiny" ) ) );

					break;
				}
			}
		}

		switch ( spell_hash_real( sender->get_name_cstr( ) ) )
		{
			default:
				break;

			case spell_hash( "global_ss_teleport_turret_blue.troy" ):
			case spell_hash( "global_ss_teleport_turret_red.troy" ):
			case spell_hash( "global_ss_teleport_target_blue.troy" ):
			case spell_hash( "global_ss_teleport_target_red.troy" ):
			{
				std::optional<uint32_t> owner = std::nullopt;
				if ( sender->get_emitter( ) && sender->get_emitter( )->is_valid( ) )
					owner = sender->get_emitter( )->get_network_id( );
				m_teleports.push_back( s_teleport( 4.1f, sender->get_network_id( ), e_teleport_type::at_nexus, owner, spell_hash( "SummonerTeleport" ) ) );

				break;
			}
		}
	}

	auto on_gain_lose_buff( game_object_script sender, buff_instance_script buff, bool is_gain ) -> void
	{
		if ( !sender || !sender->is_valid( ) || !sender->is_ai_hero( ) || !sender->is_visible( ) )
			return;

		if ( !buff || !buff->is_valid( ) )
			return;

		auto buff_hash = buff->get_hash_name( );
		switch ( buff_hash )
		{
			case buff_hash( "willrevive" ):
			{
				if ( !is_gain && sender->is_playing_animation( buff_hash( "death" ) ) )
				{
					auto position = sender->get_position( );
					position = position.extend( position == myhero->get_position( ) ? vector::zero : myhero->get_position( ), sender->get_bounding_radius( ) );

					m_revives.push_back( s_revive( position, 4.0f, sender->is_enemy( ), sender->get_network_id( ), std::nullopt, ItemId::Guardian_Angel ) );
				}
				break;
			}
		}
	}

	auto u_on_gain_buff( game_object_script sender, buff_instance_script buff ) -> void
	{
		on_gain_lose_buff( sender, buff, true );
	}

	auto u_on_lose_buff( game_object_script sender, buff_instance_script buff ) -> void
	{
		on_gain_lose_buff( sender, buff, false );
	}

	void load( )
	{
		g_cooldown->load( );
		event_handler<events::on_create_object>::add_callback( on_create_object );
		event_handler< events::on_play_animation >::add_callback( sp_on_play_animation );
		event_handler<events::on_process_spell_cast>::add_callback( sp_on_process_spell_cast );

		event_handler<events::on_buff_gain>::add_callback( u_on_gain_buff );
		event_handler<events::on_buff_lose>::add_callback( u_on_lose_buff );

		event_handler < events::on_update>::add_callback( sp_on_tick );
		event_handler < events::on_new_path>::add_callback( sp_on_new_path );
		event_handler < events::on_process_spell_cast>::add_callback( sp_do_cast );
		event_handler < events::on_stop_cast>::add_callback( sp_stop_cast );

		event_handler<events::on_draw>::add_callback( sp_trackers_on_draw, event_prority::lowest );
		event_handler<events::on_env_draw>::add_callback( sp_tracker_on_env_draw );

		event_handler<events::on_global_event>::add_callback( on_notify_tracker );
	}

	void unload( )
	{
		event_handler<events::on_create_object>::remove_handler( on_create_object );

		event_handler< events::on_play_animation >::remove_handler( sp_on_play_animation );
		event_handler<events::on_process_spell_cast>::remove_handler( sp_on_process_spell_cast );

		event_handler<events::on_buff_gain>::remove_handler( u_on_gain_buff );
		event_handler<events::on_buff_lose>::remove_handler( u_on_lose_buff );

		event_handler < events::on_update>::remove_handler( sp_on_tick );
		event_handler < events::on_new_path>::remove_handler( sp_on_new_path );
		event_handler < events::on_process_spell_cast>::remove_handler( sp_do_cast );
		event_handler < events::on_stop_cast>::remove_handler( sp_stop_cast );

		event_handler<events::on_draw>::remove_handler( sp_trackers_on_draw );
		event_handler<events::on_env_draw>::remove_handler( sp_tracker_on_env_draw );

		event_handler<events::on_global_event>::remove_handler( on_notify_tracker );

		delete cfg::path_tracker_active; delete cfg::spell_tracker_active;
		delete cfg::spell_tracker_casts_active;
		delete g_dash_tracker;

		delete cfg::monster_tracker_active; delete cfg::map_tracker_active;
		delete cfg::tracker_active_clone; 
		delete cfg::xp_ranges_active;
		delete cfg::lastseen::b_lastseen_active;

		delete cfg::b_tp_active;
		delete cfg::buffs::b_active;
		delete cfg::b_mia_circles;

		delete g_cooldown;

		structures::unload( );
	}
}