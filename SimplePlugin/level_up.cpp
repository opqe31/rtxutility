#include "../plugin_sdk/plugin_sdk.hpp"
#include "level_up.h"
#include "A_.h"

namespace level_up
{
	namespace cfg
	{
		TreeTab* leveler_menu = nullptr;
		TreeEntry* leveler_enabled = nullptr;
		TreeEntry* leveler_delay = nullptr;
		TreeEntry* leveler_start = nullptr;
		TreeEntry* leveler_r = nullptr;
		TreeEntry* leveler_qwe = nullptr;

		float hide_notification_str_t = -2.f;
		TreeEntry* notification_str;

		TreeEntry* load_button = nullptr;
		TreeEntry* save_button = nullptr;

		//std::map<int, TreeEntry*> priority_first_custom;
		std::map<int, std::map<int, TreeEntry* > > m_priority_pre6; //preset_id -> preset_level -> menu_element
		std::map<int, TreeEntry* > m_priority6;
		std::map<int, TreeEntry*> preset_name;

		TreeEntry* preset_role;
		std::map<int, std::string> m_role_name = {
			{ 0, "jungle" },
			{ 1, "supp" },
			{ 2, "adc" },
			{ 3, "top" },
			{ 4, "mid" },
		};
		std::map<std::string, int> m_role_id = {
			{ "jungle", 0 },
			{ "supp", 1 },
			{ "adc", 2 },
			{ "top", 3 },
			{ "mid", 4 },
		};
	}

	spellslot get_spell_from_string( std::string in )
	{
		if ( in == "Q" )
			return spellslot::q;
		else if ( in == "W" )
			return spellslot::w;
		else if ( in == "E" )
			return spellslot::e;
		else if ( in == "R" )
			return spellslot::r;

		return spellslot::invalid;
	}

	uint32_t* get_texture_by_skillshot_name( std::string in )
	{
		if ( in == "Q" )
			return myhero->get_spell( spellslot::q )->get_icon_texture( );
		else if ( in == "W" )
			return myhero->get_spell( spellslot::w )->get_icon_texture( );
		else if ( in == "E" )
			return myhero->get_spell( spellslot::e )->get_icon_texture( );

		return nullptr;
	}

	void save_preset( )
	{
		if ( cfg::preset_role->get_int( ) <= 4 )
			return;

		auto id = cfg::preset_role->get_int( );

		if ( cfg::m_priority_pre6.find( id ) == cfg::m_priority_pre6.end( ) ||
			cfg::m_priority6.find( id ) == cfg::m_priority6.end( ) )
		{
			console->print( "[!]0:save_preset%d|%d", -1, id );
			return;
		}

		auto&& prio_other = cfg::m_priority6[ id ]->get_prority_sorted_list( );

		for ( auto i = 1; i <= 5; i++ )
		{
			if ( cfg::m_priority_pre6 [ id ].find( i ) == cfg::m_priority_pre6 [ id ].end( ) || !cfg::m_priority_pre6 [ id ][ i ] )
			{
				console->print( "[!]1:save_preset%d|%d", i, id );
				continue;
			}

			int spell_id = std::clamp( cfg::m_priority_pre6[ id ] [ i ]->get_int( ), 0, 2 );
			cfg::m_priority_pre6 [ id ][ i ]->set_int( spell_id );
		}

		cfg::m_priority6 [ id ]->set_prority_list(
			{
				{ ( uint32_t )( prio_other.at( 0 ).display_name.at( 0 ) ), prio_other.at( 0 ).display_name, true, get_texture_by_skillshot_name( prio_other.at( 0 ).display_name ) },
			{ ( uint32_t )( prio_other.at( 1 ).display_name.at( 0 ) ), prio_other.at( 1 ).display_name, true, get_texture_by_skillshot_name( prio_other.at( 1 ).display_name ) },
			{ ( uint32_t )( prio_other.at( 2 ).display_name.at( 0 ) ), prio_other.at( 2 ).display_name, true, get_texture_by_skillshot_name( prio_other.at( 2 ).display_name ) }
			} );

		cfg::hide_notification_str_t = gametime->get_time( ) + 3.5f;
		cfg::notification_str->set_display_name( "Saved preset " + cfg::preset_name [ id ]->get_string( ) );
		cfg::notification_str->is_hidden( ) = false;
	}

	void load_preset( bool first_load )
	{
		auto&& level_data = load_data( );
		static auto sdk_smite = myhero->get_spell( spellslot::summoner1 )->get_spell_data( )->get_name( ).find( "SummonerSmite" ) != std::string::npos ||
			myhero->get_spell( spellslot::summoner2 )->get_spell_data( )->get_name( ).find( "SummonerSmite" ) != std::string::npos;

		auto id = cfg::preset_role->get_int( );
		if ( cfg::preset_role->get_int( ) <= 4 )
		{
			if ( level_data.find( myhero->get_champion( ) ) == level_data.end( ) )
			{
				cfg::hide_notification_str_t = FLT_MAX;
				cfg::notification_str->set_display_name( "Error: 1" );
				return;
			}

			auto leveler_entry = level_data [ myhero->get_champion( ) ];
			if ( first_load )
			{
				leveler_entry.erase( std::remove_if( leveler_entry.begin( ), leveler_entry.end( ), [ ]( s_champion_entry a )
				{
					return ( strcmp( a.role_name, "jungle" ) == 0 && !sdk_smite ) || ( strcmp( a.role_name, "jungle" ) != 0 && sdk_smite );
				} ), leveler_entry.end( ) );

				if ( leveler_entry.size( ) > 1 )
				{
					std::sort( leveler_entry.begin( ), leveler_entry.end( ), [ ]( s_champion_entry a, s_champion_entry b )
					{
						return a.matches > b.matches;
					} );
				}
				console->print( "first_load | best_role: %s", leveler_entry.front( ).role_name );

				if ( cfg::m_role_id.find( leveler_entry.front( ).role_name ) != cfg::m_role_id.end( ) )
					cfg::preset_role->set_int( cfg::m_role_id [ leveler_entry.front( ).role_name ] );
			}

			for ( auto role_entry : leveler_entry )
			{
				if ( role_entry.role_name != cfg::m_role_name [ cfg::preset_role->get_int( ) ] && !first_load )
					continue;

				if ( cfg::m_role_name.find( cfg::preset_role->get_int( ) ) == cfg::m_role_name.end( ) && !first_load )
				{
					cfg::hide_notification_str_t = FLT_MAX;
					cfg::notification_str->set_display_name( "Error: 2" );
					return;
				}

				if ( role_entry.first.size( ) < 3 || role_entry.other.size( ) < 5 )
					return;

				for ( auto lvl = 1; lvl <= 5; lvl++ )
				{
					auto spell_id = std::clamp( ( int )get_spell_from_string( role_entry.other.at( lvl - 1 ) ), 0, 2 );
					cfg::m_priority_pre6[ id ] [ lvl ]->set_int( spell_id );
				}

				/*cfg::priority_first->set_prority_list(
					{
						{ 1, role_entry.other.at(0), true, get_texture_by_skillshot_name(role_entry.other.at(0)) },
						{ 2, role_entry.other.at(1), true, get_texture_by_skillshot_name(role_entry.other.at(1)) },
						{ 3, role_entry.other.at(2), true, get_texture_by_skillshot_name(role_entry.other.at(2)) },
					});*/

				cfg::m_priority6[ id ]->set_prority_list(
					{
						{ buff_hash_real( role_entry.first.at( 0 ) ), role_entry.first.at( 0 ), true, get_texture_by_skillshot_name( role_entry.first.at( 0 ) ) },
						{ buff_hash_real( role_entry.first.at( 1 ) ), role_entry.first.at( 1 ), true, get_texture_by_skillshot_name( role_entry.first.at( 1 ) ) },
						{ buff_hash_real( role_entry.first.at( 2 ) ), role_entry.first.at( 2 ), true, get_texture_by_skillshot_name( role_entry.first.at( 2 ) ) },
					} );

				cfg::hide_notification_str_t = gametime->get_time( ) + 3.5f;
				cfg::notification_str->set_display_name( "Loaded default preset - " + ( std::string )role_entry.role_name );
				cfg::notification_str->is_hidden( ) = false;
				return;
			}
		}
		else
		{
			if ( cfg::m_priority_pre6.find( id ) == cfg::m_priority_pre6.end( ) ||
				cfg::m_priority6.find( id ) == cfg::m_priority6.end( ) )
			{
				console->print( "[!]0:load_preset%d|%d", -1, id );
				return;
			}

			auto prio_other = cfg::m_priority6 [ id ]->get_prority_sorted_list( );
			for ( auto lvl = 1; lvl <= 5; lvl++ )
			{
				if ( cfg::m_priority_pre6 [ id ].find( lvl ) == cfg::m_priority_pre6 [ id ].end( ) )
				{
					cfg::hide_notification_str_t = FLT_MAX;
					cfg::notification_str->set_display_name( "Error: 3" );
					continue;
				}

				int spell_id = std::clamp( cfg::m_priority_pre6 [ id ][ lvl ]->get_int( ), 0, 2 );
				cfg::m_priority_pre6[ id ] [ lvl ]->set_int( spell_id );
			}

			/*cfg::priority_first->set_prority_list(
				{
					{ 1, prio_first.at(0).display_name, true, get_texture_by_skillshot_name(prio_first.at(0).display_name) },
					{ 2, prio_first.at(1).display_name, true, get_texture_by_skillshot_name(prio_first.at(1).display_name) },
					{ 3, prio_first.at(2).display_name, true, get_texture_by_skillshot_name(prio_first.at(2).display_name) }
				});
			*/

			std::vector<ProrityCheckItem> custom_priorities = { };
			for ( auto& entry : prio_other )
				custom_priorities.push_back( { ( uint32_t )entry.display_name [ 0 ], entry.display_name, true, get_texture_by_skillshot_name( entry.display_name ) } );

			cfg::m_priority6[ id ]->set_prority_list( custom_priorities );

			auto level_to_string = [ & ]( int lvl ) -> const char*
			{
				switch ( lvl )
				{
					default: return "Q"; break;
					case 1: return "W"; break;
					case 2: return "E"; break;
				}
			};

			cfg::hide_notification_str_t = gametime->get_time( ) + 3.5f;
			cfg::notification_str->set_display_name( "Loaded preset " + cfg::preset_name [ id ]->get_string( ) );
			cfg::notification_str->is_hidden( ) = false;

		}
	}

	auto on_preset_change( TreeEntry* self )
	{
		cfg::save_button->is_hidden( ) = self->get_int( ) <= 4;

		for ( auto i = 0; i <= 7; i++ )
		{
			cfg::m_priority6 [ i ]->is_hidden( ) = cfg::preset_role->get_int( ) != i;
			cfg::preset_name [ i ]->is_hidden( ) = cfg::preset_role->get_int( ) != i || i <= 4;

			for ( auto lvl = 1; lvl <= 5; lvl++ )
				cfg::m_priority_pre6 [ i ][ lvl ]->is_hidden( ) = cfg::preset_role->get_int( ) != i;
		}
	}

	void create_menu( )
	{
		auto base_menu = menu->get_tab( "utility" );
		auto leveler_menu = base_menu->add_tab( "_leveler_menu", "Auto leveler" );
		if ( leveler_menu )
		{
			auto my_q_texture = myhero->get_spell( spellslot::q )->get_icon_texture( ),
				my_w_texture = myhero->get_spell( spellslot::w )->get_icon_texture( ),
				my_e_texture = myhero->get_spell( spellslot::e )->get_icon_texture( ),
				my_r_texture = myhero->get_spell( spellslot::r )->get_icon_texture( );

			leveler_menu->add_separator( "_leveler_general", " - - Common - - " );
			cfg::leveler_enabled = leveler_menu->add_checkbox( "_leveler_enabled", "Enabled", false );
			leveler_menu->set_assigned_active( cfg::leveler_enabled );
			cfg::leveler_delay = leveler_menu->add_slider( "_leveler_delay", "Delay (ms)", 100, 0, 2000 );
			cfg::leveler_start = leveler_menu->add_slider( "_leveler_start", "Level > x", 3, 0, 3 );
			cfg::leveler_r = leveler_menu->add_checkbox( "_leveler_r", "Level-up R", true );
			cfg::leveler_r->set_texture( my_r_texture );
			cfg::leveler_qwe = leveler_menu->add_checkbox( "_leveler_qwe", "Level-up QWE", true );

			leveler_menu->add_separator( "_leveler_1_3", " - - Priorities - - " );
			auto tab_pre6 = leveler_menu->add_tab( "_tab_pre_6", "Level 1-5" );
			{
				for ( auto id = 0; id <= 7; id++ )
				{
					for ( auto lvl = 1; lvl <= 5; lvl++ )
					{
						cfg::m_priority_pre6 [ id ][ lvl ] = tab_pre6->add_combobox( myhero->get_model( ) + "lvl" + std::to_string( lvl ) + "id" + std::to_string( id ), "Lvl " + std::to_string( lvl ), { { "Q", my_q_texture }, { "W", my_w_texture }, { "E", my_e_texture } }, 0 );
						cfg::m_priority_pre6 [ id ][ lvl ]->is_hidden( ) = true;
					}
				}
			}

			for ( auto id = 0; id <= 7; id++ )
			{
				cfg::m_priority6[ id ] = leveler_menu->add_prority_list( "m_priority6_" + myhero->get_model( ) + "_" + std::to_string( id ), "Level 6+",
					{
						{ 'Q', "Q", true, my_q_texture },
						{ 'W', "W", true, my_w_texture },
						{ 'E', "E", true, my_e_texture }
					}, true );

				cfg::m_priority6 [ id ]->is_hidden( ) = true;
			}

			leveler_menu->add_separator( "_leveler_presets", " - - Presets manager - - " );
			cfg::preset_role = leveler_menu->add_combobox( "_preset_role_" + myhero->get_model( ), "Preset name",
				{
					{ "jungle (can't modify)", nullptr },
					{ "support (can't modify)", nullptr },
					{ "adc (can't modify)", nullptr },
					{ "toplane (can't modify)", nullptr },
					{ "midlane (can't modify)", nullptr }, //4
					{ "custom1", nullptr },
					{ "custom2", nullptr },
					{ "custom3", nullptr }
				}, 0 );

			for ( auto i = 0; i <= 7; i++ )
				cfg::preset_name [ i ] = leveler_menu->add_text_input( "_preset_name_" + myhero->get_model( ) + "_" + std::to_string( i ), "Preset name", "Custom " + std::to_string( i - 4 ) );

			cfg::notification_str = leveler_menu->add_separator( "notification_str", "" );
			cfg::notification_str->is_hidden( ) = true;

			cfg::load_button = leveler_menu->add_button( "_leveler_preset_load", "Load" );
			cfg::load_button->add_property_change_callback( [ ]( TreeEntry* ) {
				load_preset( false );
			} );

			cfg::save_button = leveler_menu->add_button( "_leveler_preset_save", "Save" );
			cfg::save_button->add_property_change_callback( [ ]( TreeEntry* ) {
				save_preset( );
			} );

			cfg::preset_role->add_property_change_callback( on_preset_change );
			cfg::save_button->is_hidden( ) = cfg::preset_role->get_int( ) <= 4;

			load_preset( false );
			on_preset_change( cfg::preset_role );
		}
	}

	auto last_tick = -5.f;
	void leveler_on_tick( )
	{
		if ( gametime->get_time( ) > cfg::hide_notification_str_t )
			cfg::notification_str->is_hidden( ) = true;

		if ( !myhero || myhero->is_dead( ) || !cfg::leveler_enabled->get_bool( ) )
			return;

		if ( gametime->get_time( ) - last_tick < ( cfg::leveler_delay->get_int( ) / 1000.f ) )
			return;

		auto lvl = std::min( 18, myhero->get_level( ) );
		if ( lvl <= cfg::leveler_start->get_int( ) )
			return;

		last_tick = gametime->get_time( );
		auto c = myhero->get_champion( );
		auto level_points = myhero->get_lvlup_stacks( );

		if ( level_points <= 0 )
			return;

		if ( cfg::leveler_r->get_bool( ) && lvl >= 6 && myhero->get_champion( ) != champion_id::Udyr )
		{
			auto r_level = myhero->get_spell( spellslot::r )->level( );

			if ( c == champion_id::Elise ||
				c == champion_id::Jayce ||
				c == champion_id::Nidalee )
				r_level -= 1;

			auto r_slots_needed = 0;
			if ( lvl >= 6 )
				r_slots_needed = 1;
			if ( lvl >= 11 )
				r_slots_needed = 2;
			if ( lvl >= 16 )
				r_slots_needed = 3;

			if ( r_level < r_slots_needed )
			{
				myhero->levelup_spell( spellslot::r );
				return;
			}
		}

		if ( !cfg::leveler_qwe->get_bool( ) )
			return;

		if ( ( c == champion_id::Udyr || c == champion_id::Aphelios ) && cfg::preset_role->get_int( ) <= 4 )
		{
			auto&& level_data = load_data( );
			if ( level_data.find( c ) == level_data.end( ) )
				return;

			if ( cfg::m_role_name.find( cfg::preset_role->get_int( ) ) == cfg::m_role_name.end( ) )
				return;

			auto entries = level_data [ c ];
			for ( auto e : entries )
			{
				if ( e.role_name != cfg::m_role_name [ cfg::preset_role->get_int( ) ] )
					continue;

				auto real_pos = lvl - level_points;
				auto spell_slot = get_spell_from_string( e.other.at( real_pos ) );
				if ( spell_slot != spellslot::invalid )
					myhero->levelup_spell( spell_slot );

				return;
			}
		}
		else
		{
			auto id = cfg::preset_role->get_int( );
			if ( lvl - ( level_points - 1 ) <= 5 )
			{
				auto level_at = lvl - ( level_points - 1 );
				auto prio_slot = cfg::m_priority_pre6[ id ] [ level_at ]->get_int( );
				myhero->levelup_spell( ( spellslot )prio_slot );
				return;
			}

			auto p_menu = cfg::m_priority6 [ id ];
			auto first = get_spell_from_string( p_menu->get_prority_sorted_list( ) [ 0 ].display_name ),
				second = get_spell_from_string( p_menu->get_prority_sorted_list( ) [ 1 ].display_name ),
				third = get_spell_from_string( p_menu->get_prority_sorted_list( ) [ 2 ].display_name );

			auto first_level = myhero->get_spell( first )->level( ),
				second_level = myhero->get_spell( second )->level( ),
				third_level = myhero->get_spell( third )->level( );

			if ( first_level < 5 && lvl != 8 && lvl != 10 )
				myhero->levelup_spell( first );

			if ( second_level < 5 && first_level > 0 )
				myhero->levelup_spell( second );

			if ( third_level < 5 && first_level > 0 && second_level > 0 )
				myhero->levelup_spell( third );
		}
	}

	void load( )
	{
		load_data( );
		event_handler<events::on_update>::add_callback( leveler_on_tick );
	}

	void unload( )
	{
		event_handler<events::on_update>::remove_handler( leveler_on_tick );
	}
}