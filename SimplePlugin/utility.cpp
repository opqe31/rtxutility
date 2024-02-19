#include "../plugin_sdk/plugin_sdk.hpp"
#include "utility.h"
#include "A_.h"
#include "orb.h"
#include "summoner_spell.h"
#include "champion.h"
#include "main_hud.hpp"
#include "level_up.h"
#include "fake_cursor.h"
#include "misc.h"
#include "fow_tracker.h"
#include "spelldb.h"

#include <cstdint>

c_utility* g_utils = new c_utility( );

namespace utility
{
	std::map<champion_id, std::vector<spell_database::spell_data>> spelldb = { };
	std::map< champion_id, std::vector<spell_database::c_activate_spell_data>> spelldb_activate = { };
	void register_trap( game_object_script sender );

	struct s_ward
	{
		vector position = vector::zero;
		float range = 0.f;
		std::vector<vector> points = { };
		geometry::polygon polygon = { };

		float t_create = 0.f;
		float t_life_span = 0.f;
		bool initialized = false;
		bool is_enemy = false;

		std::pair<std::uint32_t*, vector4> icon = { nullptr, { 0, 0, 0, 0 } };
		uint32_t network_id_owner = 0;
		uint32_t network_id = 0;

		float fl_alpha = 0.001f;
		float t_last_anim = -20.f;

		float fl_alpha_i = 0.001f;
		float t_last_anim_i = -20.f;

		uint32_t model_hash;

		auto get_owner_icon( ) -> uint32_t*
		{
			if ( !network_id_owner )
				return nullptr;

			auto owner = entitylist->get_object_by_network_id( network_id_owner );

			return owner && owner->is_valid( ) ? owner->get_square_icon_portrait( ) : nullptr;
		}

		auto get_remaining_time( ) -> std::optional<float>
		{
			if ( t_create > 0.f && t_life_span > 0.f )
				return std::max( 0.f, t_create + t_life_span - gametime->get_time( ) );

			return std::nullopt;
		}

		auto get_remaining_pcent( ) -> std::optional<float>
		{
			if ( t_create > 0.f && t_life_span > 0.f )
				return std::clamp( ( t_create + t_life_span - gametime->get_time( ) ) / t_life_span, 0.05f, 1.f );

			return std::nullopt;
		}

		auto is_expired( ) -> bool
		{
			if ( t_create > 0.f && t_create + t_life_span < gametime->get_time( ) )
				return true;
			else if ( network_id != 0 )
			{
				auto obj = entitylist->get_object_by_network_id( network_id );
				return !obj || !obj->is_valid( ) || obj->is_dead( ) || obj->get_health( ) < 1;
			}
			return false;
		}
	};
	std::vector< std::pair<uint32_t, s_ward> > m_wards = { };
	std::vector< s_ward > m_traps = { };

	struct s_zhonya_map {
		bool is_ally;
		TreeTab* tab;
	};
	std::vector<s_zhonya_map> m_zhonya_tabs = { };
	struct s_evade_config
	{
		TreeEntry* enabled;
		TreeEntry* enabled_hp;

		TreeEntry* allies;
		TreeEntry* enemies;
	};

	namespace traps
	{
		TreeEntry* glow_1; TreeEntry* glow_2; TreeEntry* glow_3; TreeEntry* glow_4;

		TreeEntry* glow_th; TreeEntry* glow_bl;
	}

	namespace autoping
	{
		TreeEntry* wards;
		TreeEntry* delay_1; TreeEntry* delay_2;
		TreeEntry* randomize_pos;
		TreeEntry* only_on_screen;

		TreeEntry* enemies;
		TreeEntry* only_jungler;
	}

	struct s_trap_info 
	{
		float range;
		std::string name;
		std::optional<uint32_t> spell_hash;
		std::optional<uint32_t> default_color;
		std::optional<uint32_t> default_color2;
		std::optional<uint32_t> default_color_glow;

		auto get_config_key( ) -> std::string
		{
			auto result = name;
			std::transform( result.begin( ), result.end( ), result.begin( ),
				[ ]( unsigned char c ) -> unsigned char {
				if ( std::isalpha( c ) && std::isupper( c ) ) {
					return std::tolower( c );
				}
				return c;
			}
			);

			std::transform( result.begin( ), result.end( ), result.begin( ),
				[ ]( unsigned char c ) -> unsigned char {
				if ( c == ' ' ) {
					return '_';
				}
				return c;
			}
			);

			return result;
		}

		s_trap_info( float fl_range, std::string str_name, std::optional<uint32_t> opt_spell_hash = std::nullopt, 
			std::optional<uint32_t> opt_clr1 = std::nullopt, 
			std::optional<uint32_t> opt_clr1_2 = std::nullopt,
			std::optional<uint32_t> opt_clr2 = std::nullopt )
		{
			range = fl_range;
			name = str_name;
			spell_hash = opt_spell_hash;

			default_color = opt_clr1;
			default_color2 = opt_clr1_2;
			default_color_glow = opt_clr2;
		}
	};

	struct s_trap_menu
	{
		TreeTab* tab;

		TreeEntry* enabled_circle;
		TreeEntry* circle_color;
		TreeEntry* circle_color2;

		TreeEntry* enabled_glow;
		TreeEntry* glow_color;
	};

	const std::unordered_map<uint32_t, s_trap_info> m_trap_info =
	{
		{ buff_hash( "TeemoMushroom" ), s_trap_info( 150.f, "Teemo R", spell_hash( "TeemoRCast" ), COLOR_LGREEN, COLOR_GRAY ) },
		{ buff_hash( "CaitlynTrap" ), s_trap_info( 75.f, "Caitlyn W", spell_hash( "CaitlynW" ), COLOR_RED, COLOR_DARK_GRAY ) },
		{ buff_hash( "ShacoBox" ), s_trap_info( 300.f, "Shaco W", spell_hash( "JackInTheBox" ), COLOR_CORAL, COLOR_DARK_GRAY ) },
		{ buff_hash( "JhinTrap" ), s_trap_info( 260.f, "Jhin E", spell_hash( "JhinE" ), COLOR_CORAL, COLOR_GRAY ) },
		{ buff_hash( "NidaleeSpear" ), s_trap_info( 75.f, "Nidalee W trap", spell_hash( "Bushwhack" ), COLOR_LGREEN, COLOR_GRAY ) },
		{ buff_hash( "MaokaiSproutling" ), s_trap_info( 350.f, "Maokai E", spell_hash( "MaokaiE" ), COLOR_GRAY, COLOR_DARK_GRAY ) }
	};
	std::map<uint32_t, s_trap_menu> m_trap_menu = { };

	std::map<champion_id, std::vector<std::pair<std::string, s_evade_config> > > z_evade_whitelist;

	struct evade_queue_entry
	{
		std::string caster_name;
		std::string spell_name;
		s_evade_config priority_entry;

		bool is_special_attack;
		bool is_activate_spell;

		uint32_t buff_hash;

		float time;
		spellslot evade_method = spellslot::invalid;
		uint32_t missile_network_id = 0;
	};
	std::vector< evade_queue_entry > v_evade_queue;

	script_spell* q = nullptr;
	script_spell* w = nullptr;
	script_spell* e = nullptr;
	script_spell* r = nullptr;
	//召唤师技能

	//菜单
	TreeTab* main_tab = nullptr;

	namespace item
	{
		TreeEntry* ghostblade;
		TreeEntry* chase_gb;
		TreeEntry* _logic_1_gb;
		TreeEntry* hp_high_gb;
		TreeEntry* hp_low_gb;
		TreeEntry* mouse_dis_gb;
		TreeEntry* enemy_dis_gb;
		std::map < champion_id, TreeEntry*> use_to_gb;

		TreeEntry* flee_gb;
		TreeEntry* hp_low_flee_gb;

		TreeEntry* galeforce;
		TreeEntry* galeforce_aa;
		TreeEntry* galeforce_max_enemies;
		TreeEntry* galeforce_dive_hp;
		std::map<champion_id, TreeEntry*> galeforce_whitelist;

		TreeEntry* ironspike = nullptr;
		TreeEntry* ironspike_combo = nullptr;
		TreeEntry* ironspike_harass = nullptr;
		TreeEntry* ironspike_lane_clear = nullptr;
		TreeEntry* ironspike_lane_clear_reset_aa = nullptr;
		TreeEntry* ironspike_jungle_clear = nullptr;
		TreeEntry* ironspike_jungle_clear_reset_aa = nullptr;
		TreeEntry* ironspike_fast_lane_clear = nullptr;

		TreeEntry* goredrinker = nullptr;
		TreeEntry* goredrinker_combo_count_hero = nullptr;
		TreeEntry* goredrinker_combo_hp = nullptr;
		TreeEntry* goredrinker_harass = nullptr;
		TreeEntry* goredrinker_lane_clear = nullptr;
		TreeEntry* goredrinker_lane_clear_reset_aa = nullptr;
		TreeEntry* goredrinker_jungle_clear = nullptr;
		TreeEntry* goredrinker_jungle_clear_reset_aa = nullptr;
		TreeEntry* goredrinker_fast_lane_clear = nullptr;
		TreeEntry* goredrinker_auto_use = nullptr;

		TreeEntry* stridebreaker = nullptr;
		TreeEntry* stridebreaker_combo_count_hero = nullptr;
		TreeEntry* stridebreaker_harass = nullptr;
		TreeEntry* stridebreaker_flee = nullptr;
		TreeEntry* stridebreaker_lane_clear = nullptr;
		TreeEntry* stridebreaker_lane_clear_reset_aa = nullptr;
		TreeEntry* stridebreaker_jungle_clear = nullptr;
		TreeEntry* stridebreaker_jungle_clear_reset_aa = nullptr;
		TreeEntry* stridebreaker_fast_lane_clear = nullptr;
		std::map<std::uint32_t, TreeEntry*> stridebreaker_always_use_on;

		namespace locket
		{
			TreeEntry* enabled;
			TreeEntry* health;
			std::map < champion_id, TreeEntry* > wl;
		}

		TreeEntry* randuins = nullptr;
		std::map<std::uint32_t, TreeEntry*> randuins_always_use_on;

		TreeEntry* gargoyle_stoneplate = nullptr;
		TreeEntry* gargoyle_stoneplate_hp = nullptr;
		TreeEntry* gargoyle_stoneplate_damage = nullptr;
		TreeEntry* gargoyle_stoneplate_damage_hp = nullptr;

		TreeEntry* use_heartsteel = nullptr;
		TreeEntry* draw_heartsteel = nullptr;

		TreeEntry* auto_ward_mode;
		TreeEntry* auto_ward_control;
		TreeEntry* auto_ward_stealth;
		TreeEntry* semi_auto_ward_key;
		TreeEntry* draw_auto_ward_control;
		TreeEntry* draw_auto_ward_stealth;

		TreeEntry* auto_ward_bush;
		TreeEntry* auto_ward_bush_ctrl;
		TreeEntry* auto_ward_bush_combo;

		TreeEntry* draw_ward = nullptr;
		TreeEntry* draw_ward_minimap;
		TreeEntry* draw_ward_sz;
		TreeEntry* draw_ward_remaining_t_color;
		TreeEntry* draw_ward_ally;
		TreeEntry* draw_ward_time;
		TreeEntry* draw_ward_time_ally;
		//TreeEntry* draw_ward_time_map;
		TreeEntry* ward_cast_assist = nullptr;
		TreeEntry* sup_buy_ward = nullptr;
		TreeEntry* buy_ward = nullptr;
		TreeEntry* buy_ward_lv = nullptr;

		TreeEntry* zhonya;
		TreeEntry* min_health;
		TreeEntry* min_enemies;
		TreeEntry* min_allies;

		TreeEntry* zhonya_spells;
		TreeEntry* zhonya_buffer;
		TreeEntry* evade_force;
		TreeEntry* zhonya_wl_allies;

		TreeEntry* evade_towershot;
		TreeEntry* evade_towershot_hp;

		bool* b_gunblade;
		TreeEntry* gunblade;
		TreeEntry* semi_gunblade;
		TreeEntry* flee_gunblade;

		TreeEntry* everfrost;
		TreeEntry* hitchance_everfrost;
		TreeEntry* everfrost_range;
		TreeEntry* everfrost_combo;
		TreeEntry* everfrost_harass;
		TreeEntry* everfrost_flee;
		TreeEntry* everfrost_farm;

		/*
		TreeEntry* auto_ward_yellow;
		TreeEntry* auto_ward_blue;
		TreeEntry* auto_ward_control;*/

		//TreeEntry* pyke_spam;
	}
	namespace potions
	{
		TreeEntry* Health_Potion;
		TreeEntry* Health_Potion_hp;
		TreeEntry* Health_Potion_c;
		TreeEntry* Health_Potion_harass;
		TreeEntry* Health_Potion_other;
		TreeEntry* Health_Potion_rang;
		TreeEntry* Health_Potion_enemy;

		TreeEntry* Refillable_Potion;
		TreeEntry* Refillable_Potion_hp;
		TreeEntry* Refillable_Potion_c;
		TreeEntry* Refillable_Potion_harass;
		TreeEntry* Refillable_Potion_other;
		TreeEntry* Refillable_Potion_rang;
		TreeEntry* Refillable_Potion_enemy;

		TreeEntry* Corrupting_Potion;
		TreeEntry* Corrupting_Potion_hp;
		TreeEntry* Corrupting_Potion_c;
		TreeEntry* Corrupting_Potion_harass;
		TreeEntry* Corrupting_Potion_other;
		TreeEntry* Corrupting_Potion_rang;
		TreeEntry* Corrupting_Potion_enemy;

		TreeEntry* Biscuit;
		TreeEntry* Biscuit_hp;
		TreeEntry* Biscuit_c;
		TreeEntry* Biscuit_harass;
		TreeEntry* Biscuit_other;
		TreeEntry* Biscuit_rang;
		TreeEntry* Biscuit_enemy;
	}
	TreeEntry* fake_ping_value;

	void on_update( );
	void on_update_highest( );
	void on_cast_spell( spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process );
	void on_new_draw( );
	void on_draw( );
	void on_issue_order( game_object_script& target, vector& pos, _issue_order_type& type, bool* process );
	void on_after_attack_orbwalker( game_object_script target );
	void on_process_spell_cast( game_object_script sender, spell_instance_script spell );
	void on_create_object( game_object_script sender );
	void on_delete_object( game_object_script sender );
	void new_path( game_object_script sender, const std::vector<vector>& path, bool is_dash, float dash_speed );

	class Item {
	public:
		Item( ItemId id, float range )
		{
			_range = range;
			_id = { id, static_cast< ItemId >( static_cast< int >( id ) + 220000 ) };
		}
		Item( std::vector<ItemId> ids, float range )  
		{	
			_range = range;
			for ( auto id : ids )
			{
				_id.push_back( id );
				_id.push_back( static_cast<ItemId>( static_cast<int>( id ) + 220000 ) );
			}
		}
		bool is_vaild( )
		{
			return myhero->has_item( _id ) != spellslot::invalid;
		}

		bool is_ready( )
		{
			//auto ready_item = false;
			for ( auto&& x : _id )
			{
				if ( myhero->is_item_ready( x ) )
					return true;
			}

			return false;
		}
		void cast( game_object_script target )
		{
			if ( is_ready( ) )
			{
				auto slot = myhero->has_item( _id );
				if ( slot != spellslot::invalid && target->get_distance( myhero ) < _range - 10.f )
					myhero->cast_spell( slot, target );
			}
		}
		void cast( ) {
			if ( is_ready( ) ) {
				auto slot = myhero->has_item( _id );
				if ( slot != spellslot::invalid )
					myhero->cast_spell( slot );
			}
		}
		void cast( vector vec ) {
			if ( is_ready( ) ) {
				auto slot = myhero->has_item( _id );
				if ( slot != spellslot::invalid )
					myhero->cast_spell( slot, vec );
			}
		}

		std::vector<ItemId> _id;
		float _range;
	};

	//石像鬼
	float gargoyle_stoneplate_shield;

	auto add_targeted_spell_entry( std::string charname, TreeTab* whitelist_evade, game_object_script hero ) -> void
	{
		whitelist_evade->add_separator( charname + "_qevade_whitelist_tab_" + std::to_string( static_cast<int>( hero->get_champion( ) ) ), hero->get_model( ) );

		std::vector<std::pair<std::string, s_evade_config> > spell_list_hero = { };

		auto tab_attack = whitelist_evade->add_tab( charname + "_evade_atk_" + std::to_string( static_cast< int >( hero->get_champion( ) ) ), "Special attack" );
		auto s_attack = s_evade_config( );

		s_attack.enabled = tab_attack->add_checkbox( "special_attack", "Enabled", false );
		s_attack.enabled_hp = tab_attack->add_slider( "enabled_hp", "^Only if hp <= {x}%", 101, 0, 101 );
		s_attack.enemies = tab_attack->add_slider( "min_enemies", "^Only if enemies around >= {x}", 0, 0, 3 );
		s_attack.allies = tab_attack->add_slider( "min_allies", "^Only if allies around >= {x}", 0, 0, 3 );

		tab_attack->set_assigned_active( s_attack.enabled );
		tab_attack->set_tooltip( "Empowered attack like BlitzE" );
		s_attack.enabled_hp->set_tooltip( "101 = always" );

		switch ( hero->get_champion( ) )
		{
			default:
				break;
			case champion_id::Leona:
				tab_attack->set_texture( hero->get_spell( spellslot::q )->get_icon_texture( ) );
				break;
			case champion_id::Blitzcrank:
				tab_attack->set_texture( hero->get_spell( spellslot::e )->get_icon_texture( ) );
				break;
		}

		spell_list_hero.push_back( { "special_attack", s_attack } );

		//add targeted spells
		if ( spelldb.find( hero->get_champion( ) ) != spelldb.end( ) )
		{
			auto spelldata_hero = spelldb [ hero->get_champion( ) ];
			for ( auto spell : spelldata_hero )
			{
				if ( spell.targeted )
				{
					auto tab_spell = whitelist_evade->add_tab( charname + "_r_evade_" + spell.spell_names.front( ) + "_" + spell.menuslot + "_" + hero->get_model( ), spell.menuslot );
					tab_spell->set_texture( hero->get_spell( spell.slot )->get_icon_texture( ) );
					auto s_spell = s_evade_config( );

					s_spell.enabled = tab_spell->add_checkbox( "special_attack", "Enabled", !spell.off_by_default );
					s_spell.enabled_hp = tab_spell->add_slider( "enabled_hp", "^Only if hp <= {x}%", 101, 0, 101 );
					s_spell.enemies = tab_spell->add_slider( "min_enemies", "^Only if enemies around >= {x}", 0, 0, 3 );
					s_spell.allies = tab_spell->add_slider( "min_allies", "^Only if allies around >= {x}", 0, 0, 3 );

					tab_spell->set_assigned_active( s_spell.enabled );
					s_spell.enabled_hp->set_tooltip( "101 = always" );

					spell_list_hero.push_back( { spell.spell_names.front( ), s_spell } );
				}
			}
		}

		//add activate spells
		if ( spelldb_activate.find( hero->get_champion( ) ) != spelldb_activate.end( ) )
		{
			auto spelldata_hero = spelldb_activate [ hero->get_champion( ) ];
			for ( auto spell : spelldata_hero )
			{
				auto tab_spell = whitelist_evade->add_tab( charname + "_r_evade_" + spell.spell_names.front( ) + "_" + spell.menuslot + "_" + hero->get_model( ), spell.menuslot );
				tab_spell->set_texture( hero->get_spell( spell.slot )->get_icon_texture( ) );
				auto s_spell = s_evade_config( );

				s_spell.enabled = tab_spell->add_checkbox( "special_attack", "Enabled", !spell.menu_disabled );
				s_spell.enabled_hp = tab_spell->add_slider( "enabled_hp", "^Only if hp <= {x}%", 101, 0, 101 );
				s_spell.enemies = tab_spell->add_slider( "min_enemies", "^Only if enemies around >= {x}", 0, 0, 3 );
				s_spell.allies = tab_spell->add_slider( "min_allies", "^Only if allies around >= {x}", 0, 0, 3 );

				tab_spell->set_assigned_active( s_spell.enabled );
				s_spell.enabled_hp->set_tooltip( "101 = always" );

				spell_list_hero.push_back( { spell.spell_names.front( ), s_spell } );
			}
		}
		//add targeted spells
		z_evade_whitelist [ hero->get_champion( ) ] = spell_list_hero;
	}

	auto apply_glow_trap( game_object_script obj, bool cb = false ) -> void
	{
		auto it_menu = m_trap_menu.find( buff_hash_real( obj->get_model_cstr( ) ) );
		if ( it_menu == m_trap_menu.end( ) )
			return;

		if ( !it_menu->second.enabled_glow->get_bool( ) )
		{
			if ( glow->is_glowed( obj ) && !cb )
				glow->remove_glow( obj );
		}
		else
			glow->apply_glow( obj, it_menu->second.glow_color->get_color( ), traps::glow_th->get_int( ), traps::glow_bl->get_int( ) );
	}

	auto menu_update_glow( ) -> void
	{
		for ( auto&& trap : m_traps )
		{
			if ( trap.is_expired( ) )
				continue;

			auto obj = entitylist->get_object_by_network_id( trap.network_id );
			if ( !obj || !obj->is_valid( ) )
				continue;

			apply_glow_trap( obj );
		}
	}

	void load( )
	{
		spelldb = spell_database::load( );
		spelldb_activate = spell_database::load_activate( );

		g_wards->load_trick_wards( );

		A_trans::loadTranslation( );

		orbwalker->set_movement( true );
		orbwalker->set_attack( true );

		auto icon = draw_manager->load_texture_from_file( L"_utils/main_icon.png" );

		q = plugin_sdk->register_spell( spellslot::q, 0 );
		w = plugin_sdk->register_spell( spellslot::w, 0 );
		e = plugin_sdk->register_spell( spellslot::e, 0 );
		r = plugin_sdk->register_spell( spellslot::r, 0 );

		q->set_spell_lock( false );
		w->set_spell_lock( false );
		e->set_spell_lock( false );
		r->set_spell_lock( false );

#if defined ( __TEST )
		main_tab = menu->create_tab( "utility", ": Utils [d143]" );
#elif defined( _VBETA )
		main_tab = menu->create_tab( "utility", ": Utils [b]" );
#else
		main_tab = menu->create_tab( "utility", ": Utils" );
#endif

		if ( icon && icon->texture )
			main_tab->set_texture( icon->texture );

		{
			//main_tab->adjust_height_to_main_tab() = true;

			main_tab->add_separator( "_utility", " - - : Utils - - " );

			summoner_spell::cleanse_menu( );

			auto summoner = main_tab->add_tab( "summoner", "Summoner spells" );
			{
				summoner->add_separator( "summoner_spell", " - - Summoner spell - - " );

				summoner_spell::all_menu( );
			}

			spell_tracker::create_menu( );

			auto item = main_tab->add_tab( "item", "Items" );
			{
				item->add_separator( "offensive", " - - Offensive - - " );
				{
					auto hextech_gunblade = item->add_tab( "hextech_gunblade", "Hextech gunblade" );
					{
						auto icon = database->get_item_by_id( ItemId::Hextech_Gunblade )->get_texture( );
						if ( icon.first )
						{
							auto e_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
							hextech_gunblade->set_assigned_texture( e_icon );
						}

						hextech_gunblade->add_separator( "gb_sep", " - - Hextech gunblade - - " );
						item::gunblade = hextech_gunblade->add_checkbox( "use_gb", "Use if lethal", true );
						item::semi_gunblade = hextech_gunblade->add_hotkey( "semi_gunblade", "Semi-manual cast", TreeHotkeyMode::Hold, 0x00, false );
						item::flee_gunblade = hextech_gunblade->add_checkbox( "use_gb_flee", "Use in flee", true );

						item::b_gunblade = new bool( false );
						hextech_gunblade->set_assigned_active( item::b_gunblade );

						auto on_gb_change = [ ]( TreeEntry* ) {
							*item::b_gunblade = item::gunblade->get_bool( ) || item::flee_gunblade->get_bool( ) || item::semi_gunblade->get_int( ) != 0;
						};
						item::gunblade->add_property_change_callback( on_gb_change );
						item::semi_gunblade->add_property_change_callback( on_gb_change );
						item::flee_gunblade->add_property_change_callback( on_gb_change );
						on_gb_change( 0 );
					}

					auto everfrost = item->add_tab( "everfrost", "Everfrost" );
					{
						auto icon = database->get_item_by_id( ItemId::Everfrost )->get_texture( );
						auto e_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						everfrost->set_assigned_texture( e_icon );

						everfrost->add_separator( "everfrost_sep", " - - Everfrost - - " );
						item::everfrost = everfrost->add_checkbox( "use_everfrost", "Everfrost", true );
						item::everfrost_range = everfrost->add_slider( "everfrost_range", "Range", 80, 60, 100 );
						item::hitchance_everfrost = everfrost->add_combobox( "hitchance_everfrost", "Everfrost hitchance",
							{ { "low", nullptr }, { "medium", nullptr }, { "high", nullptr }, { "very_high", nullptr }, { "dashing", nullptr } }, 2 ); //+3

						everfrost->add_separator( "everfrost_sep1", " - - Modes - - " );
						item::everfrost_combo = everfrost->add_checkbox( "everfrost_combo", "Combo", true );
						item::everfrost_harass = everfrost->add_checkbox( "everfrost_harass", "Harass", false );
						item::everfrost_flee = everfrost->add_checkbox( "everfrost_flee", "Flee", true );
						item::everfrost_farm = everfrost->add_slider( "everfrost_fastlaneclear", "Fastlaneclear minions", 0, 0, 5 );

						everfrost->set_assigned_active( item::everfrost );
					}

					auto ghostblade = item->add_tab( "ghostblade", "Yoomu's Ghostblade" );
					{
						auto icon = database->get_item_by_id( ItemId::Youmuus_Ghostblade )->get_texture( );
						auto gb_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						ghostblade->set_assigned_texture( gb_icon );

						ghostblade->add_separator( "yoomus_gb", " - - Yoomu's Ghostblade - - " );

						item::ghostblade = ghostblade->add_checkbox( "use", "Yoomu's Ghostblade", true );
						ghostblade->set_assigned_active( item::ghostblade );

						ghostblade->add_separator( "chase", " - - Chase logic - - " );
						item::chase_gb = ghostblade->add_checkbox( "chase_ghostblade", "Chase", true, true );
						item::_logic_1_gb = ghostblade->add_checkbox( "_logic1", "  ^-Avoid wasting logic (?)", true, true );
						item::_logic_1_gb->set_tooltip( "Avoiding wasting Yoomu's Ghostblade" );
						item::hp_high_gb = ghostblade->add_slider( "hp_high", "  ^-Enemy HP >= x%", 10, 10, 100 );
						item::hp_low_gb = ghostblade->add_slider( "hp_low", "  ^-Enemy HP <= x%", 40, 10, 100 );
						item::mouse_dis_gb = ghostblade->add_slider( "mouse_dis", "  ^-Mouse distance <= x%", 300, 100, 500 );
						item::enemy_dis_gb = ghostblade->add_slider( "enemy_dis", "  ^-Enemy distance <= AA range + x", 500, 300, 600 );

						auto chase_to = ghostblade->add_tab( "chase_to", "Chase whitelist" );
						{
							chase_to->add_separator( "gb0", " - - Chase - - " );
							for ( auto&& ally : entitylist->get_enemy_heroes( ) )
							{
								item::use_to_gb [ ally->get_champion( ) ] = chase_to->add_checkbox( "use_to" + std::to_string( ( int )ally->get_champion( ) ), ally->get_model( ), true, true );
								item::use_to_gb [ ally->get_champion( ) ]->set_texture( ally->get_square_icon_portrait( ) );
							}
						}

						ghostblade->add_separator( "gb4", " - - Flee logic - - " );
						item::flee_gb = ghostblade->add_checkbox( "flee", "Flee", true, true );
						item::hp_low_flee_gb = ghostblade->add_slider( "hp_low_flee", "  ^-HP <= x%", 33, 10, 80 );
					}

					auto galeforce = item->add_tab( "galeforce", "Galeforce" );
					{
						auto icon = database->get_item_by_id( ItemId::Galeforce )->get_texture( );
						auto galeforce_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						galeforce->set_assigned_texture( galeforce_icon );

						galeforce->add_separator( "galefprce_sep", " - - Galeforce - - " );
						item::galeforce = galeforce->add_checkbox( "use_galeforce", "Galeforce", true );
						galeforce->set_assigned_active( item::galeforce );
						item::galeforce_aa = galeforce->add_slider( "galeforce_aa", "Add x AA", 1, 0, 3 );
						item::galeforce_max_enemies = galeforce->add_slider( "galeforce_max_enemies", "Max enemies around", 2, 1, 6 );
						item::galeforce_dive_hp = galeforce->add_slider( "galeforce_dive_hp", "Disable into turret if hp <= {x} %", 75, 0, 101 );

						galeforce->add_separator( "galefprce_wl_sep", " - - Use on - - " );
						for ( auto&& x : entitylist->get_enemy_heroes( ) )
						{
							item::galeforce_whitelist [ x->get_champion( ) ] = galeforce->add_checkbox( "use_galeforce" + std::to_string( ( int )x->get_champion( ) ), x->get_model( ), true );
							item::galeforce_whitelist [ x->get_champion( ) ]->set_texture( x->get_square_icon_portrait( ) );
						}
					}

					auto ironspike = item->add_tab( "ironspike", "Ironspike" );
					{
						auto icon = database->get_item_by_id( ItemId::Ironspike_Whip )->get_texture( );
						auto ironspike_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						ironspike->set_assigned_texture( ironspike_icon );

						ironspike->add_separator( "ironspike", " - - Ironspike - - " );
						item::ironspike = ironspike->add_checkbox( "use_ironspike", "Ironspike", true );
						ironspike->set_assigned_active( item::ironspike );
						ironspike->add_separator( "ironspike_1", " - - Combo - - " );
						item::ironspike_combo = ironspike->add_checkbox( "ironspike_combo", "Use ironspike combo", true );
						ironspike->add_separator( "ironspike_5", " - - Harass - - " );
						item::ironspike_harass = ironspike->add_checkbox( "ironspike_harass", "Use ironspike harass", true );
						ironspike->add_separator( "ironspike_2", " - - LaneClear - - " );
						item::ironspike_lane_clear = ironspike->add_slider( "ironspike_lane_clear", "Use ironspike >= x minion", 7, 1, 10 );
						item::ironspike_lane_clear_reset_aa = ironspike->add_slider( "ironspike_lane_clear_reset_aa", "Use ironspike >= x minion (after aa)", 5, 1, 10 );
						item::ironspike_jungle_clear = ironspike->add_slider( "ironspike_jungle_clear", "Use ironspike >= x monster", 2, 1, 10 );
						item::ironspike_jungle_clear_reset_aa = ironspike->add_slider( "ironspike_jungle_clear_reset_aa", "Use ironspike >= x monster (after aa)", 1, 1, 10 );
						ironspike->add_separator( "ironspike_3", " - - Fast LaneClear - - " );
						item::ironspike_fast_lane_clear = ironspike->add_checkbox( "ironspike_fast_lane_clear", "Use ironspike fast laneclear", true );
					}

					auto stridebreaker = item->add_tab( "stridebreaker", "Stridebreaker" );
					{
						auto stridebreakerr = database->get_item_by_id( ItemId::Stridebreaker )->get_texture( );
						auto stridebreaker_icon = create_texture_descriptor( stridebreakerr.first, { stridebreakerr.second.x, stridebreakerr.second.y, stridebreakerr.second.z, stridebreakerr.second.w } );
						stridebreaker->set_assigned_texture( stridebreaker_icon );

						stridebreaker->add_separator( "stridebreaker", " - - Stridebreaker - - " );
						item::stridebreaker = stridebreaker->add_checkbox( "use_stridebreaker", "Stridebreaker", true );
						stridebreaker->set_assigned_active( item::stridebreaker );
						stridebreaker->add_separator( "stridebreaker_1", " - - Combo - - " );
						item::stridebreaker_combo_count_hero = stridebreaker->add_slider( "stridebreaker_combo_count_hero", "Use stridebreaker >= x hero", 2, 1, 6 );
						stridebreaker->add_separator( "stridebreaker_5", " - - Harass - - " );
						item::stridebreaker_harass = stridebreaker->add_checkbox( "stridebreaker_harass", "Use stridebreaker harass", true );
						stridebreaker->add_separator( "stridebreaker_f", " - - Flee - - " );
						item::stridebreaker_flee = stridebreaker->add_checkbox( "stridebreaker_flee", "Use stridebreaker flee", true );
						stridebreaker->add_separator( "stridebreaker_2", " - - LaneClear - - " );
						item::stridebreaker_lane_clear = stridebreaker->add_slider( "stridebreaker_lane_clear", "Use stridebreaker >= x minion", 7, 1, 10 );
						item::stridebreaker_lane_clear_reset_aa = stridebreaker->add_slider( "stridebreaker_lane_clear_reset_aa", "Use stridebreaker >= x minion (after aa)", 5, 1, 10 );
						item::stridebreaker_jungle_clear = stridebreaker->add_slider( "stridebreaker_jungle_clear", "Use stridebreaker >= x monster", 2, 1, 10 );
						item::stridebreaker_jungle_clear_reset_aa = stridebreaker->add_slider( "stridebreaker_jungle_clear_reset_aa", "Use stridebreaker >= x monster (after aa)", 1, 1, 10 );
						stridebreaker->add_separator( "stridebreaker_3", " - - Fast LaneClear - - " );
						item::stridebreaker_fast_lane_clear = stridebreaker->add_checkbox( "stridebreaker_fast_lane_clear", "Use stridebreaker fast laneclear", true );
						stridebreaker->add_separator( "stridebreaker_4", " - - Auto use on - - " );
						for ( auto& enemy : entitylist->get_enemy_heroes( ) )
						{
							item::stridebreaker_always_use_on [ enemy->get_network_id( ) ] = stridebreaker->add_checkbox( std::to_string( enemy->get_network_id( ) ), enemy->get_model( ), false, true );
							item::stridebreaker_always_use_on [ enemy->get_network_id( ) ]->set_texture( enemy->get_square_icon_portrait( ) );
						}
					}

					auto goredrinker = item->add_tab( "goredrinker", "Goredrinker" );
					{
						auto icon = database->get_item_by_id( ItemId::Goredrinker )->get_texture( );
						auto goredrinker_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						goredrinker->set_assigned_texture( goredrinker_icon );

						goredrinker->add_separator( "goredrinker", " - - Goredrinker - - " );
						item::goredrinker = goredrinker->add_checkbox( "use_goredrinker", "Goredrinker", true );
						goredrinker->set_assigned_active( item::goredrinker );
						goredrinker->add_separator( "goredrinker_1", " - - Combo - - " );
						item::goredrinker_combo_count_hero = goredrinker->add_slider( "goredrinker_combo_count_hero", "Use Goredrinker >= x hero", 2, 1, 6 );
						item::goredrinker_combo_hp = goredrinker->add_slider( "goredrinker_combo_hp", "Use Goredrinker <= x% hp", 30, 0, 100 );
						goredrinker->add_separator( "goredrinker_5", " - - Harass - - " );
						item::goredrinker_harass = goredrinker->add_checkbox( "goredrinker_harass", "Use Goredrinker harass", true );
						goredrinker->add_separator( "goredrinker_2", " - - LaneClear - - " );
						item::goredrinker_lane_clear = goredrinker->add_slider( "goredrinker_lane_clear", "Use Goredrinker >= x minion", 7, 1, 10 );
						item::goredrinker_lane_clear_reset_aa = goredrinker->add_slider( "goredrinker_lane_clear_reset_aa", "Use Goredrinker >= x minion (after aa)", 5, 1, 10 );
						item::goredrinker_jungle_clear = goredrinker->add_slider( "goredrinker_jungle_clear", "Use Goredrinker >= x monster", 2, 1, 10 );
						item::goredrinker_jungle_clear_reset_aa = goredrinker->add_slider( "goredrinker_jungle_clear_reset_aa", "Use Goredrinker >= x monster (after aa)", 1, 1, 10 );
						goredrinker->add_separator( "goredrinker_3", " - - Fast LaneClear - - " );
						item::goredrinker_fast_lane_clear = goredrinker->add_checkbox( "goredrinker_fast_lane_clear", "Use Goredrinker fast laneclear", true );
						goredrinker->add_separator( "goredrinker_4", " - - Auto - - " );
						item::goredrinker_auto_use = goredrinker->add_slider( "goredrinker_auto_use", "Auto Goredrinker >= x hero", 3, 1, 6 );
					}
				}

				item->add_separator( "deffensive", " - - Deffensive - - " );
				{
					auto zhonya = item->add_tab( "zhonya", "Zhonya's hourglass" );
					{
						zhonya->add_separator( "zhonya_sep", " - - Zhonya's hourglass - - " );
						item::zhonya = zhonya->add_checkbox( "zhonya", "Zhonya's hourglass", true );

						zhonya->add_separator( "health_sep", " - - Health logic - - " );
						item::min_health = zhonya->add_slider( "min_health", "Use if health <= {x}%", 25, -1, 60 ); item::min_health->set_tooltip( "-1 = never" );
						item::min_enemies = zhonya->add_slider( "min_enemies", "^Only if enemies around >= {x}", 1, 0, 3 );
						item::min_allies = zhonya->add_slider( "min_allies", "^Only if allies around >= {x}", 0, 0, 3 );

						zhonya->add_separator( "spells_sep", " - - Spells - - " );
						item::zhonya_spells = zhonya->add_checkbox( "zhonya_spells", "Enabled", true );
						item::zhonya_buffer = zhonya->add_slider( "zhonya_buffer", "Evade buffer (ticks)", 2, 1, 6, true );
						item::evade_force = zhonya->add_slider( "zhonya_override_hp", "Evade anything if hp <= {x}% (?)", 33, -1, 100, true );
						item::evade_force->set_tooltip( "-1 = disabled" );

						zhonya->add_separator( "spells_whitelist", " - - Whitelist - - " );
						item::zhonya_wl_allies = zhonya->add_checkbox( "zhonya_wl_allies", "Show allies", true );

						auto tab_tw = zhonya->add_tab( "_evade_towershot", "Towershot" );
						item::evade_towershot = tab_tw->add_checkbox( "evade_towershot", "Enabled", true );
						item::evade_towershot_hp = tab_tw->add_slider( "min_health", "^Only if health <= {x}%", 25, 0, 60 );
						tab_tw->set_assigned_active( item::evade_towershot );

						for ( auto&& x : entitylist->get_all_heroes( ) )
						{
							auto tab = zhonya->add_tab( "_evade_" + std::to_string( static_cast< int >( x->get_champion( ) ) ), x->get_model( ) );
							tab->set_texture( x->get_square_icon_portrait( ) );
							add_targeted_spell_entry( "_evade", tab, x );

							m_zhonya_tabs.push_back( s_zhonya_map { x->is_ally( ), tab } );
						}

#pragma region cb
						auto zicon = database->get_item_by_id( ItemId::Zhonyas_Hourglass )->get_texture( );
						auto z_icon = create_texture_descriptor( zicon.first, { zicon.second.x, zicon.second.y, zicon.second.z, zicon.second.w } );

						if ( z_icon )
							zhonya->set_texture( z_icon );
						zhonya->set_assigned_active( item::zhonya );

						auto min_health_change = [ ]( TreeEntry* s ) {
							item::min_enemies->is_hidden( ) =
								item::min_allies->is_hidden( ) = s->get_int( ) <= -1;
						};
						min_health_change( item::min_health );
						item::min_health->add_property_change_callback( min_health_change );

						auto zhonya_wl_allies_click = [ ]( TreeEntry* s ) {
							for ( auto&& p : m_zhonya_tabs )
								p.tab->is_hidden( ) = ( p.is_ally && !s->get_bool( ) );
						};

						/*scheduler->delay_action( 0.1f, [ & ]( ) { */zhonya_wl_allies_click( item::zhonya_wl_allies );/* } );*/
						item::zhonya_wl_allies->add_property_change_callback( zhonya_wl_allies_click );

						for ( auto&& x : entitylist->get_enemy_turrets( ) )
						{
							if ( x && x->get_square_icon_portrait( ) )
							{
								tab_tw->set_texture( x->get_square_icon_portrait( ) );
								break;
							}
						}
#pragma endregion
					}

					auto gargoyle_stoneplate = item->add_tab( "gargoyle_stoneplate", "Gargoyle Stoneplate" );
					{
						auto icon = database->get_item_by_id( ItemId::Gargoyle_Stoneplate )->get_texture( );
						auto gargoyle_stoneplate_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						gargoyle_stoneplate->set_assigned_texture( gargoyle_stoneplate_icon );

						gargoyle_stoneplate->add_separator( "gargoyle_stoneplate", " - - Gargoyle Stoneplate - - " );
						item::gargoyle_stoneplate = gargoyle_stoneplate->add_checkbox( "use_gargoyle_stoneplate", "Gargoyle Stoneplate", true );
						gargoyle_stoneplate->set_assigned_active( item::gargoyle_stoneplate );
						gargoyle_stoneplate->add_separator( "gargoyle_stoneplate_1", " - - Auto - - " );
						item::gargoyle_stoneplate_hp = gargoyle_stoneplate->add_slider( "gargoyle_stoneplate_hp", "Use Gargoyle Stoneplate <= x% hp", 15, 0, 50 );
						item::gargoyle_stoneplate_damage = gargoyle_stoneplate->add_slider( "gargoyle_stoneplate_damage", "Use Gargoyle Stoneplate incoming damage >= x% Gargoyle Stoneplate shield", 80, 0, 100 );
						item::gargoyle_stoneplate_damage_hp = gargoyle_stoneplate->add_slider( "gargoyle_stoneplate_damage_hp", "  ^- Only <= x % hp", 30, 0, 100 );
					}

					auto randuins = item->add_tab( "randuins", "Randuins" );
					{
						auto icon = database->get_item_by_id( ItemId::Randuins_Omen )->get_texture( );
						auto randuins_icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						randuins->set_assigned_texture( randuins_icon );

						randuins->add_separator( "randuins", " - - Randuins - - " );
						item::randuins = randuins->add_checkbox( "use_randuins", "Randuins", true );
						randuins->set_assigned_active( item::randuins );
						randuins->add_separator( "randuins_1", " - - Auto use on - - " );
						for ( auto& enemy : entitylist->get_enemy_heroes( ) )
						{
							item::randuins_always_use_on [ enemy->get_network_id( ) ] = randuins->add_checkbox( std::to_string( enemy->get_network_id( ) ), enemy->get_model( ), false, true );
							item::randuins_always_use_on [ enemy->get_network_id( ) ]->set_texture( enemy->get_square_icon_portrait( ) );
						}
					}

					auto solari = item->add_tab( "solari", "Locket of the Iron Solari" );
					{
						auto icon = database->get_item_by_id( ItemId::Locket_of_the_Iron_Solari )->get_texture( );
						auto _icon = create_texture_descriptor( icon.first, { icon.second.x, icon.second.y, icon.second.z, icon.second.w } );
						solari->set_assigned_texture( _icon );

						solari->add_separator( "solari_sep", " - -  Locket of the Iron Solari  - - " );
						item::locket::enabled = solari->add_checkbox( "enabled", "Enabled", true );
						item::locket::health = solari->add_slider( "health", " ^ - Health (%)", 15, 0, 60 );

						solari->add_separator( "solari_sep2", " - -  Whitelist  - - " );
						for ( auto&& x : entitylist->get_ally_heroes( ) )
						{
							if ( x && x->is_valid( ) )
							{
								item::locket::wl [ x->get_champion( ) ] = solari->add_checkbox( "whitelist_" + std::to_string( static_cast< int >( x->get_champion( ) ) ), x->get_model( ), true );
								item::locket::wl [ x->get_champion( ) ]->set_texture( x->get_square_icon_portrait( ) );
							}
						}
						solari->set_assigned_active( item::locket::enabled );
					}

					auto heartsteel = item->add_tab( "heartsteel", "Heartsteel" );
					{
						auto heart = database->get_item_by_id( ItemId::Heartsteel )->get_texture( );
						auto heart_icon = create_texture_descriptor( heart.first, { heart.second.x, heart.second.y, heart.second.z, heart.second.w } );
						heartsteel->set_assigned_texture( heart_icon );
						//heartsteel->add_image_item("aaa", heart.first,30 );

						heartsteel->add_separator( "heartsteel", " - - Heartsteel - - " );
						item::use_heartsteel = heartsteel->add_checkbox( "use_heartsteel", "Force attack heartsteel enemy", true );
						heartsteel->set_assigned_active( item::use_heartsteel );
						item::draw_heartsteel = heartsteel->add_checkbox( "draw_heartsteel", "Draw heartsteel state", true );
					}
				}

				item->add_separator( "potions", " - - Potions - - " );
				{
					auto hp_potion = item->add_tab( "hp_potion", "Health Potion" );
					{
						auto hp = database->get_item_by_id( ItemId::Health_Potion )->get_texture( );
						auto hp_icon = create_texture_descriptor( hp.first, { hp.second.x, hp.second.y, hp.second.z, hp.second.w } );
						hp_potion->set_assigned_texture( hp_icon );

						hp_potion->add_separator( "hp_potion", " - - Health Potion - - " );
						potions::Health_Potion = hp_potion->add_checkbox( "Health_Potion", "Use Health Potion", true );
						potions::Health_Potion_hp = hp_potion->add_slider( "Health_Potion_hp", "Hp <= x%", 50, 0, 100 );
						potions::Health_Potion_c = hp_potion->add_checkbox( "Health_Potion_c", "Use in combo", true );
						potions::Health_Potion_harass = hp_potion->add_checkbox( "Health_Potion_harass", "Use in harass", false );
						potions::Health_Potion_other = hp_potion->add_checkbox( "Health_Potion_other", "Use in other mode", false );
						potions::Health_Potion_rang = hp_potion->add_slider( "Health_Potion_rang", "Use if x range", 1000, 1, 1000 );
						potions::Health_Potion_enemy = hp_potion->add_slider( "Health_Potion_enemy", "  ^- enemy >= x", 1, 0, 6 );
						hp_potion->set_assigned_active( potions::Health_Potion );
					}

					auto green_potion = item->add_tab( "green_potion", "Refillable Potion" );
					{
						auto hp = database->get_item_by_id( ItemId::Refillable_Potion )->get_texture( );
						auto hp_icon = create_texture_descriptor( hp.first, { hp.second.x, hp.second.y, hp.second.z, hp.second.w } );
						green_potion->set_assigned_texture( hp_icon );

						green_potion->add_separator( "green_potion", " - - Refillable Potion - - " );
						potions::Refillable_Potion = green_potion->add_checkbox( "Refillable_Potion", "Use Refillable Potion", true );
						potions::Refillable_Potion_hp = green_potion->add_slider( "Refillable_Potion_hp", "Hp <= x%", 50, 0, 100 );
						potions::Refillable_Potion_c = green_potion->add_checkbox( "Refillable_Potion_c", "Use in combo", true );
						potions::Refillable_Potion_harass = green_potion->add_checkbox( "Refillable_Potion_harass", "Use in harass", false );
						potions::Refillable_Potion_other = green_potion->add_checkbox( "Refillable_Potion_other", "Use in other mode", false );
						potions::Refillable_Potion_rang = green_potion->add_slider( "Refillable_Potion_rang", "Use if x range", 1000, 1, 1000 );
						potions::Refillable_Potion_enemy = green_potion->add_slider( "Refillable_Potion_enemy", "  ^- enemy >= x", 1, 0, 6 );
						green_potion->set_assigned_active( potions::Refillable_Potion );
					}

					auto many_potion = item->add_tab( "many_potion", "Corrupting Potion" );
					{
						auto hp = database->get_item_by_id( ItemId::Corrupting_Potion )->get_texture( );
						auto hp_icon = create_texture_descriptor( hp.first, { hp.second.x, hp.second.y, hp.second.z, hp.second.w } );
						many_potion->set_assigned_texture( hp_icon );

						many_potion->add_separator( "many_potion", " - - Corrupting Potion - - " );
						potions::Corrupting_Potion = many_potion->add_checkbox( "Corrupting_Potion", "Use Corrupting Potion", true );
						potions::Corrupting_Potion_hp = many_potion->add_slider( "Corrupting_Potion_hp", "Hp <= x%", 50, 0, 100 );
						potions::Corrupting_Potion_c = many_potion->add_checkbox( "Corrupting_Potion_c", "Use in combo", true );
						potions::Corrupting_Potion_harass = many_potion->add_checkbox( "Corrupting_Potion_harass", "Use in harass", false );
						potions::Corrupting_Potion_other = many_potion->add_checkbox( "Corrupting_Potion_other", "Use in other mode", false );
						potions::Corrupting_Potion_rang = many_potion->add_slider( "Corrupting_Potion_rang", "Use if x range", 1000, 1, 1000 );
						potions::Corrupting_Potion_enemy = many_potion->add_slider( "Corrupting_Potion_enemy", "  ^- enemy >= x", 1, 0, 6 );
						many_potion->set_assigned_active( potions::Corrupting_Potion );
					}

					auto cokkie_potion = item->add_tab( "biscuit_potion", "Biscuit" );
					{
						auto hp = database->get_item_by_id( ItemId::Total_Biscuit_of_Everlasting_Will )->get_texture( );
						auto hp_icon = create_texture_descriptor( hp.first, { hp.second.x, hp.second.y, hp.second.z, hp.second.w } );
						cokkie_potion->set_assigned_texture( hp_icon );

						cokkie_potion->add_separator( "cokkie_potion", " - - Biscuit - - " );
						potions::Biscuit = cokkie_potion->add_checkbox( "Biscuit", "Use Biscuit", true );
						potions::Biscuit_hp = cokkie_potion->add_slider( "Biscuit_hp", "Hp <= x%", 30, 0, 100 );
						potions::Biscuit_c = cokkie_potion->add_checkbox( "Biscuit_c", "Use in combo", true );
						potions::Biscuit_harass = cokkie_potion->add_checkbox( "Biscuit_harass", "Use in harass", false );
						potions::Biscuit_other = cokkie_potion->add_checkbox( "Biscuit_other", "Use in other mode", false );
						potions::Biscuit_rang = cokkie_potion->add_slider( "Biscuit_rang", "Use if x range", 1000, 1, 1000 );
						potions::Biscuit_enemy = cokkie_potion->add_slider( "Biscuit_enemy", "  ^- enemy >= x", 1, 0, 6 );
						cokkie_potion->set_assigned_active( potions::Biscuit );
					}
				}

				item->add_separator( "other", " - - Other - - " );
				auto draw_ward = item->add_tab( "draw_ward", "Ward" );
				{
					auto ward = database->get_item_by_id( ItemId::Stealth_Ward )->get_texture( );
					auto ward_icon1 = create_texture_descriptor( ward.first, { ward.second.x, ward.second.y, ward.second.z, ward.second.w } );
					auto ward_icon2 = create_texture_descriptor( ward.first, { ward.second.x, ward.second.y, ward.second.z, ward.second.w } );
					auto ward_icon3 = create_texture_descriptor( ward.first, { ward.second.x, ward.second.y, ward.second.z, ward.second.w } );
					auto ward_icon4 = create_texture_descriptor( ward.first, { ward.second.x, ward.second.y, ward.second.z, ward.second.w } );

					auto blue = database->get_item_by_id( ItemId::Farsight_Alteration )->get_texture( );
					auto blue_icon1 = create_texture_descriptor( blue.first, { blue.second.x, blue.second.y, blue.second.z, blue.second.w } );
					auto blue_icon2 = create_texture_descriptor( blue.first, { blue.second.x, blue.second.y, blue.second.z, blue.second.w } );

					auto red = database->get_item_by_id( ItemId::Oracle_Lens )->get_texture( );
					auto red_icon1 = create_texture_descriptor( red.first, { red.second.x, red.second.y, red.second.z, red.second.w } );
					auto red_icon2 = create_texture_descriptor( red.first, { red.second.x, red.second.y, red.second.z, red.second.w } );

					auto ward_ct = database->get_item_by_id( ItemId::Control_Ward )->get_texture( );
					auto ward_icon_ct = create_texture_descriptor( ward_ct.first, { ward_ct.second.x, ward_ct.second.y, ward_ct.second.z, ward_ct.second.w } );
					auto ward_icon_ct1 = create_texture_descriptor( ward_ct.first, { ward_ct.second.x, ward_ct.second.y, ward_ct.second.z, ward_ct.second.w } );
					auto ward_icon_ct2 = create_texture_descriptor( ward_ct.first, { ward_ct.second.x, ward_ct.second.y, ward_ct.second.z, ward_ct.second.w } );

					//auto control = database->get_item_by_id( ItemId::Control_Ward )->get_texture( );

					draw_ward->set_assigned_texture( ward_icon1 );

					draw_ward->add_separator( "draw_ward1", " - - Ward - - " );
					item::draw_ward = draw_ward->add_checkbox( "ward_bush", "Auto ward enemies in bush", true );
					item::draw_ward = draw_ward->add_checkbox( "draw_ward2", "Draw ward range", true );
					item::draw_ward_minimap = draw_ward->add_checkbox( "draw_ward_minimap", "Draw ward minimap", true );
					draw_ward->set_assigned_active( item::draw_ward );
					item::draw_ward_ally = draw_ward->add_hotkey( "draw_ward_ally", "Draw ward range ally", TreeHotkeyMode::Hold, 0x14, false );
					item::draw_ward_time = draw_ward->add_hotkey( "draw_ward_time", "Draw ward time", TreeHotkeyMode::Hold, 0x14, false );
					item::draw_ward_time_ally = draw_ward->add_hotkey( "draw_ward_time_ally", "Draw ward time ally", TreeHotkeyMode::Hold, 0x14, false );
					//item::draw_ward_time_map = draw_ward->add_hotkey( "draw_ward_time_map", "Draw ward time minimap", TreeHotkeyMode::Hold, 0x14, false );
					item::draw_ward_sz = draw_ward->add_slider( "draw_ward_sz", "Size", static_cast< int32_t >( renderer->get_dpi_factor( ) * 100.f ), 50, 300 );
					item::draw_ward_remaining_t_color = draw_ward->add_colorpick( "draw_ward_remaining_t_color", "Progress bar color", g_utils->clr_default_coral );

					draw_ward->add_separator( "auto_warding_sep", " - - Auto warding - - " );
					item::auto_ward_mode = draw_ward->add_combobox( "auto_ward_mode", "Auto warding", { { "Disabled", nullptr }, { "Suggested", nullptr }, { "All ", nullptr } }, 0 );
					item::semi_auto_ward_key = draw_ward->add_hotkey( "semi_auto_ward_key", "Semi-ward", TreeHotkeyMode::Hold, 0x00, false );
					item::semi_auto_ward_key->set_tooltip( "Use ward on closest spot" );

					item::auto_ward_control = draw_ward->add_checkbox( "auto_ward_control", "Use control ward", true ); item::auto_ward_control->set_texture( ward_icon_ct );
					item::auto_ward_stealth = draw_ward->add_checkbox( "auto_ward_stealth", "Use stealth ward", true ); item::auto_ward_stealth->set_texture( ward_icon2 );

					draw_ward->add_separator( "draw_ward4", " - - Draw ward spots - - " );
					item::draw_auto_ward_control = draw_ward->add_checkbox( "draw_auto_ward_control", "Draw control ward spots", true ); item::draw_auto_ward_control->set_texture( ward_icon_ct1 );
					item::draw_auto_ward_stealth = draw_ward->add_checkbox( "draw_auto_ward_stealth", "Draw stealth ward spots", true ); item::draw_auto_ward_stealth->set_texture( ward_icon3 );

					draw_ward->add_separator( "wards_bush_assist", " - - Auto ward bush - - " );
					item::auto_ward_bush = draw_ward->add_checkbox( "auto_ward_bush", "Use stealth ward", true ); item::auto_ward_bush->set_texture( ward_icon4 );
					item::auto_ward_bush_ctrl = draw_ward->add_checkbox( "auto_ward_bush_ctrl", "Use control ward", false ); item::auto_ward_bush_ctrl->set_texture( ward_icon_ct2 );
					item::auto_ward_bush_combo = draw_ward->add_checkbox( "auto_ward_bush_combo", "Only in Combo", true );

					draw_ward->add_separator( "wards_cast_assist", " - - Trick spots - - " );
					item::ward_cast_assist = draw_ward->add_checkbox( "ward_cast_assist", "Ward cast assist", true );

					draw_ward->add_separator( "buy_ward", " - - Auto buy - - " );
					item::buy_ward = draw_ward->add_combobox( "auto_buy_ward", "Auto buy other ward", { { "None", nullptr, }, { "Oracle Lens", red_icon1 }, { "Farsight Alteration", blue_icon1 } }, 0, true );
					item::buy_ward_lv = draw_ward->add_slider( "buy_ward_level", " ^- Level >= x", 9, 6, 18 );
					item::sup_buy_ward = draw_ward->add_combobox( "sup_buy_ward", "Auto buy other ward (Sup)", { { "None", nullptr, }, { "Oracle Lens", red_icon2 }, { "Farsight Alteration", blue_icon2 } }, 1, true );


					/*
					draw_ward->add_separator( "auto_ward_bush", " - - Auto ward bush - - " );
					item::auto_ward_yellow = draw_ward->add_checkbox( "auto_ward_yellow", "Use stealthward/support item", true );
					item::auto_ward_blue = draw_ward->add_checkbox( "auto_ward_blue", "Use farsight", false );
					item::auto_ward_control = draw_ward->add_checkbox( "auto_ward_control", "Use control ward", false );

					auto ward_icon_yellow = create_texture_descriptor( ward.first, { ward.second.x, ward.second.y, ward.second.z, ward.second.w } );
					auto ward_icon_blue = create_texture_descriptor( blue.first, { blue.second.x, blue.second.y, blue.second.z, blue.second.w } );
					auto ward_icon_ctrl = create_texture_descriptor( control.first, { control.second.x, control.second.y, control.second.z, control.second.w } );

					item::auto_ward_yellow->set_texture( ward_icon_yellow );
					item::auto_ward_blue->set_texture( ward_icon_blue );
					item::auto_ward_control->set_texture( ward_icon_ctrl );*/
				}
			}

			auto trap = main_tab->add_tab( "trap", "Traps" );
			trap->add_separator( "separator_traps", " - -  Traps  - - " );
			auto tab_traps_common = trap->add_tab( "tab_traps_common", "Common" );
			trap->add_separator( "separator_traps_s", " - -  Settings  - - " );
			{
				tab_traps_common->add_separator( "sep_circle", " - -  Circles  - - " );
				traps::glow_1 = tab_traps_common->add_slider( "insize_glow_sz", "Inside glow size", 69, 0, 100 );
				traps::glow_2 = tab_traps_common->add_slider( "insize_glow_pw", "Inside glow power", 40, 0, 100 );
				traps::glow_3 = tab_traps_common->add_slider( "outside_glow_sz", "Outside glow size", 100, 0, 100 );
				traps::glow_4 = tab_traps_common->add_slider( "outside_glow_pw", "Outside glow power", 20, 0, 100 );

				tab_traps_common->add_separator( "sep_glow", " - -  Glow  - - " );
				traps::glow_th = tab_traps_common->add_slider( "glow_th", "Thickness", 1, 1, 10 );
				traps::glow_bl = tab_traps_common->add_slider( "glow_bl", "Blur", 1, 1, 10 );

				traps::glow_th->add_property_change_callback( [ ]( TreeEntry* s ) { menu_update_glow( ); } );
				traps::glow_bl->add_property_change_callback( [ ]( TreeEntry* s ) { menu_update_glow( ); } );
			}
			for ( auto x : m_trap_info )
			{
				auto trap_menu = s_trap_menu( );

				trap_menu.tab = trap->add_tab( x.second.get_config_key( ), x.second.name );
				trap_menu.tab->add_separator( x.second.get_config_key( ) + "_sep", " - -  " + x.second.name + "  - - " );

				if ( x.second.spell_hash.has_value( ) )
				{
					auto spell = database->get_spell_by_hash( x.second.spell_hash.value( ) );
					if ( spell && spell->get_icon_texture_by_index( 0 ) )
						trap_menu.tab->set_texture( spell->get_icon_texture_by_index( 0 ) );
				}

				trap_menu.enabled_circle = trap_menu.tab->add_checkbox( x.second.get_config_key( ) + "_circle", "Draw circle", true );

				float def_color_1 [ 4 ]; A_::d3d_to_menu( x.second.default_color.has_value( ) ? x.second.default_color.value( ) : utility::COLOR_WHITE, def_color_1 );
				float def_color_1_2 [ 4 ]; A_::d3d_to_menu( x.second.default_color2.has_value( ) ? x.second.default_color2.value( ) : utility::COLOR_WHITE, def_color_1_2 );
				float def_color_2 [ 4 ]; A_::d3d_to_menu( x.second.default_color_glow.has_value( ) ? x.second.default_color_glow.value( ) : utility::COLOR_WHITE, def_color_2 );

				trap_menu.circle_color = trap_menu.tab->add_colorpick( x.second.get_config_key( ) + "_circle_color", "Circle color", def_color_1 );
				trap_menu.circle_color2 = trap_menu.tab->add_colorpick( x.second.get_config_key( ) + "_circle_color2", "Circle color2", def_color_1_2 );

				trap_menu.tab->add_separator( x.second.get_config_key( ) + "_glow_sep", " - -  Glow  - - " );
				trap_menu.enabled_glow = trap_menu.tab->add_checkbox( x.second.get_config_key( ) + "_glow", "Glow", true );
				trap_menu.glow_color = trap_menu.tab->add_colorpick( x.second.get_config_key( ) + "_glow_color", "Glow color", def_color_2 );

				trap_menu.enabled_glow->add_property_change_callback( [ ]( TreeEntry* s ) { menu_update_glow( ); } );
				trap_menu.glow_color->add_property_change_callback( [ ]( TreeEntry* s ) { menu_update_glow( ); } );

				m_trap_menu [ x.first ] = trap_menu;
			}

			level_up::create_menu( );
			auto hero = main_tab->add_tab( "hero", "Hero" );
			{
				hero->add_separator( "hero", " - - Hero - - " );
				champion::create_menu( );
			}

			misc::create_menu( );
			auto pings = main_tab->add_tab( "auto_pings", "Auto ping" );
			{
				pings->add_separator( "ping_wards_sep", " - -  Wards  - - " );
				autoping::wards = pings->add_checkbox( "ping_wards", "Enabled", false );
				autoping::delay_1 = pings->add_slider( "delay_1", "Min delay", 333, 0, 2000 );
				autoping::delay_2 = pings->add_slider( "delay_2", "Max delay", 660, 0, 2000 );
				autoping::randomize_pos = pings->add_slider( "randomize_pos", "Randomize position", 35, 0, 150 );
				autoping::only_on_screen = pings->add_checkbox( "only_on_screen", "Only on screen", true );

				pings->add_separator( "ping_enemies_sep", " - -  Enemies (FoW)  - - " );
				autoping::enemies = pings->add_checkbox( "ping_enemies", "Enabled", false );
				autoping::only_jungler = pings->add_checkbox( "only_jungler", "Only jungler", true );

				pings->add_separator( "ping_obj_sep", " - -  Objectives (FoW)  - - " );
				fow_tracker->global_ping = pings->add_checkbox( "global_ping_objective", "Enabled", false );
			}

			main_tab->add_tab( "notifier", "Notify" );
			main_tab->add_tab( "sidehud", "Side HUD" );

			main_tab->add_separator( "fakeping_sep", " - - Fake latency ping - - " );
			fake_ping_value = main_tab->add_text_input( "fake_ping_text", "Fake latency", "0" ); fake_ping_value->set_tooltip( "[0-" + std::to_string( std::numeric_limits<uint16_t>::max( ) ) + "]" );
			main_tab->add_button( "fake_ping", "Send ping" )->add_property_change_callback( [ ]( TreeEntry* s ) {
				auto get_ping_v = [ ]( ) -> std::optional<uint16_t>
				{
					auto txt = fake_ping_value->get_string( );
					if ( txt.empty( ) )
						return std::nullopt;

					auto ping_v = std::atoi( txt.c_str( ) );
					return static_cast< uint16_t >( std::clamp( ping_v, 0, static_cast< int >( std::numeric_limits<uint16_t>::max( ) ) ) );
				};

				auto v = get_ping_v( );
				if ( v.has_value( ) )
					myhero->send_latency_ping( v.value( ) );
			} );

			main_tab->add_separator( "info0", " - - Info - - " );
			main_tab->add_tab( "utility_info", "Info" );
		}

		event_handler<events::on_update>::add_callback( on_update );
		event_handler<events::on_update>::add_callback( on_update_highest, event_prority::highest );

		event_handler < events::on_cast_spell>::add_callback( on_cast_spell );

		event_handler<events::on_draw>::add_callback( on_draw );
		event_handler<events::on_env_draw>::add_callback( on_new_draw );

		event_handler<events::on_issue_order>::add_callback( on_issue_order );

		event_handler<events::on_after_attack_orbwalker>::add_callback( on_after_attack_orbwalker );
		event_handler<events::on_process_spell_cast>::add_callback( on_process_spell_cast );

		event_handler< events::on_create_object >::add_callback( on_create_object );
		event_handler< events::on_delete_object >::add_callback( on_delete_object );

		A_::on_load( );
		spell_tracker::load( );
		level_up::load( );
		misc::all_load( );
		fow_tracker->load( );
	}

	void unload( )
	{
		//dash::unload();
		summoner_spell::all_unload( );
		champion::all_unload( );
		misc::all_unload( );
		menu->delete_tab( main_tab );

		plugin_sdk->remove_spell( q );
		plugin_sdk->remove_spell( w );
		plugin_sdk->remove_spell( e );
		plugin_sdk->remove_spell( r );

		event_handler<events::on_update>::remove_handler( on_update );
		event_handler<events::on_update>::remove_handler( on_update_highest );

		event_handler < events::on_cast_spell>::remove_handler( on_cast_spell );

		event_handler<events::on_draw>::remove_handler( on_draw );
		event_handler<events::on_env_draw>::remove_handler( on_new_draw );

		event_handler<events::on_issue_order>::remove_handler( on_issue_order );

		event_handler<events::on_after_attack_orbwalker>::remove_handler( on_after_attack_orbwalker );
		event_handler<events::on_process_spell_cast>::remove_handler( on_process_spell_cast );

		event_handler< events::on_create_object >::remove_handler( on_create_object );
		event_handler< events::on_delete_object >::remove_handler( on_delete_object );

		event_handler< events::on_new_path >::remove_handler( new_path );


		spell_tracker::unload( );
		level_up::unload( );
		fow_tracker->unload( );

		delete fow_tracker; delete item::b_gunblade;
	}

	void hidden( )
	{
		////ARAM
		//if ( missioninfo->get_map_id( ) == game_map_id::HowlingAbyss )
		//{
		//	const auto item = ( TreeTab* ) main_tab->get_entry( "item" );
		//	const auto many_potion = ( TreeTab* ) item->get_entry( "many_potion" );
		//	const auto draw_ward = ( TreeTab* ) item->get_entry( "draw_ward" );
		//	many_potion->is_hidden( ) = draw_ward->is_hidden( ) = true;
		//}
	}

#pragma region items

	void use_ward_bush( )
	{
	}


	auto use_galeforce( ) -> void
	{
		constexpr static auto ITEM_GALEFORCE_RANGE = 750;
		constexpr static auto GALEFORCE_DASH_RANGE = 420;

		auto get_galeforce_damage = [ ]( game_object_script target ) -> float
		{
			auto base_damage = std::clamp( 150.f + std::max( myhero->get_level( ) - 8, 0 ) * 18.18f, 150.f, 350.f )
				+ ( myhero->get_additional_attack_damage( ) ) * 0.45f;

			auto missing_health = std::clamp( 100.f - target->get_health_percent( ), 0.f, 75.f );
			auto bonus_damage = floorf( missing_health / 7.5f ) * 5.f;

			base_damage += ( base_damage * bonus_damage / 100.f );

			auto real_damage = damagelib->calculate_damage_on_unit( myhero, target, damage_type::physical, base_damage );

			return real_damage;
		};

		if ( !item::galeforce->get_bool( ) || !orbwalker->combo_mode( ) )
			return;

		if ( !myhero->can_move( ) )
			return;

		auto gf = myhero->has_item( { ItemId::Galeforce, ItemId::Typhoon } );
		if ( gf == spellslot::invalid )
			return;

		auto spell_state = myhero->get_spell_state( gf );
		if ( spell_state != spell_state::Ready )
			return;

		auto enemies = entitylist->get_enemy_heroes( );
		enemies.erase( std::remove_if( enemies.begin( ), enemies.end( ), [ ]( game_object_script x ) {
			if ( !x || !x->is_valid_target( ( ITEM_GALEFORCE_RANGE + GALEFORCE_DASH_RANGE ) * 0.9f ) || x->is_invulnerable( ) ||
				x->has_buff( { buff_hash( "KindredRNoDeathBuff" ), buff_hash( "UndyingRage" ), buff_hash( "ChronoRevive" ), buff_hash( "ChronoShift" ) } ) )
				return true;

			if ( item::galeforce_whitelist.find( x->get_champion( ) ) == item::galeforce_whitelist.end( ) ||
				!item::galeforce_whitelist [ x->get_champion( ) ]->get_bool( ) )
				return true;

			return false;
		} ), enemies.end( ) );

		auto target = target_selector->get_target( enemies, damage_type::physical );
		if ( !target || !target->is_valid_target( ) )
			return;

		auto can_kill_gf = target->get_real_health( true, false ) - get_galeforce_damage( target ) < 0;
		auto can_kill_gfaa = target->get_real_health( true, false ) - get_galeforce_damage( target ) - myhero->get_auto_attack_damage( target, false ) * item::galeforce_aa->get_int( ) < 0;
		if ( !can_kill_gf && !can_kill_gfaa )
			return;

		if ( target->get_position( ).count_enemies_in_range( 750.f ) > item::galeforce_max_enemies->get_int( ) )
			return;

		auto v_points = std::vector<vector> { };
		for ( auto i = 200; i < GALEFORCE_DASH_RANGE - 20; i += 20 )
		{
			auto circle = geometry::geometry::circle_points( myhero->get_position( ), static_cast<float>( i ), 360 / 10 );
			v_points.insert( v_points.begin( ), circle.begin( ), circle.end( ) );
		}

		auto max_range = can_kill_gf ? ( ITEM_GALEFORCE_RANGE ) * 0.9f : ( myhero->get_attack_range( ) + myhero->get_bounding_radius( ) + target->get_bounding_radius( ) ) * 0.85f;
		v_points.erase( std::remove_if( v_points.begin( ), v_points.end( ), [ target, max_range ]( vector x ) {
			if ( x.distance( target ) > max_range )
				return true;

			auto d = x.distance( myhero );
			for ( auto i = 20; i < d; i += 10 )
			{
				if ( myhero->get_position( ).extend( x, static_cast<float>( i ) ).is_wall( ) )
					return true;
			}

			if ( evade->is_dangerous_path( { myhero->get_position( ), x }, 0.33f ) )
				return true;

			if ( x.is_under_enemy_turret( ) && myhero->get_health_percent( ) < item::galeforce_dive_hp->get_int( ) )
				return true;

			if ( x.count_enemies_in_range( 750.f ) >= myhero->count_enemies_in_range( 750.f ) )
			{
				if ( x.count_enemies_in_range( 750.f ) > item::galeforce_max_enemies->get_int( ) && x.distance( target ) < myhero->get_distance( target ) )
					return true;
			}

			return false;
		} ), v_points.end( ) );

		if ( v_points.empty( ) )
			return;

		std::sort( v_points.begin( ), v_points.end( ), [ cursor = hud->get_hud_input_logic( )->get_game_cursor_position( ) ]( vector p1, vector p2 ) {
			return p1.distance( cursor ) < p2.distance( cursor );
		} );

		myhero->cast_spell( gf, v_points.front( ), true, false );
	}

	auto use_solari_locket( ) -> void
	{
		if ( !item::locket::enabled->get_bool( ) )
			return;

		static constexpr auto SOLARI_RANGE = 850.f;
		Item solari = Item( ItemId::Locket_of_the_Iron_Solari, SOLARI_RANGE * 0.9f );
		if ( !solari.is_vaild( ) || !solari.is_ready( ) )
			return;

		auto allies = entitylist->get_ally_heroes( );
		allies.erase( std::remove_if( allies.begin( ), allies.end( ), [ & ]( game_object_script x ) {
			if ( !x || !x->is_valid( ) || x->is_dead( ) )
				return true;

			if ( x->get_distance( myhero ) > solari._range )
				return true;

			auto dmg = health_prediction->get_incoming_damage( x, 0.5f, true );
			if ( dmg <= 0.f || ( x->get_health( ) - dmg ) / x->get_max_health( ) > static_cast< float >( item::locket::health->get_int( ) ) / 100.f )
				return true;

			return false;
		} ), allies.end( ) );

		if ( !allies.empty( ) )
			solari.cast( );
	}

	auto gb_logic( ) -> void
	{
		auto gb_item = myhero->has_item( { ItemId::Youmuus_Wake, ItemId::Youmuus_Ghostblade_Arena, ItemId::Youmuus_Ghostblade, ItemId::Youmuus_Wake_Arena } );
		if ( gb_item == spellslot::invalid )
			return;

		auto spell_state = myhero->get_spell_state( gb_item );
		if ( spell_state != spell_state::Ready )
			return;

		if ( !item::ghostblade->get_bool( ) )
			return;

		if ( item::chase_gb->get_bool( ) && orbwalker->combo_mode( ) )
		{
			auto dis = item::enemy_dis_gb->get_int( ) + myhero->get_attack_range( ) + myhero->get_bounding_radius( );
			auto hud_dis = item::mouse_dis_gb->get_int( );
			auto hud_pos = hud->get_hud_input_logic( )->get_game_cursor_position( );

			auto target = target_selector->get_target( dis, damage_type::physical );
			if ( target && target->get_distance( hud_pos ) < hud_dis && A_::checkbox( item::use_to_gb, target ) )
			{
				auto is_dashing = false;
				{
					auto ai_path = target->get_path_controller( );
					auto thru_wall = false;
					if ( ai_path && ai_path->is_dashing( ) )
					{
						auto d = ai_path->get_start_vec( ).distance( ai_path->get_end_vec( ) );
						if ( d > 20 )
						{
							for ( int i = 10; i < d; i += 10 )
							{
								if ( ai_path->get_start_vec( ).extend( ai_path->get_end_vec( ), static_cast<float>( i ) ).is_wall( ) )
								{
									thru_wall = true;
									break;
								}
							}
						}
						if ( !thru_wall )
							is_dashing = true;
					}
				}
				bool  = !myhero->is_in_auto_attack_range( target ) || target->get_move_speed( ) - myhero->get_move_speed( ) > 33.f || is_dashing;
				bool hp = target->get_health_percent( ) >= item::hp_high_gb->get_int( ) && target->get_health_percent( ) <= item::hp_low_gb->get_int( );
				if ( hp && myhero->is_facing( target ) &&  )
				{
					myhero->cast_spell( gb_item );
					return;
				}
			}
		}
		if ( item::flee_gb->get_bool( ) && orbwalker->flee_mode( ) )
		{
			if ( myhero->count_enemies_in_range( 750.f ) <= 0 || myhero->get_health_percent( ) > item::hp_low_flee_gb->get_int( ) )
				return;

			myhero->cast_spell( gb_item );
			return;
		}
	}
	//鐵刺
	void use_ironspike( )
	{
		if ( !item::ironspike->get_bool( ) || !myhero->can_move( ) )
			return;

		Item ironspike( ItemId::Ironspike_Whip, 400 );
		if ( ironspike.is_ready( ) )
		{
			if ( orbwalker->combo_mode( ) )
			{
				auto enemy = entitylist->get_enemy_heroes( );

				enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), [ ]( game_object_script x )
				{
					return !x->is_valid( ) || x->is_dead( ) || !x->is_valid_target( 400 ) || myhero->is_in_auto_attack_range( x );
				} ), enemy.end( ) );

				if ( !enemy.empty( ) )
				{
					ironspike.cast( );
				}
			}
			if ( orbwalker->mixed_mode( ) )
			{
				auto enemy = entitylist->get_enemy_heroes( );

				enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), [ & ]( game_object_script x )
				{
					return x->is_dead( ) || !x->is_valid_target( ironspike._range );
				} ), enemy.end( ) );

				if ( !enemy.empty( ) )
				{
					ironspike.cast( );
				}
			}
			if ( orbwalker->lane_clear_mode( ) )
			{
				if ( A_::count_minions_in_range( ironspike._range ) >= item::ironspike_lane_clear->get_int( ) )
				{
					ironspike.cast( );
				}
				if ( A_::count_monsters_in_range( ironspike._range ) >= item::ironspike_jungle_clear->get_int( ) )
				{
					ironspike.cast( );
				}
			}
			if ( orbwalker->lane_clear_mode( ) && ( orbwalker->get_orb_state( ) == ( orbwalker_state_flags::fast_lane_clear | orbwalker_state_flags::lane_clear ) ) && item::ironspike_fast_lane_clear->get_bool( ) )
			{
				if ( A_::count_minions_in_range( ironspike._range ) >= 1 || A_::count_monsters_in_range( ironspike._range ) >= 1 )
				{
					ironspike.cast( );
				}
			}
		}
	}

	void use_zhonya( )
	{
		if ( !item::zhonya->get_bool( ) )
			return;

		static std::vector<uint32_t> inv_shield_hashes =
		{
			buff_hash( "NocturneShroudofDarkness" ),
			buff_hash( "itemmagekillerveil" ),
			buff_hash( "bansheesveil" ),
			buff_hash( "SivirE" ),
			buff_hash( "malzaharpassiveshield" )
		};

		auto zhonya_item = myhero->has_item( 
			{ 
				ItemId::Zhonyas_Hourglass, ItemId::Commencing_Stopwatch, ItemId::Perfectly_Timed_Stopwatch, ItemId::Stopwatch, ItemId::Zhonyas_Hourglass_Arena,
				static_cast<ItemId>( 3090 ), static_cast< ItemId >( 3090 + 220000 ) //Wooglets Witchcap
			} );
		if ( zhonya_item == spellslot::invalid )
			return;

		if ( myhero->get_spell_state( zhonya_item ) != spell_state::Ready )
			return;

		auto pred_health_raw = health_prediction->get_health_prediction( myhero, 0.33f, 0.f );
		auto pred_health = pred_health_raw / myhero->get_max_health( ) * 100.f;

#pragma region health_based
		if ( item::min_health->get_int( ) != -1 )
		{
			auto allow_logic_num = ( item::min_enemies->get_int( ) == 0 || myhero->count_enemies_in_range( 1250 ) >= item::min_enemies->get_int( ) ) &&
				( item::min_allies->get_int( ) == 0 || myhero->count_allies_in_range( 900.f ) >= item::min_allies->get_int( ) );

			auto allow_logic_health = pred_health <= item::min_health->get_int( ) && pred_health_raw < myhero->get_health( );

			auto allow_logic_inv = !myhero->is_invulnerable( ) && !myhero->has_buff( inv_shield_hashes );

			if ( allow_logic_num && allow_logic_health && allow_logic_inv )
			{
				myhero->cast_spell( zhonya_item );
				return;
			}
		}
#pragma endregion

#pragma region process_evade_list
		auto lowhp = item::evade_force->get_int( ) != -1 && pred_health <= item::evade_force->get_int( );
		for ( auto i = 0; i < v_evade_queue.size( ); i++ )
		{
			if ( i >= v_evade_queue.size( ) )
				continue;

			auto t_hit = v_evade_queue [ i ].time - ( ( ping->get_ping( ) / 1000.f ) + 0.05f );
			if ( v_evade_queue [ i ].missile_network_id != 0 )
			{
				auto missile = entitylist->get_object_by_network_id( v_evade_queue [ i ].missile_network_id );
				if ( missile && missile->is_valid( ) && missile->get_position( ).is_valid( ) && missile->is_visible( ) && missile->is_missile( ) && missile->missile_get_target_id( ) == myhero->get_id( ) )
					t_hit = gametime->get_time( ) +
					( ( myhero->get_distance( missile ) - myhero->get_bounding_radius( ) ) / missile->get_missile_sdata( )->MissileSpeed( ) ) -
					( ( ping->get_ping( ) / 1000.f ) + item::zhonya_buffer->get_int( ) / 30.f );
			}

			if ( t_hit > gametime->get_time( ) )
				continue;

			if ( gametime->get_time( ) - t_hit >= ( item::zhonya_buffer->get_int( ) / 30.f ) + 0.2f )
			{
				v_evade_queue.erase( v_evade_queue.begin( ) + i );
				continue;
			}

#pragma region do_evade_missile
			if ( v_evade_queue [ i ].spell_name == "tower_shot" )
			{
				if ( item::evade_towershot->get_bool( ) && pred_health <= item::evade_towershot_hp->get_int( ) )
				{
					myhero->cast_spell( zhonya_item );
					return;
				}
				continue;
			}
#pragma endregion

#pragma region do_evade
			{
				if ( lowhp )
				{
					//..myhero->print_chat( 0x3, "evade_tick_low" );
					myhero->cast_spell( zhonya_item );
					return;
				}

				if ( !v_evade_queue [ i ].priority_entry.enabled || 
					!v_evade_queue [ i ].priority_entry.allies || 
					!v_evade_queue [ i ].priority_entry.enabled_hp || 
					!v_evade_queue [ i ].priority_entry.enemies )
				{
					//myhero->print_chat( 0x3, "no_ref_entry: %s", v_evade_queue [ i ].spell_name.c_str( ) );
					if ( lowhp )
					{
						myhero->cast_spell( zhonya_item );
						return;
					}
				}
				else
				{
					//myhero->print_chat( 0x3, "0ref: %s", v_evade_queue [ i ].spell_name.c_str( ) );
					if ( !v_evade_queue [ i ].priority_entry.enabled->get_bool( ) )
						continue;

					if ( v_evade_queue [ i ].priority_entry.enabled_hp->get_int( ) != 101 && pred_health > v_evade_queue [ i ].priority_entry.enabled_hp->get_int( ) )
						continue;

					//myhero->print_chat( 0x3, "1ref: %s", v_evade_queue [ i ].spell_name.c_str( ) );

					auto allow_logic_num = ( v_evade_queue [ i ].priority_entry.enemies->get_int( ) == 0 || myhero->count_enemies_in_range( 1250 ) >= v_evade_queue [ i ].priority_entry.enemies->get_int( ) ) &&
						( v_evade_queue [ i ].priority_entry.allies->get_int( ) == 0 || myhero->count_allies_in_range( 900.f ) >= v_evade_queue [ i ].priority_entry.allies->get_int( ) );

					auto allow_logic_inv = !myhero->is_invulnerable( ) && !myhero->has_buff( inv_shield_hashes );

					if ( allow_logic_num && allow_logic_inv )
					{
						//myhero->print_chat( 0x3, "evade_tick" );
						myhero->cast_spell( zhonya_item );
						return;
					}
				}
			}
#pragma endregion
		}
#pragma endregion
	}

	//挺進
	void use_stridebreaker( )
	{
		if ( !item::stridebreaker->get_bool( ) || !myhero->can_move( ) )
			return;

		Item stridebreaker( { ItemId::Stridebreaker, ItemId::Dreamshatter, ItemId::Stridebreaker_Arena, ItemId::Dreamshatter_Arena }, 400 );
		if ( stridebreaker.is_ready( ) )
		{
			auto enemy = entitylist->get_enemy_heroes( );
			enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), [ & ]( game_object_script x )
			{
				return !x->is_valid( ) || x->is_dead( ) || !x->is_valid_target( stridebreaker._range ) || !A_::checkbox( item::stridebreaker_always_use_on, x );
			} ), enemy.end( ) );

			if ( !enemy.empty( ) )
			{
				for ( auto& enemy : enemy )
				{
					if ( enemy->get_move_speed( ) > 200 )
					{
						stridebreaker.cast( );
						return;
					}
				}
			}

			auto enemies_count = myhero->get_position( ).count_enemies_in_range( 400 );

			bool combo_count = orbwalker->combo_mode( ) && enemies_count >= item::stridebreaker_combo_count_hero->get_int( );
			bool flee_count = orbwalker->flee_mode( ) && item::stridebreaker_flee->get_bool( ) && enemies_count > 0;
			bool harass_hit = orbwalker->mixed_mode( ) && item::stridebreaker_harass->get_bool( ) && enemies_count != 0;
			bool laneclear_hit = orbwalker->lane_clear_mode( ) && myhero->count_enemies_in_range( 1300 ) == 0 &&
				( A_::count_minions_in_range( stridebreaker._range ) >= item::stridebreaker_lane_clear->get_int( ) ||
					A_::count_monsters_in_range( stridebreaker._range ) >= item::stridebreaker_jungle_clear->get_int( ) );
			bool fast_lane_clear_hit = orbwalker->lane_clear_mode( ) && ( orbwalker->get_orb_state( ) == ( orbwalker_state_flags::fast_lane_clear | orbwalker_state_flags::lane_clear ) ) &&
				item::goredrinker_fast_lane_clear->get_bool( ) &&
				( A_::count_minions_in_range( stridebreaker._range ) >= 1 || A_::count_monsters_in_range( stridebreaker._range ) >= 1 );

			if ( combo_count || harass_hit || laneclear_hit || fast_lane_clear_hit || flee_count )
			{
				stridebreaker.cast( );
				return;
			}
		}
	}

	//渴血
	void use_goredrinker( )
	{
		if ( !item::goredrinker->get_bool( ) || !myhero->can_move( ) )
			return;

		Item goredrinker( { ItemId::Goredrinker, ItemId::Ceaseless_Hunger, ItemId::Goredrinker_Arena, ItemId::Ceaseless_Hunger_Arena }, 400 );
		if ( goredrinker.is_ready( ) )
		{
			bool auto_cast = myhero->get_position( ).count_enemies_in_range( 440 ) >= item::goredrinker_auto_use->get_int( );
			bool combo_count = orbwalker->combo_mode( ) && myhero->get_position( ).count_enemies_in_range( 400 ) >= item::goredrinker_combo_count_hero->get_int( );
			bool combo_hp = orbwalker->combo_mode( ) && myhero->count_enemies_in_range( 400 ) != 0 && myhero->get_health_percent( ) <= item::goredrinker_combo_hp->get_int( );
			bool harass_hit = orbwalker->mixed_mode( ) && item::goredrinker_harass->get_bool( ) && myhero->count_enemies_in_range( 400 ) != 0;
			bool laneclear_hit = orbwalker->lane_clear_mode( ) && myhero->count_enemies_in_range( 1300 ) == 0 &&
				( A_::count_minions_in_range( goredrinker._range ) >= item::goredrinker_lane_clear->get_int( ) ||
					A_::count_monsters_in_range( goredrinker._range ) >= item::goredrinker_jungle_clear->get_int( ) );
			bool fast_lane_clear_hit = orbwalker->lane_clear_mode( ) && ( orbwalker->get_orb_state( ) & orbwalker_state_flags::fast_lane_clear ) &&
				item::goredrinker_fast_lane_clear->get_bool( ) &&
				( A_::count_minions_in_range( goredrinker._range ) >= 1 || A_::count_monsters_in_range( goredrinker._range ) >= 1 );

			if ( auto_cast || combo_count || combo_hp || harass_hit || laneclear_hit || fast_lane_clear_hit )
			{
				goredrinker.cast( );
				return;
			}
		}
	}

	//石像鬼
	void use_gargoyle_stoneplate( )
	{
		if ( item::gargoyle_stoneplate->get_bool( ) && !myhero->is_invulnerable( ) && !myhero->has_buff( buff_hash( "alphastrike" ) ) )
		{
			Item gargoyle_stoneplate( { ItemId::Gargoyle_Stoneplate, ItemId::Gargoyle_Stoneplate_Arena }, 0 );
			if ( gargoyle_stoneplate.is_ready( ) )
			{
				if ( myhero->get_health( ) - health_prediction->get_incoming_damage( myhero, 0.5f, false ) <= 0.0f )
				{
					gargoyle_stoneplate.cast( );
				}
				if ( myhero->get_health_percent( ) <= item::gargoyle_stoneplate_hp->get_int( ) )
				{
					if ( myhero->get_position( ).count_enemies_in_range( 600 ) >= 1 )
					{
						gargoyle_stoneplate.cast( );
					}
				}
				if ( myhero->get_health_percent( ) <= item::gargoyle_stoneplate_damage_hp->get_int( ) )
				{
					gargoyle_stoneplate_shield = ( myhero->get_max_health( ) - myhero->get_base_hp( ) - myhero->get_hpPerLevel( ) * ( myhero->get_level( ) - 1 ) ) < 0 ? 100 : ( 100 + myhero->get_max_health( ) - myhero->get_base_hp( ) - myhero->get_hpPerLevel( ) * ( myhero->get_level( ) - 1 ) );
					if ( health_prediction->get_incoming_damage( myhero, 0.5f, false ) >= gargoyle_stoneplate_shield * item::gargoyle_stoneplate_damage->get_int( ) / 100.0f )
					{
						gargoyle_stoneplate.cast( );
					}
				}
			}
		}
	}

	//蘭頓
	void use_randuins( )
	{
		if ( item::randuins->get_bool( ) )
		{
			Item randuins( { ItemId::Randuins_Omen, ItemId::Randuins_Omen_Arena }, 450 );
			if ( randuins.is_ready( ) )
			{
				auto enemy = entitylist->get_enemy_heroes( );

				enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), [ ]( game_object_script x )
				{
					return !x->is_valid( ) || x->is_dead( ) || !x->is_valid_target( 500 ) || !A_::checkbox( item::randuins_always_use_on, x );
				} ), enemy.end( ) );

				if ( !enemy.empty( ) )
				{
					for ( auto& enemy : enemy )
					{
						if ( enemy->get_move_speed( ) > 200 )
						{
							randuins.cast( );
						}
					}
				}
			}
		}
	}

	//心之刚
	void use_heartsteel( )
	{
		Item heartsteel( { ItemId::Heartsteel, ItemId::Heartsteel_Arena }, 0 );
		if ( item::use_heartsteel->get_bool( ) && heartsteel.is_vaild( ) )
		{
			auto enemy = entitylist->get_enemy_heroes( );

			enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), [ ]( game_object_script x )
			{
				if ( !x->is_valid( ) )
					return true;

				auto buff = x->get_buff( buff_hash( "3084proctracker" ) );

				return x->is_dead( ) || !myhero->is_in_auto_attack_range( x ) || buff == nullptr || !buff->is_valid( ) || !buff->is_alive( ) ||
					abs( 25000 - buff->get_remaining_time( ) ) < 2.5f;
			} ), enemy.end( ) );

			std::sort( enemy.begin( ), enemy.end( ), [ ]( game_object_script a, game_object_script b )
			{
				return a->get_health( ) < b->get_health( );
			} );

			orbwalker->set_orbwalking_target( enemy.empty( ) ? 0 : enemy.front( ) );
		}
	}

#define ITEM_EVERFROST_RANGE 925.f
	void use_everfrost( )
	{
		if ( !item::everfrost->get_bool( ) )
			return;

		if ( !myhero->can_move( ) )
			return;

		auto item = myhero->has_item( { ItemId::Everfrost, ItemId::Eternal_Winter, ItemId::Everfrost_Arena, ItemId::Eternal_Winter_Arena } );
		if ( item == spellslot::invalid )
			return;

		if ( myhero->get_spell_state( item ) != spell_state::Ready )
			return;

		auto flee = orbwalker->flee_mode( ) && item::everfrost_flee->get_bool( );
		if ( ( orbwalker->combo_mode( ) && item::everfrost_combo->get_bool( ) ) ||
			( orbwalker->harass( ) && item::everfrost_harass->get_bool( ) ) ||
			flee )
		{
			auto enemies = entitylist->get_enemy_heroes( );
			enemies.erase( std::remove_if( enemies.begin( ), enemies.end( ), [ ]( game_object_script x ) {
				if ( !x || !x->is_valid_target( ITEM_EVERFROST_RANGE * item::everfrost_range->get_int( ) / 100.f, vector::zero, true ) )
					return true;

				if ( A_::is_spell_immune( x ) )
					return true;

				if ( A_::is_invulnerable( x, std::max( 0.f, 0.3f - ping->get_ping( ) / 1000.f ), false ) || !A_::can_be_killed( x, true ) )
					return true;

				return false;
			} ), enemies.end( ) );

			if ( !enemies.empty( ) )
			{
				if ( flee )
				{
					std::sort( enemies.begin( ), enemies.end( ), [ v = myhero->get_position( ) ]( game_object_script x1, game_object_script x2 ) {
						return x1->get_distance( v ) < x2->get_distance( v );
					} );
				}
				auto target = flee ? enemies.front( ) : target_selector->get_target( enemies, damage_type::magical );
				if ( target && target->is_valid( ) )
				{
					if ( target->get_immovibility_time( ) > 0.3f - ping->get_ping( ) / 1000.f )
						return;

					//40-60
					prediction_input input;
					input.unit = target;
					input.delay = 0.3f;
					input.collision_objects = { };
					input.radius = 60.f;
					input.use_bounding_radius = false;
					input.range = ITEM_EVERFROST_RANGE;

					auto predict = prediction->get_prediction( &input );
					if ( ( int ) predict.hitchance >= item::hitchance_everfrost->get_int( ) )
					{
						myhero->cast_spell( item, predict.get_cast_position( ) );
						return;
					}
				}
			}
		}
	}

	//potion
	void red_potion( )
	{
		auto slot = myhero->has_item( ItemId::Health_Potion );
		if ( slot == spellslot::invalid )return;
		if ( !myhero->is_item_ready( ItemId::Health_Potion ) )return;
		if ( myhero->has_buff( buff_hash( "item2003" ) ) )return;

		bool red_p = potions::Health_Potion->get_bool( );
		if ( !red_p ) return;

		int red_hp = potions::Health_Potion_hp->get_int( );
		if ( myhero->get_health_percent( ) > red_hp ) return;

		bool red_c = potions::Health_Potion_c->get_bool( ) && orbwalker->combo_mode( );
		bool red_h = potions::Health_Potion_harass->get_bool( ) && orbwalker->mixed_mode( );
		bool red_other = potions::Health_Potion_other->get_bool( ) && !orbwalker->mixed_mode( ) && !orbwalker->combo_mode( );
		if ( !red_c && !red_h && !red_other ) return;

		if ( myhero->get_level( ) < 2 ) return;
		if ( myhero->count_enemies_in_range( static_cast<float>( potions::Health_Potion_rang->get_int( ) ) ) >= potions::Health_Potion_enemy->get_int( ) )
		{
			myhero->cast_spell( slot );
			return;
		}
	}

	void green_potion( )
	{
		auto slot = myhero->has_item( ItemId::Refillable_Potion );
		if ( slot == spellslot::invalid )return;
		if ( !myhero->is_item_ready( ItemId::Refillable_Potion ) )return;
		if ( myhero->has_buff( buff_hash( "ItemCrystalFlask" ) ) )return;

		bool green_p = potions::Refillable_Potion->get_bool( );
		if ( !green_p ) return;

		int green_hp = potions::Refillable_Potion_hp->get_int( );
		if ( myhero->get_health_percent( ) > green_hp ) return;

		bool green_c = potions::Refillable_Potion_c->get_bool( ) && orbwalker->combo_mode( );
		bool green_h = potions::Refillable_Potion_harass->get_bool( ) && orbwalker->mixed_mode( );
		bool green_other = potions::Refillable_Potion_other->get_bool( ) && !orbwalker->mixed_mode( ) && !orbwalker->combo_mode( );
		if ( !green_c && !green_h && !green_other ) return;

		if ( myhero->get_level( ) < 2 ) return;
		if ( myhero->count_enemies_in_range( static_cast<float>( potions::Refillable_Potion_rang->get_int( ) ) ) >= potions::Refillable_Potion_enemy->get_int( ) )
		{
			myhero->cast_spell( slot );
			return;
		}
	}

	void big_potion( )
	{
		auto slot = myhero->has_item( ItemId::Corrupting_Potion );
		if ( slot == spellslot::invalid )return;
		if ( !myhero->is_item_ready( ItemId::Corrupting_Potion ) )return;
		if ( myhero->has_buff( buff_hash( "ItemDarkCrystalFlask" ) ) )return;

		bool big_p = potions::Corrupting_Potion->get_bool( );
		if ( !big_p ) return;

		int big_hp = potions::Corrupting_Potion_hp->get_int( );
		if ( myhero->get_health_percent( ) > big_hp ) return;

		bool big_c = potions::Corrupting_Potion_c->get_bool( ) && orbwalker->combo_mode( );
		bool big_h = potions::Corrupting_Potion_harass->get_bool( ) && orbwalker->mixed_mode( );
		bool big_other = potions::Corrupting_Potion_other->get_bool( ) && !orbwalker->mixed_mode( ) && !orbwalker->combo_mode( );
		if ( !big_c && !big_h && !big_other ) return;

		if ( myhero->get_level( ) < 2 ) return;
		if ( myhero->count_enemies_in_range( static_cast<float>( potions::Corrupting_Potion_rang->get_int( ) ) ) >= potions::Corrupting_Potion_enemy->get_int( ) )
		{
			myhero->cast_spell( slot );
			return;
		}
	}

	void biscuit_potion( )
	{
		auto slot = myhero->has_item( ItemId::Total_Biscuit_of_Everlasting_Will );
		if ( slot == spellslot::invalid )return;
		if ( !myhero->is_item_ready( ItemId::Total_Biscuit_of_Everlasting_Will ) )return;
		if ( myhero->has_buff( buff_hash( "Item2010" ) ) )return;

		bool biscuit_p = potions::Biscuit->get_bool( );
		if ( !biscuit_p ) return;

		int biscuit_hp = potions::Biscuit_hp->get_int( );
		if ( myhero->get_health_percent( ) > biscuit_hp ) return;

		bool biscuit_c = potions::Biscuit_c->get_bool( ) && orbwalker->combo_mode( );
		bool biscuit_h = potions::Biscuit_harass->get_bool( ) && orbwalker->mixed_mode( );
		bool biscuit_other = potions::Biscuit_other->get_bool( ) && !orbwalker->mixed_mode( ) && !orbwalker->combo_mode( );
		if ( !biscuit_c && !biscuit_h && !biscuit_other ) return;

		if ( myhero->get_level( ) < 2 ) return;
		if ( myhero->count_enemies_in_range( static_cast<float>( potions::Biscuit_rang->get_int( ) ) ) >= potions::Biscuit_enemy->get_int( ) )
		{
			myhero->cast_spell( slot );
			return;
		}
	}
#pragma endregion 

	//ward
	void use_buy_ward( )
	{
		static auto b_purchased = false;
		static auto base = vector::zero;

		if ( !base.is_valid( ) )
		{
			auto spawns = entitylist->get_all_spawnpoints( );
			if ( spawns.empty( ) )
				return;

			for ( auto home : spawns )
			{
				if ( home && home->is_valid( ) && home->is_ally( ) )
					base = home->get_position( );
			}
			
		}

		if ( !base.is_valid( ) || !myhero->can_move( ) )
			return;

		if ( myhero->get_distance( base ) > 500 )
			return;

		if ( b_purchased )
			return;

		if ( item::buy_ward->get_int( ) != 0 && myhero->get_level( ) >= item::buy_ward_lv->get_int( ) )
		{
			if ( item::buy_ward->get_int( ) == 1 )
			{
				myhero->buy_item( ItemId::Oracle_Lens );
				b_purchased = true;
			}
			if ( item::buy_ward->get_int( ) == 2 )
			{
				myhero->buy_item( ItemId::Farsight_Alteration );
				b_purchased = true;
			}
		}

		if ( item::sup_buy_ward->get_int( ) != 0 )
		{
			auto sup_slot1 = myhero->has_item( {
				ItemId::Targons_Buckler, ItemId::Bulwark_of_the_Mountain,
				ItemId::Frostfang, ItemId::Shard_of_True_Ice,
				ItemId::Steel_Shoulderguards, ItemId::Runesteel_Spaulders } );
			if ( sup_slot1 != spellslot::invalid )
			{
				if ( item::sup_buy_ward->get_int( ) == 1 )
				{
					myhero->buy_item( ItemId::Oracle_Lens );
					b_purchased = true;
				}
				if ( item::sup_buy_ward->get_int( ) == 2 )
				{
					myhero->buy_item( ItemId::Farsight_Alteration );
					b_purchased = true;
				}
			}
		}
	}

	float get_global_level( )
	{
		float lvl = 0;
		int amount = 0;
		for ( auto&& hero : entitylist->get_all_heroes( ) )
		{
			if ( hero && hero->is_valid( ) )
			{
				lvl += hero->get_level( );
				amount++;
			}
		}
		return ( lvl / amount );
	}

	//draw
	auto register_trap( game_object_script sender ) -> void
	{
		if ( sender->is_ally( ) )
			return;

		auto model_hash = buff_hash_real( sender->get_model_cstr( ) );
		auto it_trap = m_trap_info.find( model_hash );

		if ( it_trap == m_trap_info.end( ) )
			return;

		auto it_existing = std::find_if( m_traps.begin( ), m_traps.end( ), [ & ]( s_ward trap ) {
			return trap.network_id == sender->get_network_id( );
		} );
		if ( it_existing != m_traps.end( ) )
			return;
		
		s_ward trap;
		trap.position = sender->get_position( );
		trap.icon.first = sender->get_square_icon_portrait( );
		trap.network_id = sender->get_network_id( );
		trap.is_enemy = sender->is_enemy( );
		trap.range = it_trap->second.range;

		if ( sender->get_owner( ) && sender->get_owner( )->is_valid( ) )
			trap.network_id_owner = sender->get_owner( )->get_network_id( );

		trap.model_hash = model_hash;

		m_traps.push_back( trap );
		apply_glow_trap( sender, true );
	}

	////物件
	void on_create_object( game_object_script sender ) // KatarinaRMis|2400.00
	{
		if ( !sender || !sender->is_valid( ) || !sender->get_position( ).is_valid( ) || sender->is_ai_hero( ) )
			return;

		if ( sender->get_emitter_resources_hash( ) == buff_hash( "SharedWardTracker_Pingable" ) )
		{
			if ( autoping::wards->get_bool( ) && ( A_::is_on_screen( sender->get_position( ) ) || !autoping::only_on_screen->get_bool( ) ) )
			{
				auto delay = A_::random_float( autoping::delay_1->get_int( ) / 1000.f, autoping::delay_2->get_int( ) / 1000.f );
				scheduler->delay_action( delay, [ v = sender->get_position( ) ]( ) 
				{
					auto pos = autoping::randomize_pos->get_int( ) > 0 ? A_::randomize( v, 0.f, static_cast<float>( autoping::randomize_pos->get_int( ) ) ) : v;

					myhero->cast_ping( pos, nullptr, _player_ping_type::area_is_warded );
				} );
			}
		}

		auto object_hash = buff_hash_real( sender->get_name_cstr( ) );
		
		register_trap( sender );
		switch ( object_hash )
		{
			case buff_hash( "VisionWard" ):
			case buff_hash( "SightWard" ):
			case buff_hash( "JammerDevice" ):
			{
				auto range = 0.f;
				if ( object_hash == buff_hash( "VisionWard" ) || ( object_hash == buff_hash( "SightWard" ) && sender->get_max_health( ) != 1 ) || object_hash == buff_hash( "JammerDevice" ) )
					range = 900.0f;
				else if ( object_hash == buff_hash( "SightWard" ) && sender->get_max_health( ) == 1 )
					range = 500.0f;

				auto id = -1;
				if ( sender->get_max_health( ) >= 4 )
					id = ( int )ItemId::Control_Ward;
				else if ( sender->get_max_health( ) == 3 )
					id = ( int )ItemId::Stealth_Ward;
				else
					id = ( int )ItemId::Farsight_Alteration;

				/*switch ( object_hash )
				{
					case buff_hash( "SightWard" ):
						id = ( int )ItemId::Stealth_Ward;
						break;
					case buff_hash( "JammerDevice" ):
						id = ( int )ItemId::Control_Ward;
						break;
					case buff_hash( "VisionWard" ):
						id = ( int )ItemId::Farsight_Alteration;
						break;
				}*/

				auto t_create = gametime->get_time( );
				auto t_life_span = 90.f + ( 30.f / 17.f ) * ( get_global_level( ) - 1.f );
				auto it_existing = std::find_if( m_wards.begin( ), m_wards.end( ), [ & ]( std::pair< uint32_t, s_ward > p )
				{
					return sender->get_network_id( ) == p.first;
				} );

				if ( it_existing == m_wards.end( ) )
				{
					auto owner = sender->get_owner( );
					s_ward ward;
					ward.position = sender->get_position( ),
					ward.range = range;
					ward.points = { };
					ward.polygon = { };
					ward.t_create = sender->get_max_health( ) != 3 ? -1.f : t_create;
					ward.t_life_span = sender->get_max_health( ) != 3 ? -1.f : t_life_span;
					ward.initialized = false;
					ward.is_enemy = sender->is_enemy( );

					ward.icon = { nullptr, { 0, 0, 0, 0 } };
					ward.network_id_owner = owner ? owner->get_network_id( ) : 0;
					ward.network_id = sender->get_network_id( );

					if ( id != 1 )
					{
						auto dbitem = database->get_item_by_id( ( ItemId )id );
						if ( dbitem->get_texture( ).first )
							ward.icon = dbitem->get_texture( );
					}

					m_wards.push_back( { sender->get_network_id( ), ward } );
				}
				else
				{
					it_existing->second.t_create = t_create;
					it_existing->second.t_life_span = t_life_span;
					it_existing->second.is_enemy = sender->is_enemy( );
				}

				break;
			}

			default:
				break;
		}
	}

	void on_delete_object( game_object_script sender )
	{
		if ( !sender || !sender->is_valid( ) )
			return;

		auto it_ward = std::find_if( m_wards.begin( ), m_wards.end( ), [ & ]( std::pair< uint32_t, s_ward > p )
		{
			return sender->get_network_id( ) == p.first;
		} );

		if ( it_ward != m_wards.end( ) )
			it_ward->second.t_create = 1.f;

		auto it_trap = std::find_if( m_traps.begin( ), m_traps.end( ), [ & ]( s_ward w ) {
			return w.network_id == sender->get_network_id( );
		} );

		if ( it_trap != m_traps.end( ) )
		{
			it_trap->t_create = 1.f;
			if ( glow->is_glowed( sender ) )
				glow->remove_glow( sender );
		}
	}

	//cast
	auto on_process_spell_cast( game_object_script sender, spell_instance_script spell ) -> void
	{
		if ( !sender || !sender->is_valid( ) || myhero->is_dead( ) || !sender->is_enemy( ) || !sender->is_ai_hero( ) || !spell || !spell->get_spell_data( ) )
			return;

		auto cid = sender->get_champion( );
		auto spell_name = spell->get_spell_data( )->get_name( );
		auto spell_name_lower = spell_name;
		std::transform( spell_name_lower.begin( ), spell_name_lower.end( ), spell_name_lower.begin( ),
			[ ]( unsigned char c ) { return std::tolower( c ); } );
		auto is_special_attack = ( spell->is_auto_attack( ) && spell_name_lower.find( "basicattack" ) == std::string::npos &&
			spell_name_lower.find( "critattack" ) == std::string::npos );

		auto find_result = std::find_if( z_evade_whitelist.begin( ), z_evade_whitelist.end( ), [ is_special_attack, spell_name, cid ]( const auto& pair )
		{
			if ( is_special_attack )
				return pair.first == cid;
			else
			{
				auto it = std::find_if( pair.second.begin( ), pair.second.end( ), [ spell_name ]( const auto& std_pair ) { return std_pair.first == spell_name; } );
				return it != pair.second.end( );
			}
		} );
		if ( find_result == z_evade_whitelist.end( ) )
			return;

		auto&& config_array = find_result->second;

		//activate_spell_data
		if ( !spell->is_auto_attack( ) )
		{
			for ( auto&& spell_data_targeted : spelldb_activate )
			{
				for ( auto&& supported_spell : spell_data_targeted.second )
				{
					for ( auto&& name_option : supported_spell.spell_names )
					{
						if ( name_option != spell_name )
							continue;

						for ( auto&& pair : config_array )
						{
							if ( pair.first != spell_name )
								continue;

							if ( supported_spell.buff != 0 )
							{
								auto buff = supported_spell.buff_holder_enemy ? sender->get_buff( supported_spell.buff ) : myhero->get_buff( supported_spell.buff );
								if ( !buff || !buff->is_valid( ) )
									continue;

								if ( supported_spell.buff_stacks != 0 && buff->get_count( ) < supported_spell.buff_stacks )
									continue;

								if ( sender->get_distance( myhero ) > supported_spell.range )
									continue;
							}
							auto t_hit = std::max( gametime->get_time( ),
								gametime->get_time( ) + supported_spell.delay - ( item::zhonya_buffer->get_int( ) / 30.f ) );
							v_evade_queue.push_back( evade_queue_entry { "caster_name", spell_name, pair.second, false, false, 0, t_hit } );
							return;
#ifdef __TEST
							console->print( "[sc]detected activate_spell_data->supported | name: [%s] | caster: [%s] %.1f", spell_name.c_str( ), "caster_name", t_hit - gametime->get_time( ) );
#endif
						}
					}
				}
			}
		}

		if ( spell->get_last_target_id( ) != myhero->get_id( ) )
			return;

		//targeted
		if ( !spell->is_auto_attack( ) )
		{
			for ( auto&& spell_data : spelldb )
			{
				for ( auto&& supported_spell : spell_data.second )
				{
					if ( !supported_spell.targeted )
						continue;

					for ( auto&& name_option : supported_spell.spell_names )
					{
						if ( name_option != spell_name )
							continue;

						for ( auto&& pair : config_array )
						{
							if ( pair.first != spell_name )
								continue;

							if ( supported_spell.buff_enemy > 0 && !sender->has_buff( supported_spell.buff_enemy ) )
								continue;

							if ( supported_spell.buff_me && !myhero->has_buff( supported_spell.buff_me ) )
								continue;

							if ( supported_spell.track_only_buff )
								continue;

							if ( supported_spell.missile && !supported_spell.missile_hashes.empty( ) ) //ignore on_proc, detect via on_create
								continue;

							auto t_hit = std::max( gametime->get_time( ),
								gametime->get_time( ) + supported_spell.spellDelay +
								( supported_spell.missile ? spell->get_start_position( ).distance( spell->get_end_position( ) ) / supported_spell.spellSpeed : 0.f ) -
								( item::zhonya_buffer->get_int( ) / 30.f ) );
							v_evade_queue.push_back( evade_queue_entry { "caster_name", spell_name, pair.second, false, false, 0, t_hit } );
#ifdef __TEST
							console->print( "[sc]detected spelldata->supported | name: [%s] | caster: [%s] %.1f", spell_name.c_str( ), "caster_name", t_hit - gametime->get_time( ) );
#endif
							return;
						}
					}
				}
			}
		}

		//attack
		if ( is_special_attack )
		{
#ifdef __TEST
			console->print( "[sc]detected special atk: %s | %s", spell_name_lower.c_str( ), "caster_name" );
#endif
			for ( auto&& pair : config_array )
			{
				if ( pair.first != "special_attack" )
					continue;

				auto t_hit = std::max( gametime->get_time( ), gametime->get_time( ) + spell->get_attack_cast_delay( ) - ( item::zhonya_buffer->get_int( ) / 30.f ) );
				v_evade_queue.push_back( evade_queue_entry { "caster_name", spell->get_spell_data( )->get_name( ), pair.second, true, false, 0, t_hit } );
#ifdef __TEST
				console->print( "[sc]detected special atk | name: [%s] | caster: [%s]", spell_name_lower.c_str( ), "caster_name" );
				console->print( "[sc]t hit: %.1f", t_hit - gametime->get_time( ) );
#endif
				return;

			}
		}

		auto pred_health_raw = health_prediction->get_health_prediction( myhero, 0.33f, 0.f );
		auto pred_health = pred_health_raw / myhero->get_max_health( ) * 100.f;
		if ( item::evade_force->get_int( ) != -1 && pred_health <= item::evade_force->get_int( ) && ( sender->is_ai_hero( ) || sender->is_ai_turret( ) ) )
		{
			/*
				struct evade_queue_entry
				{
					std::string caster_name;
					std::string spell_name;
					s_evade_config priority_entry;

					bool is_special_attack;
					bool is_activate_spell;

					uint32_t buff_hash;

					float time;
					spellslot evade_method = spellslot::invalid;
					uint32_t missile_network_id = 0;
				};		
			*/
			auto t_hit = std::max( gametime->get_time( ), gametime->get_time( ) + spell->get_attack_cast_delay( ) - ( item::zhonya_buffer->get_int( ) / 30.f ) );
			v_evade_queue.push_back( evade_queue_entry { "caster_name", "[force]" + spell->get_spell_data( )->get_name( ), s_evade_config{ nullptr, nullptr, nullptr, nullptr }, true, false, 0, t_hit } );
		}
	}

	//After AA
	void on_after_attack_orbwalker( game_object_script target )
	{
		if ( !target || !target->is_valid( ) )
			return;

		//myhero->print_chat(1, "Q:%f, W:%f, E:%f, R:%f", q->get_damage(target), w->get_damage(target), e->get_damage(target), r->get_damage(target));
		Item ironspike( ItemId::Ironspike_Whip, 400 );
		Item goredrinker( { ItemId::Goredrinker, ItemId::Ceaseless_Hunger, ItemId::Goredrinker_Arena, ItemId::Ceaseless_Hunger_Arena }, 400 );
		Item stridebreaker( { ItemId::Stridebreaker, ItemId::Stridebreaker_Arena, ItemId::Dreamshatter, ItemId::Dreamshatter_Arena }, 400 );

		if ( target->is_ai_hero( ) && orbwalker->combo_mode( ) )
		{
			//鐵刺
			if ( ironspike.is_ready( ) && item::ironspike->get_bool( ) && item::ironspike_combo->get_bool( ) )
			{
				auto target = target_selector->get_target( 400, damage_type::true_dmg );
				if ( target != nullptr )
				{
					ironspike.cast( );
				}
			}
			//挺進
			if ( stridebreaker.is_ready( ) && item::stridebreaker->get_bool( ) )
			{
				if ( myhero->get_position( ).count_enemies_in_range( 400 ) >= item::stridebreaker_combo_count_hero->get_int( ) )
				{
					stridebreaker.cast( );
				}
			}
			//渴血
			if ( goredrinker.is_ready( ) && item::goredrinker->get_bool( ) )
			{
				if ( myhero->get_position( ).count_enemies_in_range( 400 ) >= item::goredrinker_combo_count_hero->get_int( ) )
				{
					goredrinker.cast( );
				}
				if ( myhero->get_health_percent( ) <= item::goredrinker_combo_hp->get_int( ) )
				{
					auto target = target_selector->get_target( 400, damage_type::true_dmg );
					if ( target != nullptr )
					{
						goredrinker.cast( );
					}
				}
			}

		}
		if ( ( target->is_monster( ) || target->is_minion( ) ) && orbwalker->lane_clear_mode( ) && !target->is_plant( ) )
		{
			if ( ironspike.is_ready( ) && item::ironspike->get_bool( ) )
			{
				if ( A_::count_minions_in_range( ironspike._range ) >= item::ironspike_lane_clear_reset_aa->get_int( ) )
				{
					ironspike.cast( );
					return;
				}
				if ( A_::count_monsters_in_range( ironspike._range ) >= item::ironspike_jungle_clear_reset_aa->get_int( ) )
				{
					ironspike.cast( );
					return;
				}
			}
			if ( goredrinker.is_ready( ) && item::goredrinker->get_bool( ) )
			{
				if ( A_::count_minions_in_range( goredrinker._range ) >= item::goredrinker_lane_clear_reset_aa->get_int( ) )
				{
					goredrinker.cast( );
					return;
				}
				if ( A_::count_monsters_in_range( goredrinker._range ) >= item::goredrinker_jungle_clear_reset_aa->get_int( ) )
				{
					goredrinker.cast( );
					return;
				}
			}
			if ( stridebreaker.is_ready( ) && item::stridebreaker->get_bool( ) )
			{
				if ( A_::count_minions_in_range( stridebreaker._range ) >= item::stridebreaker_lane_clear_reset_aa->get_int( ) )
				{
					stridebreaker.cast( );
					return;
				}
				if ( A_::count_monsters_in_range( stridebreaker._range ) >= item::stridebreaker_jungle_clear_reset_aa->get_int( ) )
				{
					stridebreaker.cast( );
					return;
				}
			}
		}
	}

	//
	void on_issue_order( game_object_script& target, vector& pos, _issue_order_type& type, bool* process )
	{
		g_wards->on_issue_order( type );
	}

	//
	void new_path( game_object_script sender, const std::vector<vector>& path, bool is_dash, float dash_speed )
	{
	}
	//

	auto auto_ping_enemies( ) -> void
	{
		if ( !autoping::enemies->get_bool( ) )
			return;

		static auto t_last_t = -20.f;
		if ( gametime->get_time( ) - t_last_t <= 0.1f )
			return;

		t_last_t = gametime->get_time( );

		static auto t_last_ping = -20.f;
		if ( gametime->get_time( ) - t_last_ping <= 1.0f )
			return;

		static std::map<uint32_t, vector> m_last_pings = { };

		for ( auto&& x : entitylist->get_enemy_heroes( ) )
		{
			if ( !x || !x->is_valid( ) || x->is_visible( ) || x->is_dead( ) || ( autoping::only_jungler->get_bool( ) && !A_::is_jungler( x ) ) )
				continue;

			auto tracker_data = fow_tracker->get_data( x->get_network_id( ) );
			if ( gametime->get_time( ) - tracker_data.t_last_update_particle >= 2.f )
				continue;

			auto it_last = m_last_pings.find( x->get_network_id( ) );
			if ( it_last == m_last_pings.end( ) )
			{
				m_last_pings [ x->get_network_id( ) ] = vector::zero;
				continue;
			}

			if ( !it_last->second.is_valid( ) || it_last->second.distance( tracker_data.position ) > 175.f )
			{
				myhero->cast_ping( tracker_data.position, nullptr, _player_ping_type::danger );
				t_last_ping = gametime->get_time( );
				it_last->second = tracker_data.position;
				return;
			}
		}
	}

	void on_update_highest( )
	{
		summoner_spell::all_logic( );

		auto_ping_enemies( );
	}

	struct s_player_info
	{
		float last_visible = -20.f;
		vector last_direction = vector::zero;
		vector last_position = vector::zero;

		float ms;
	};

	auto auto_ward_bush( ) -> void
	{
		
	}

	auto use_gunblade( ) -> void
	{
		auto item = myhero->has_item( { ItemId::Hextech_Gunblade, static_cast<ItemId>( static_cast<int>( ItemId::Hextech_Gunblade ) + 220000 ) } );
		if ( item == spellslot::invalid )
			return;

		if ( myhero->get_spell_state( item ) != spell_state::Ready )
			return;

		auto use_any = ( item::semi_gunblade->get_bool( ) ) || ( orbwalker->flee_mode( ) && item::flee_gunblade->get_bool( ) );
		auto target = target_selector->get_target( 695.f, damage_type::magical );
		if ( !target || !target->is_valid_target( 695.f ) || !A_::can_be_killed( target, true ) )
			return;

		auto is_lethal = [ & ]( ) {
			auto raw_magical = 175.f + 78.f / 17.f * std::clamp( myhero->get_level( ) - 1, 1, 18 );
			raw_magical += 0.3f * myhero->get_total_ability_power( );

			return target->get_real_health( false, true ) - damagelib->calculate_damage_on_unit( myhero, target, damage_type::magical, raw_magical ) < 0.f;
		};

		if ( use_any || ( item::gunblade->get_bool( ) && is_lethal( ) ) )
		{
			myhero->cast_spell( item, target, true, false );
			return;
		}
	}

	void on_update( )
	{
		g_wards->update_wardpoints( );
		champion::all_logic( );
		misc::all_logic( );

		if ( myhero->is_dead( ) || myhero->is_recalling( ) )
		{
			v_evade_queue.clear( );
			return;
		}

		g_wards->on_tick( );
		g_wards->auto_ward( item::auto_ward_mode->get_int( ), item::auto_ward_control->get_bool( ), item::auto_ward_stealth->get_bool( ),
			item::semi_auto_ward_key -> get_bool( ) );

		//spell_check( );
		red_potion( );
		green_potion( );
		big_potion( );
		biscuit_potion( );
		auto_ward_bush( );

		//chat
		//Hiden
		hidden( );

		//裝備
		use_gunblade( );
		use_zhonya( );
		use_everfrost( );
		gb_logic( );
		use_galeforce( );
		use_ironspike( );
		use_stridebreaker( );
		use_goredrinker( );
		use_gargoyle_stoneplate( );
		use_randuins( );
		use_heartsteel( );
		use_buy_ward( );
		use_solari_locket( );
		use_ward_bush( );
	}

	auto on_cast_spell( spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process ) -> void
	{
		g_wards->on_cast_spell( spell_slot, process, item::ward_cast_assist->get_bool( ) );
	}

	auto draw_wards( ) -> void
	{
		static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
		static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

		auto draw_ward_r = [ & ]( s_ward& m_ward ) -> void
		{
			if ( !m_ward.position.is_valid( ) || m_ward.range <= 0.f )
				return;

			auto size = vector( 30.f * item::draw_ward_sz->get_int( ) / 100.f, 30.f * item::draw_ward_sz->get_int( ) / 100.f );
			if ( !m_ward.initialized )
			{
				auto is_grass = m_ward.position.is_wall_of_grass( );
				auto ward_circle_base = geometry::geometry::circle_points( m_ward.position, 50, 360 / 10 );
				auto ward_circle_real = std::vector<vector> { };
				auto ward_polygon = geometry::polygon( );

				for ( auto&& p : ward_circle_base )
				{
					auto&& v_end_side = p.extend( m_ward.position, -m_ward.range + 50 );
					bool grass_break_point = false;
					for ( auto i = 0; i <= m_ward.range - 50; i += 10 )
					{
						auto scan_pos = p.extend( m_ward.position, static_cast< float >( -i ) );
						if ( is_grass && !scan_pos.is_wall_of_grass( ) )
							grass_break_point = true;

						if ( ( grass_break_point || !is_grass ) && scan_pos.is_wall_of_grass( ) )
						{
							v_end_side = scan_pos;
							//grass_break_point = false;
							break;
						}
						else if ( ( scan_pos.is_wall( ) && !scan_pos.is_building( ) ) || ( int )navmesh->get_collision_flag( scan_pos ) == 6336 )
						{
							v_end_side = scan_pos;
							break;
						}
					}

					ward_circle_real.push_back( v_end_side );
					ward_polygon.add( v_end_side );
				}

				ward_circle_real.push_back( ward_circle_real.front( ) );
				{
					m_ward.points = ward_circle_real;
					m_ward.polygon = ward_polygon;
					m_ward.initialized = true;
					//m_ward.is_enemy = pos->is_enemy( );
				}
			}

			auto color = m_ward.is_enemy && m_ward.polygon.is_inside( myhero->get_position( ) ) ? MAKE_COLOR( 255, 121, 121, 100 ) : MAKE_COLOR( 223, 249, 251, 100 );
			color = A_::set_alpha( color, static_cast< unsigned long >( A_::get_alpha( color ) * m_ward.fl_alpha ) );
			for ( auto i = 0; i < m_ward.points.size( ) - 1; i++ )
			{
				auto v_start = m_ward.points [ i ],
					v_end = m_ward.points [ i + 1 ];

				if ( v_start.is_valid( ) && v_end.is_valid( ) )
					draw_manager->add_line( v_start, v_end, color, 5.f );
			}

#pragma region minimap
			if ( item::draw_ward_minimap->get_bool( ) )
			{
				std::vector<vector> m_path = { };
				for ( auto i = 0; i < m_ward.points.size( ); i++ )
				{
					auto v_start = m_ward.points [ i ];

					if ( !gui->get_tactical_map( )->to_map_coord( v_start, v_start ) )
						continue;

					m_path.push_back( v_start );
				}

				for ( auto i = 0; i < m_path.size( ) - 1; i++ )
					draw_manager->add_line_on_screen( m_path [ i ], m_path [ i + 1 ], color, 2.f );

				/*if ( !m_path.empty( ) )
				{
					m_path.push_back( m_path.front( ) );
					draw_manager->add_path_filled_on_screen( m_path, MAKE_COLOR( 255, 255, 255, 100 ) );
				}*/

				/*if ( item::draw_ward_time_map->get_bool( ) && m_ward.is_enemy )
				{
					auto pos_ward = m_ward.position;
					if ( m_ward.icon.first && gui->get_tactical_map( )->to_map_coord( pos_ward, pos_ward ) )
					{
						auto sz_icon = size / 3.f;
						draw_manager->add_image( m_ward.icon.first, pos_ward - sz_icon/2.f, sz_icon, 90.f, { m_ward.icon.second.x, m_ward.icon.second.y }, { m_ward.icon.second.z, m_ward.icon.second.w },
							{ 1.F, 1.F, 1.F, m_ward.fl_alpha } );
						draw_manager->add_circle_on_screen( pos_ward, sz_icon.x / 2.f, A_::set_alpha( COLOR_DARK_GRAY, static_cast< int >( 255 * m_ward.fl_alpha ) ), 1.f );
					}
				}*/
			}
#pragma endregion

			if ( m_ward.points.front( ).is_valid( ) && m_ward.points.back( ).is_valid( ) )
				draw_manager->add_line( m_ward.points.front( ), m_ward.points.back( ), color, 4.f );

#pragma region draw_ward_remaining_time
			if ( ( item::draw_ward_time->get_bool( ) && m_ward.is_enemy ) || ( item::draw_ward_time_ally->get_bool( ) && !m_ward.is_enemy ) )
			{
				if ( gametime->get_time( ) - m_ward.t_last_anim_i >= 0.02f )
				{
					m_ward.fl_alpha_i = std::clamp( easing_fn_in( m_ward.fl_alpha_i ), 0.001f, 0.999f );
					m_ward.t_last_anim_i = gametime->get_time( );
				}
			}
			else
			{
				if ( gametime->get_time( ) - m_ward.t_last_anim_i >= 0.02f )
				{
					m_ward.fl_alpha_i = std::clamp( easing_fn_out( m_ward.fl_alpha_i ), 0.001f, 0.999f );
					m_ward.t_last_anim_i = gametime->get_time( );
				}
			}

			if ( m_ward.fl_alpha_i > 0.01f )
			{
				auto w2s = vector::zero;
				renderer->world_to_screen( m_ward.position, w2s );

				auto fl_alpha = std::min( m_ward.fl_alpha_i, m_ward.fl_alpha );
				auto draw_ward_time = [ fl_alpha ]( float pcent, float time, vector pos, vector size ) -> void {
					auto arc = A_::arc_points( pcent, pos, size.x / 2.f );

					auto clr = item::draw_ward_remaining_t_color->get_color( );
					clr = A_::set_alpha( clr, static_cast< unsigned long >( A_::get_alpha( clr ) * fl_alpha ) );
					if ( !arc.empty( ) )
					{
						for ( auto i = 0; i < arc.size( ) - 1; i++ )
							draw_manager->add_line_on_screen( arc [ i ], arc [ i + 1 ], clr, 3.f );
					}

					auto sz_font = static_cast< int >( size.x / 2.f );
					auto sz_text = draw_manager->calc_text_size( sz_font, "%.0f", time );
					draw_manager->add_text_on_screen( pos - sz_text / 2.f, A_::set_alpha( COLOR_WHITE, static_cast< unsigned long >( 255 * fl_alpha ) ), sz_font, "%.0f", time );
				};

				if ( w2s.is_valid( ) )
				{
					w2s.y -= size.y / 2;

					if ( m_ward.icon.first )
					{
						draw_manager->add_image( m_ward.icon.first, w2s, size, 90.f, { m_ward.icon.second.x, m_ward.icon.second.y }, { m_ward.icon.second.z, m_ward.icon.second.w },
							{ 0.33f, 0.33f, 0.33f, m_ward.fl_alpha } );
						draw_manager->add_circle_on_screen( w2s + size / 2.f, size.x / 2.f, A_::set_alpha( COLOR_DARK_GRAY, static_cast< int >( 255 * fl_alpha ) ), 3.f );

						auto p_icon = m_ward.get_owner_icon( );
						if ( p_icon )
						{
							draw_manager->add_image( p_icon, w2s + vector( size.x, 0 ), size / 2.f, 90.f, { 0.f, 0.f }, { 1.f, 1.f }, { 1.f, 1.f, 1.f, fl_alpha } );
							draw_manager->add_circle_on_screen( w2s + vector( size.x, 0 ) + size / 4.f, size.x / 4.f, A_::set_alpha( COLOR_DARK_GRAY, static_cast< int >( 255 * fl_alpha ) ), 2.f );
						}
					}

					auto pos = entitylist->get_object_by_network_id( m_ward.network_id );

					auto remaining_time = pos && pos->is_valid( ) && pos->is_ally( ) ? std::max( pos->get_mana( ), 0.f ) : -1.f;
					auto remaining_pcent = pos && pos->is_valid( ) && pos->is_ally( ) ? std::clamp( pos->get_mana_percent( ) / 100.f, 0.05f, 1.f ) : -1.f;

					if ( pos && pos->is_valid( ) && pos->is_ally( ) && pos->get_health( ) >= 1 )
					{
						auto object_hash = buff_hash_real( pos->get_name_cstr( ) );
						if ( object_hash != buff_hash( "JammerDevice" ) && object_hash != buff_hash( "VisionWard" ) && remaining_time != -1.f && remaining_pcent != -1.f )
							draw_ward_time( remaining_pcent, remaining_time, w2s + size / 2.f, size );
					}
					else
					{
						auto timer = m_ward.get_remaining_time( ),
							pcent = m_ward.get_remaining_pcent( );
						if ( timer.has_value( ) && pcent.has_value( ) )
							draw_ward_time( pcent.value( ), timer.value( ), w2s + size / 2.f, size );
					}
				}
			}
#pragma endregion 
		};

		m_wards.erase( std::remove_if( m_wards.begin( ), m_wards.end( ), [ ]( std::pair<uint32_t, s_ward>& p ) {
			return p.second.is_expired( ) && p.second.fl_alpha <= 0.01f;
		} ), m_wards.end( ) );

		if ( !m_wards.empty( ) )
		{
			auto can_draw = [ ]( s_ward w ) -> bool {
				return ( w.is_enemy && item::draw_ward->get_bool( ) ) || ( !w.is_enemy && item::draw_ward_ally->get_bool( ) );
			};

			for ( auto&& ward : m_wards )
			{
				if ( !ward.second.is_expired( ) && can_draw( ward.second ) )
				{
					if ( gametime->get_time( ) - ward.second.t_last_anim >= 0.02f )
					{
						ward.second.fl_alpha = std::clamp( easing_fn_in( ward.second.fl_alpha ), 0.001f, 0.999f );
						ward.second.t_last_anim = gametime->get_time( );
					}
				}
				else
				{
					if ( gametime->get_time( ) - ward.second.t_last_anim >= 0.02f )
					{
						ward.second.fl_alpha = std::clamp( easing_fn_out( ward.second.fl_alpha ), 0.001f, 0.999f );
						ward.second.t_last_anim = gametime->get_time( );
					}

					if ( ward.second.fl_alpha <= 0.01f )
						continue;
				}

				draw_ward_r( ward.second );
			}
		}
	}

	auto draw_traps( ) -> void
	{
		static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
		static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

		m_traps.erase( std::remove_if( m_traps.begin( ), m_traps.end( ), [ ]( s_ward t ) {
			return t.is_expired( ) && t.fl_alpha <= 0.01f;
		} ), m_traps.end( ) );

		for ( auto&& trap : m_traps )
		{
			auto it_menu = m_trap_menu.find( trap.model_hash );
			if ( it_menu == m_trap_menu.end( ) )
				continue;

			auto can_draw = it_menu->second.enabled_circle->get_bool( ) && !trap.is_expired( );
			if ( can_draw )
			{
				if ( gametime->get_time( ) - trap.t_last_anim >= 0.02f )
				{
					trap.fl_alpha = std::clamp( easing_fn_in( trap.fl_alpha ), 0.001f, 0.999f );
					trap.t_last_anim = gametime->get_time( );
				}
			}
			else
			{
				if ( gametime->get_time( ) - trap.t_last_anim >= 0.02f )
				{
					trap.fl_alpha = std::clamp( easing_fn_out( trap.fl_alpha ), 0.001f, 0.999f );
					trap.t_last_anim = gametime->get_time( );
				}
			}

			if ( trap.fl_alpha <= 0.01f )
				continue;

#if !defined( __TEST )
			if ( !trap.is_enemy )
				continue;
#endif
			
			draw_manager->add_circle_with_glow_gradient( trap.position,
				A_::set_alpha( it_menu->second.circle_color->get_color( ), static_cast< unsigned int >( A_::get_alpha( it_menu->second.circle_color->get_color( ) ) * trap.fl_alpha ) ),
				A_::set_alpha( it_menu->second.circle_color2->get_color( ), static_cast< unsigned int >( A_::get_alpha( it_menu->second.circle_color2->get_color( ) ) * trap.fl_alpha ) ),
				trap.range, 2.f, glow_data( static_cast<float>( traps::glow_1->get_int( ) ) / 100.f,
					static_cast< float >( traps::glow_2->get_int( ) ) / 100.f, 
					static_cast< float >( traps::glow_3->get_int( ) ) / 100.f,
					static_cast< float >( traps::glow_4->get_int( ) ) / 100.f ) );
		}
	}

	void on_draw( )
	{
		misc::all_draw( );

		if ( keyboard_state->is_pressed( keyboard_game::tab ) )
			return;

		summoner_spell::all_draw( );

		g_wards->draw_ward_points( item::draw_auto_ward_stealth->get_bool( ), item::draw_auto_ward_control->get_bool( ),
			item::auto_ward_mode->get_int( ), item::auto_ward_control->get_bool( ), item::auto_ward_stealth->get_bool( ) );

		if ( myhero->is_dead( ) )
			return;

		g_wards->draw_trick_wards( item::ward_cast_assist->get_bool( ) );

		/*if (gametime->get_time() > cassiopeia_r_time + 0.4f - ping->get_ping() / 1000.0f && cassiopeia_r_time + 0.6f > gametime->get_time())
		{

			//draw_manager->add_circle(cassiopeia_start_pos, 50, MAKE_COLOR(0, 255, 0, 255), 4.f);
			//draw_manager->add_circle(cassiopeia_end_pos, 50, MAKE_COLOR(0, 255, 0, 255), 4.f);

			auto radius = 700;
			const auto perp = (cassiopeia_end_pos - cassiopeia_start_pos).normalized().perpendicular(); //垂直
			auto cassiopeia_r_left_pos = cassiopeia_end_pos + perp * (radius);
			auto cassiopeia_r_right_pos = cassiopeia_end_pos - perp * (radius);
			cassiopeia_r_left_pos = cassiopeia_r_left_pos.extend(cassiopeia_start_pos, 200.0f);
			cassiopeia_r_right_pos = cassiopeia_r_right_pos.extend(cassiopeia_start_pos, 200.0f);

			geometry::polygon q1_sector;

			q1_sector.add(cassiopeia_start_pos);
			q1_sector.add(cassiopeia_r_left_pos);
			q1_sector.add(cassiopeia_end_pos);
			q1_sector.add(cassiopeia_r_right_pos);

			draw_manager->add_line(cassiopeia_start_pos, cassiopeia_r_left_pos, MAKE_COLOR(255, 0, 0, 255), 1.0f);
			draw_manager->add_line(cassiopeia_r_left_pos, cassiopeia_end_pos, MAKE_COLOR(255, 0, 0, 255), 1.0f);
			draw_manager->add_line(cassiopeia_end_pos, cassiopeia_r_right_pos, MAKE_COLOR(255, 0, 0, 255), 1.0f);
			draw_manager->add_line(cassiopeia_r_right_pos, cassiopeia_start_pos, MAKE_COLOR(255, 0, 0, 255), 1.0f);
		}*/

		if ( item::draw_heartsteel->get_bool( ) )
		{
			Item heartsteel( ItemId::Heartsteel, 0 );
			if ( item::use_heartsteel->get_bool( ) && heartsteel.is_vaild( ) )
			{
				auto enemy = entitylist->get_enemy_heroes( );

				enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), [ ]( game_object_script x )
				{
					return !x->is_valid( ) || x->is_dead( ) || !x->is_valid_target( 1000 );
				} ), enemy.end( ) );

				if ( !enemy.empty( ) )
				{
					for ( auto& enemy : enemy )
					{
						auto bar_pos = enemy->get_hpbar_pos( );
						auto buff = enemy->get_buff( buff_hash( "3084proctracker" ) );
						auto buff_cd = enemy->get_buff( buff_hash( "3084targetcooldownbuff" ) );

						if ( buff != nullptr && buff->is_valid( ) && buff->is_alive( ) && bar_pos.is_valid( ) )
						{
							float pos = 142 + abs( 25000 - buff->get_remaining_time( ) ) * 20;
							float pos1 = pos >= 192 ? 192 : pos;

							auto bar_pos = enemy->get_hpbar_pos( );
							auto heart = database->get_item_by_id( ItemId::Heartsteel )->get_texture( );
							auto enemy_pos = bar_pos;
							//auto enemy_pos = enemy->get_position() + vector(90, 200);
							//renderer->world_to_screen(enemy_pos, enemy_pos);

							if ( heart.first )
								draw_manager->add_image( heart.first, { enemy_pos.x + 112, enemy_pos.y + 12 }, { 30, 30 }, 0.f, { heart.second.x, heart.second.y }, { heart.second.z, heart.second.w } );

							draw_manager->add_line_on_screen( { enemy_pos.x + 142, enemy_pos.y + 40 }, { enemy_pos.x + 192, enemy_pos.y + 40 }, MAKE_COLOR( 0, 0, 0, 255 ), 5.0f );
							if ( pos1 < 190 )
							{
								draw_manager->add_line_on_screen( { enemy_pos.x + 142, enemy_pos.y + 40 }, { enemy_pos.x + pos1, enemy_pos.y + 40 }, MAKE_COLOR( 0, 255, 0, 255 ), 5.0f );
							}
							else
							{
								draw_manager->add_line_on_screen( { enemy_pos.x + 142, enemy_pos.y + 40 }, { enemy_pos.x + pos1, enemy_pos.y + 40 }, MAKE_COLOR( 255, 0, 0, 255 ), 5.0f );
							}

						}
						if ( buff_cd != nullptr && buff_cd->is_valid( ) && buff_cd->is_alive( ) && bar_pos.is_valid( ) )
						{
							float pos = 142.f + buff_cd->get_remaining_time( ) * 1.6f;

							auto bar_pos = enemy->get_hpbar_pos( );
							auto heart = database->get_item_by_id( ItemId::Heartsteel )->get_texture( );
							auto enemy_pos = bar_pos;
							//auto enemy_pos = enemy->get_position() + vector(90, 200);
							//renderer->world_to_screen(enemy_pos, enemy_pos);
							if ( heart.first )
								draw_manager->add_image( heart.first, { enemy_pos.x + 112, enemy_pos.y + 12 }, { 30, 30 }, 0.f, { heart.second.x, heart.second.y }, { heart.second.z, heart.second.w } );

							draw_manager->add_line_on_screen( { enemy_pos.x + 127, enemy_pos.y + 12 }, { enemy_pos.x + 127, enemy_pos.y + 42 }, MAKE_COLOR( 0, 0, 0, 125 ), 30.0f );

							draw_manager->add_line_on_screen( { enemy_pos.x + 142, enemy_pos.y + 40 }, { enemy_pos.x + 192, enemy_pos.y + 40 }, MAKE_COLOR( 0, 0, 0, 255 ), 5.0f );
							draw_manager->add_line_on_screen( { enemy_pos.x + 142, enemy_pos.y + 40 }, { enemy_pos.x + pos, enemy_pos.y + 40 }, MAKE_COLOR( 255, 255, 255, 255 ), 5.0f );
						}
					}
				}
			}
		}

		draw_wards( );
	}

	void on_new_draw( )
	{
		summoner_spell::all_new_draw( );
		champion::all_new_draw( );
		misc::all_new_draw( );

		draw_traps( );
	}
}