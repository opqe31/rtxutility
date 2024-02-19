#include "fow_tracker.h"
#include "A_.h"
#include "main_hud.hpp"

#include "notify.h"

#include <sstream>
#include <regex>

c_fow_tracker* fow_tracker = new c_fow_tracker( );

#define AGGRO_TIME_TOL 15.f

struct s_last_aggro 
{
	float time = -8.f;
	float t_last_notify = -20.f;
};

s_last_aggro last_aggro_herald;
s_last_aggro last_aggro_dragon;

#if defined( __TEST )
struct s_debug_draw {
	std::string text;
	std::string text_alt;
	vector position;
	float time;

	s_debug_draw( std::string main_text, std::string alt_text, vector p, float t )
	{
		text = main_text;
		text_alt = alt_text;
		position = p;
		time = gametime->get_time( ) + t;
	}
};
std::vector< s_debug_draw > m_draw;
#endif

std::map<int, float> m_last_damage_taken = { };

auto c_fow_tracker::is_objective_under_attack( int obj ) -> bool
{
	auto it_dmg_taken = m_last_damage_taken.find( obj );
	auto dmg_taken_v = it_dmg_taken != m_last_damage_taken.end( ) ? gametime->get_time( ) - it_dmg_taken->second : FLT_MAX;

	if ( obj == neutral_camp_id::Dragon )
		return std::min( gametime->get_time( ) - last_aggro_dragon.time, dmg_taken_v ) <= AGGRO_TIME_TOL;
	else if ( obj == neutral_camp_id::Baron || obj == neutral_camp_id::Herlad )
		return std::min( gametime->get_time( ) - last_aggro_herald.time, dmg_taken_v ) <= AGGRO_TIME_TOL;

	return false;
}

void c_fow_tracker::update_position( uint32_t network_id, vector position, bool p )
{
	if ( tracker_data.find( network_id ) == tracker_data.end( ) )
		tracker_data [ network_id ] = { };

	if ( p )
		tracker_data [ network_id ].t_last_update_particle = gametime->get_time( );

	tracker_data [ network_id ].t_last_update = gametime->get_time( );
	tracker_data [ network_id ].position = position;
}

auto c_fow_tracker::get_data( uint32_t network_id ) -> s_fow_tracker_entry
{
	if ( tracker_data.find( network_id ) == tracker_data.end( ) )
		return s_fow_tracker_entry { -600.f, -600.f, vector::zero };

	return tracker_data [ network_id ];
}

auto c_fow_tracker::is_enabled( e_enabled_mode mode ) -> bool
{
	if ( !cfg_ref )
		return false;

	switch ( mode )
	{
		default:
			return false;

		case e_enabled_mode::all:
		{
			for ( auto&& x : cfg_ref->get_prority_sorted_list( ) )
				if ( x.is_active )
					return true;

			return false;
		}

		case e_enabled_mode::native:
		{
			for ( auto&& x : cfg_ref->get_prority_sorted_list( ) )
				if ( x.is_active && x.display_name == "Native" )
					return true;

			return false;
		}

		case e_enabled_mode::minimap:
		{
			for ( auto&& x : cfg_ref->get_prority_sorted_list( ) )
				if ( x.is_active && x.display_name == "Minimap" )
					return true;

			return false;
		}
	}
}

auto c_fow_tracker::get_camp_id_mutual( int id ) -> int
{
	switch ( id )
	{
		default:
			return id;

		case neutral_camp_id::Blue_Order:
		case neutral_camp_id::Blue_Chaos:
			return neutral_camp_id::Blue_Order;

		case neutral_camp_id::Wolves_Order:
		case neutral_camp_id::Wolves_Chaos:
			return neutral_camp_id::Wolves_Order;

		case neutral_camp_id::Raptors_Chaos:
		case neutral_camp_id::Raptors_Order:
			return neutral_camp_id::Raptors_Order;

		case neutral_camp_id::Red_Order:
		case neutral_camp_id::Red_Chaos:
			return neutral_camp_id::Red_Order;


		case neutral_camp_id::Krugs_Order:
		case neutral_camp_id::Krugs_Chaos:
			return neutral_camp_id::Krugs_Order;

		case neutral_camp_id::Gromp_Order:
		case neutral_camp_id::Gromp_Chaos:
			return neutral_camp_id::Gromp_Order;

		case neutral_camp_id::Crab_Bottom:
		case neutral_camp_id::Crab_Top:
			return neutral_camp_id::Crab_Bottom;
	}
}

auto c_fow_tracker::get_monster_icon( int camp ) -> uint32_t*
{
	auto camp_id = get_camp_id_mutual( camp );

	auto it = m_monster_icons.find( camp_id );
	if ( it == m_monster_icons.end( ) )
		return nullptr;

	return it->second;
}

auto c_fow_tracker::update_monster_icon( int camp, game_object_script x ) -> void
{
	auto camp_id = get_camp_id_mutual( camp );

	if ( x->get_square_icon_portrait( ) )
		m_monster_icons [ camp_id ] = x->get_square_icon_portrait( );
}

auto c_fow_tracker::camp_respawn_t( int camp, bool kill ) -> float
{
	auto m_s = [ ]( int minutes, float seconds = 0 ) {
		return 60.f * static_cast<float>( minutes ) + static_cast<float>( seconds );
	};

	auto is_elder = [ ]( ) -> bool {
		auto count_ally = 0,
			count_enemy = 0;

		for ( auto&& x : entitylist->get_ally_heroes( ) )
			if ( x && x->is_valid( ) )
				count_ally += x->get_hero_stat( int_hero_stat::DRAGON_KILLS );

		for ( auto&& x : entitylist->get_enemy_heroes( ) )
			if ( x && x->is_valid( ) )
				count_enemy += x->get_hero_stat( int_hero_stat::DRAGON_KILLS );

		return count_ally >= 4 || count_enemy >= 4;
	};

	auto t_respawn = 1.f;
	auto camp_id = get_camp_id_mutual( camp );

	switch ( camp_id )
	{
		case neutral_camp_id::Blue_Order:
		case neutral_camp_id::Red_Order: //0, 1
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 1, 30 ) && !kill ) ?
				( m_s( 1, 30 ) ) : m_s( 5 );

			break;
		}

		case neutral_camp_id::Gromp_Order: //0, 1
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 1, 42 ) && !kill ) ?
				( m_s( 1, 42 ) ) : m_s( 2, 15 );

			break;
		}

		case neutral_camp_id::Raptors_Order: //0,1
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 1, 30 ) && !kill ) ?
				( m_s( 1, 30 ) ) : m_s( 2, 15 );

			break;
		}

		case neutral_camp_id::Wolves_Order: //0,1
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 1, 30 ) && !kill ) ?
				( m_s( 1, 30 ) ) : m_s( 2, 15 );

			break;
		}

		case neutral_camp_id::Krugs_Order: //0,1
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 1, 42 ) && !kill ) ? m_s( 1, 42 ) : m_s( 2, 15 );

			break;
		}

		case neutral_camp_id::Dragon: //elder: 6.f * 60.f
		{
			if ( gametime->get_time( ) <= m_s( 5 ) && !kill )
				t_respawn = m_s( 5 );
			else
			{
				if ( is_elder( ) )
					t_respawn = m_s( 6 );
				else
					t_respawn = m_s( 5 );
			}

			break;
		}

		case neutral_camp_id::Herlad: //0, 1
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 8 ) && !kill ) ? m_s( 8 ) : m_s( 6 );

			break;
		}

		case neutral_camp_id::Baron: //0, 1
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 20 ) && !kill ) ? m_s( 20 ) : m_s( 6 );

			break;
		}

		//respawn is after crab ward is dead ( 90.f )
		case neutral_camp_id::Crab_Bottom: //3:30 2:30
		{
			t_respawn = ( gametime->get_time( ) <= m_s( 3, 30 ) && !kill ) ? m_s( 3, 30 ) : m_s( 4 ); //2.f * 60.f + 30.f + 90.f;

			break;
		}
	}

	if ( t_respawn > 5.f )
		t_respawn -= 5.f;

	return t_respawn;
}

auto c_fow_tracker::camp_id_from_string( game_object_script sender ) -> std::optional<uint32_t>
{
	std::regex pattern( "[0-9.]+" );

	std::smatch matches;
	std::string result;

	auto name = sender->get_name( );
	for ( auto it = std::sregex_iterator( name.begin( ), name.end( ), pattern ); it != std::sregex_iterator( ); ++it )
		result += it->str( );

	if ( result.empty( ) )
		return std::nullopt;

	auto v_id = A_::explode_string( result, '.' );
	if ( v_id.empty( ) )
		return std::nullopt;

	return std::atoi( v_id [ 0 ].c_str( ) );
}

auto c_fow_tracker::notify_objective_attack( game_object_script sender, uint32_t* opt_icon ) -> void
{
	if ( !sender->is_monster( ) || sender->is_visible( ) )
		return;

	auto sender_name = sender->get_name( );
	std::transform( sender_name.begin( ), sender_name.end( ), sender_name.begin( ),
		[ ]( unsigned char c ) { return std::tolower( c ); } );

	auto dragon = sender_name.find( "sru_dragon" ) != std::string::npos;
	auto baron = sender_name.find( "sru_baron" ) != std::string::npos;
	auto herald = sender_name.find( "sru_riftherald" ) != std::string::npos;

	if ( !dragon && !baron && !herald )
		return;

	if ( ( !dragon && gametime->get_time( ) - last_aggro_herald.t_last_notify >= AGGRO_TIME_TOL ) ||
		( dragon && gametime->get_time( ) - last_aggro_dragon.t_last_notify >= AGGRO_TIME_TOL ) )
	{
		auto name_notify = sender->get_model( );
		if ( name_notify.size( ) > 4 )
			name_notify = name_notify.substr( 4 );

		g_notify->add( sender->get_square_icon_portrait( ), opt_icon, -1.f, {
			c_notify::s_notify_text( name_notify, c_notify::e_color_mode::active_text ),
			c_notify::s_notify_text( " is under ", c_notify::e_color_mode::text ),
			c_notify::s_notify_text( "attack", c_notify::e_color_mode::active_text ),
			c_notify::s_notify_text( "!", c_notify::e_color_mode::text )
			} );

		if ( dragon )
			last_aggro_dragon.t_last_notify = gametime->get_time( );
		else
			last_aggro_herald.t_last_notify = gametime->get_time( );
	}
}

auto on_network_packet_fow( game_object_script sender, std::uint32_t network_id, pkttype_e type, void* args ) -> void
{
	if ( !args || type != pkttype_e::PKT_S2C_PlayAnimation_s || !sender || !sender->is_valid( ) )
		return;

	auto data = ( PKT_S2C_PlayAnimationArgs* )args;
	if ( !data ) 
		return;

	auto sender_name = sender->get_name( );
	std::transform( sender_name.begin( ), sender_name.end( ), sender_name.begin( ),
		[ ]( unsigned char c ) { return std::tolower( c ); } );

	if ( strlen( data->animation_name ) <= 0 )
		return;

	auto anim_hash = buff_hash_real( data->animation_name );

	if ( sender->is_monster( ) && anim_hash == buff_hash( "crab_hide" ) )
	{
		auto camp_id = fow_tracker->camp_id_from_string( sender );
		if ( camp_id.has_value( ) && ( ( camp_id.value( ) == neutral_camp_id::Crab_Bottom || camp_id.value( ) == neutral_camp_id::Crab_Top ) ) )
		{
			if ( fow_tracker->update_minimap && fow_tracker->update_minimap->get_bool( ) )
				camp_manager->update_camp_alive_status( camp_id.value( ), false );
		}
	}

#ifdef __TEST
	/*if ( !sender->is_visible( ) )
		myhero->print_chat( 0x3, "name: %s(skin:%s | model:%s)|anim: %s (%.0f|%.0f)", 
			sender->get_name_cstr( ),
			sender->get_base_skin_name( ).c_str( ), 
			sender->get_model_cstr( ), data->animation_name, sender->get_position( ).x, sender->get_position( ).y );*/
#endif

	auto model_hash = buff_hash_real( sender->get_model_cstr( ) );
	//switch ( model_hash )
	//{
	//	default:
	//		break;

	//	case buff_hash( "SRU_Plant_Vision" ):
	//	case buff_hash( "SRU_Plant_Satchel" ):
	//	case buff_hash( "SRU_Plant_Health" ):
	//	{
	//		//fow_tracker->handle_plant( sender, anim_hash );

	//		break;
	//	}
	//}

	auto dragon = sender->is_monster( ) && sender_name.find( "sru_dragon" ) != std::string::npos;
	auto baron = sender->is_monster( ) && sender_name.find( "sru_baron" ) != std::string::npos;
	auto herald = sender->is_monster( ) && sender_name.find( "sru_riftherald" ) != std::string::npos;
	if ( dragon || baron || herald )
	{
#pragma region local_ping
		if ( fow_tracker->local_ping && fow_tracker->local_ping->get_bool( ) && A_::obs_safe( ) &&
			!sender->is_visible( ) )
		{
			if ( ( ( herald || baron ) && gametime->get_time( ) - last_aggro_herald.time >= AGGRO_TIME_TOL ) ||
				( dragon && gametime->get_time( ) - last_aggro_dragon.time >= AGGRO_TIME_TOL ) )
			{
				auto position = A_::randomize( camp_manager->get_camp_position( dragon ? neutral_camp_id::Dragon : neutral_camp_id::Herlad ), -70.f, 70.f );
				myhero->cast_local_ping( position, sender, _player_ping_type::danger, true, true );
			}
		}
#pragma endregion local_ping
#pragma region global_ping
		if ( fow_tracker->global_ping && fow_tracker->global_ping->get_bool( ) && A_::obs_safe( ) &&
			!sender->is_visible( ) )
		{
			if ( ( ( herald || baron ) && gametime->get_time( ) - last_aggro_herald.time >= AGGRO_TIME_TOL ) ||
				( dragon && gametime->get_time( ) - last_aggro_dragon.time >= AGGRO_TIME_TOL ) )
			{
				auto position = A_::randomize( camp_manager->get_camp_position( dragon ? neutral_camp_id::Dragon : neutral_camp_id::Herlad ), -70.f, 70.f );
				myhero->cast_ping( position, sender, _player_ping_type::danger );
			}
		}
#pragma endregion global_ping
#pragma region notify
		fow_tracker->notify_objective_attack( sender );
#pragma endregion

		if ( herald || baron )
			last_aggro_herald.time = gametime->get_time( );

		if ( dragon )
			last_aggro_dragon.time = gametime->get_time( );
	}
}

auto on_delete_object_fow( game_object_script sender ) -> void
{
	if ( !sender || !sender->is_valid( ) )
		return;

	fow_tracker->handle_player_death( sender );
	if ( sender->is_monster( ) )
		fow_tracker->update_camp_status( sender, true );

	if ( !sender->is_visible( ) && sender->is_monster( ) )
	{
#ifdef __TEST
		myhero->print_chat( 0x3, "[%.1f]on_delete_object_fow[-1]: %s", gametime->get_time( ), sender->get_name_cstr( ) );
#endif
	}
}

auto on_create_object_fow( game_object_script sender ) -> void
{
	if ( !sender || !sender->is_valid( ) )
		return;

	if ( !fow_tracker->is_enabled( ) )
		return;

	auto emitter = sender->get_emitter( );
	auto attachment_t = sender->get_particle_target_attachment_object( );
	auto attachment = sender->get_particle_attachment_object( );

	auto emitter_not_self = [ & ]( game_object_script s ) {
		return emitter && emitter->is_valid( ) && emitter->is_ai_base( ) && emitter->get_network_id( ) != s->get_network_id( ) && !emitter->is_dead( );
	};

	auto monster_object = [ ]( game_object_script m ) {
		return m && m->is_valid( ) && m->is_ai_base( ) && !m->is_dead( ) && !m->is_visible( ) && A_::is_monster( m );
	};

	if ( monster_object( emitter ) )
	{
		//myhero->print_chat( 0x3, "emitter %s", sender->get_name_cstr( ) );
	}

	if ( monster_object( attachment_t ) && emitter_not_self( attachment_t ) )
	{
		auto camp_id = fow_tracker->camp_id_from_string( attachment_t );
		if ( camp_id.has_value( ) )
			m_last_damage_taken [ camp_id.value( ) ] = gametime->get_time( );

		//myhero->print_chat( 0x3, "attachment_t %s", sender->get_name_cstr( ) );
	}

	if ( monster_object( attachment ) && emitter_not_self( attachment ) )
	{
		auto camp_id = fow_tracker->camp_id_from_string( attachment );
		if ( camp_id.has_value( ) )
			m_last_damage_taken [ camp_id.value( ) ] = gametime->get_time( );
		
		//myhero->print_chat( 0x3, "attachment %s", sender->get_name_cstr( ) );
	}
}

constexpr const int ignore_ids[ ] = {
	neutral_camp_id::Dragon,
	neutral_camp_id::Baron,

	neutral_camp_id::Herlad
};

auto c_fow_tracker::update_camp_status( game_object_script sender, bool notify ) -> void
{
	std::regex pattern( "[0-9.]+" );

	std::smatch matches;
	std::string result;

	auto name = sender->get_name( );
	for ( auto it = std::sregex_iterator( name.begin( ), name.end( ), pattern ); it != std::sregex_iterator( ); ++it )
		result += it->str( );

	if ( !result.empty( ) )
	{
		auto v_id = A_::explode_string( result, '.' );
		if ( v_id.size( ) >= 2 )
		{
			auto name_lower = sender->get_name( );
			std::transform( name_lower.begin( ), name_lower.end( ), name_lower.begin( ),
				[ ]( unsigned char c ) { return std::tolower( c ); } );

#pragma region notify
			if ( !sender->is_visible( ) && !A_::is_epic_monster( name_lower ) && notify )
			{
				auto name_notify = sender->get_model( );
				if ( name_notify.size( ) > 4 )
					name_notify = name_notify.substr( 4 );

				auto is_small = name_lower.find( "mini" ) != std::string::npos;
				if ( ( g_notify->e_notify_jungle->get_prority( buff_hash( "dead_small" ) ).second && is_small ) ||
					( g_notify->e_notify_jungle->get_prority( buff_hash( "dead_big" ) ).second && !is_small ) )
				{
					g_notify->add( sender->get_square_icon_portrait( ), nullptr, -1.f, {
						c_notify::s_notify_text( name_notify, c_notify::e_color_mode::active_text ),
						c_notify::s_notify_text( " has been killed in ", c_notify::e_color_mode::text ),
						c_notify::s_notify_text( "FoW", c_notify::e_color_mode::active_text )
						} );
				}
			}
#pragma endregion

			auto camp_id = std::atoi( v_id [ 0 ].c_str( ) );

			std::vector<game_object_script> alive_monsters;
			for ( auto&& i : camp_manager->get_camp_minions( camp_id ) )
			{
				auto x = entitylist->get_object_by_network_id( i );
				if ( x && x->is_valid( ) && !x->is_dead( ) && x->is_monster( ) )
				{
					auto name = x->get_name( );
					std::transform( name.begin( ), name.end( ), name.begin( ),
						[ ]( unsigned char c ) { return std::tolower( c ); } );

					if ( A_::is_monster( name ) )
						alive_monsters.push_back( x );
				}
			}

			if ( alive_monsters.empty( ) )
			{
				auto camp_respawn_t = fow_tracker->camp_respawn_t( camp_id, true );
				if ( camp_respawn_t != 1.f &&
					std::find( std::begin( ignore_ids ), std::end( ignore_ids ), camp_id ) == std::end( ignore_ids ) )
				{
					camp_manager->update_camp_respawn_time( camp_id, gametime->get_time( ) + camp_respawn_t );

					if ( fow_tracker->update_minimap && fow_tracker->update_minimap->get_bool( ) )
						camp_manager->update_camp_alive_status( camp_id, false );
				}
			}
		}
	}
};

auto c_fow_tracker::handle_player_death( game_object_script sender ) -> void
{
	if ( sender->is_ai_hero( ) )
	{
		auto it = fow_tracker->tracker_data.find( sender->get_network_id( ) );
		if ( it != fow_tracker->tracker_data.end( ) )
		{
			it->second.t_last_update = it->second.t_last_update_particle = -200.f;
			it->second.position = vector::zero;
		}
	}
}

auto on_death_object_fow( game_object_script sender ) -> void
{
	if ( !sender || !sender->is_valid( ) )
		return;

	fow_tracker->handle_player_death( sender );
	if ( sender->is_monster( ) )
		fow_tracker->update_camp_status( sender, false );

	if ( !sender->is_visible( ) && sender->is_monster( ) )
	{
#ifdef __TEST
		myhero->print_chat( 0x3, "[%.1f]on_death_object_fow[-1]: %s", gametime->get_time( ), sender->get_name_cstr( ) );
#endif
	}
}

auto on_play_sound_fow( const on_play_sound_args& args ) -> void
{
	if ( !args.target && !args.source )
		return;

	if ( args.target && args.target->is_valid( ) && !args.target->is_visible( ) && args.target->is_ai_hero( ) && args.target_sound_position.is_valid( ) &&
		args.target->is_enemy( ) )
	{
		fow_tracker->update_position( args.target->get_network_id( ), args.target_sound_position, true );

#ifdef __TEST
		//myhero->print_chat( 0x3, "[%.1f]on_play_sound_fow: %s | target: %s", gametime->get_time( ), args.sound_name, args.target->get_model_cstr( ) );
		m_draw.push_back( s_debug_draw(
			"snd (" + std::string( args.sound_name ) + ")",
			"target: " + args.target->get_model( ),
			args.target_sound_position,
			10.f
		) );
#endif
	}

	if ( args.source && args.source->is_valid( ) && !args.source->is_visible( ) && args.source->is_ai_hero( ) && args.source_sound_position.is_valid( ) &&
		args.source->is_enemy( ) )
	{
		fow_tracker->update_position( args.source->get_network_id( ), args.source_sound_position, true );

		if ( fow_tracker->is_enabled( c_fow_tracker::e_enabled_mode::native ) )
			args.source->set_position( args.source_sound_position );

#ifdef __TEST
		//myhero->print_chat( 0x3, "[%.1f]on_play_sound_fow2: %s | src: %s", gametime->get_time( ), args.sound_name, args.source->get_model_cstr( ) );
		m_draw.push_back( s_debug_draw(
			"snd (" + std::string( args.sound_name ) + ")",
			"src: " + args.source->get_model( ),
			args.source_sound_position,
			10.f
		) );
#endif
	}
}

auto c_fow_tracker::get_last_damage_taken( int camp_id ) -> std::optional<float>
{
	auto it = m_last_damage_taken.find( camp_id );

	if ( camp_id == neutral_camp_id::Dragon || camp_id == neutral_camp_id::Baron || camp_id == neutral_camp_id::Herlad )
	{
		auto last_t = std::max( camp_id == neutral_camp_id::Dragon ? last_aggro_dragon.time : last_aggro_herald.time, it != m_last_damage_taken.end( ) ? it->second : -200.f );
		if ( gametime->get_time( ) - last_t <= AGGRO_TIME_TOL )
			return last_t;
		else
			return std::nullopt;
	}

	if ( it == m_last_damage_taken.end( ) )
		return std::nullopt;

	if ( gametime->get_time( ) - it->second <= AGGRO_TIME_TOL )
		return it->second;
	else
		return std::nullopt;
}

auto on_notify_fow( std::uint32_t hash_name, const char* event_name, global_event_params_script params )
{
	if ( !params )
		return;

	auto event_id = params->get_argument( 0 ),
		sender_networkid = params->get_argument( 1 ),
		target_networkid = params->get_argument( 3 );

	auto src = entitylist->get_object_by_network_id( sender_networkid ),
		dest = entitylist->get_object_by_network_id( target_networkid );

	switch ( hash_name )
	{
		default: 
			break;
	}

#if defined( __TEST )
	//if ( src && src->is_valid( ) )
	//{
	//	myhero->print_chat( 0x3, "src on_notify: %s | %s", event_name, src->get_name_cstr( ) );
	//}

	//if ( dest && dest->is_valid( ) )
	//{
	//	myhero->print_chat( 0x3, "dest on_notify: %s | %s", event_name, dest->get_name_cstr( ) );
	//}
	//myhero->print_chat( 0x3, "on_notify: %s", event_name );
#endif
}

auto on_docast_fow( game_object_script sender, spell_instance_script spell ) -> void
{
	if ( !spell || spell->get_spell_data( ) )
		return;

	if ( !fow_tracker->cfg_ref || !fow_tracker->cfg_ref->get_bool( ) )
		return;

	auto name = spell->get_spell_data( )->get_name( );
	std::transform( name.begin( ), name.end( ), name.begin( ),
		[ ]( unsigned char c ) { return std::tolower( c ); } );

	if ( spell->get_last_target_id( ) )
	{
		auto target = entitylist->get_object( spell->get_last_target_id( ) );
		if ( target && target->is_valid( ) && A_::is_epic_monster( name ) )
		{
			if ( name.find( "sru_baron" ) != std::string::npos || name.find( "sru_riftherald" ) != std::string::npos )
				last_aggro_herald.time = gametime->get_time( );
			else
				last_aggro_dragon.time = gametime->get_time( );

			fow_tracker->notify_objective_attack( target );
		}
	}
}

auto on_effect_fow( game_object_script sender, const effect_create_data_client& create_data ) -> void
{
	if ( !sender || !sender->is_valid( ) )
		return;

	if ( !fow_tracker->is_enabled( ) )
		return;

	auto&& character_attachment = create_data.character_attachment;
	auto&& second_emitter = create_data.second_emitter_object; 

	if ( character_attachment && character_attachment->is_valid( ) && character_attachment->is_ai_base( ) && character_attachment->is_ai_hero( ) && 
		character_attachment->is_enemy( ) && !character_attachment->is_visible( ) && !character_attachment->is_dead( ) && sender->get_position( ).is_valid( ) &&
		( !create_data.character_attachment->get_path_controller( ) || create_data.character_attachment->get_path_controller( )->get_path_count( ) == 0 ) && 
		!create_data.character_attachment->is_dead( ) && create_data.character_attachment->get_position( ).is_valid( ) 
		)
	{
		fow_tracker->update_position( character_attachment->get_network_id( ), sender->get_position( ), true );

		if ( fow_tracker->is_enabled( c_fow_tracker::e_enabled_mode::native ) )
			character_attachment->set_position( sender->get_position( ) );

#if defined( __TEST )
		myhero->print_chat( 0x3, "[character_attachment]%s|%s (%s|%s)", character_attachment->get_name_cstr( ), character_attachment->get_model_cstr( ), 
			sender->get_name_cstr( ), sender->get_model_cstr( ) );
#endif
	}

	if ( second_emitter && second_emitter->is_valid( ) && second_emitter->is_ai_base( ) && second_emitter->is_ai_hero( ) &&
		second_emitter->is_enemy( ) && !second_emitter->is_visible( ) && !second_emitter->is_dead( ) && second_emitter->get_position( ).is_valid( ) &&
		( !second_emitter->get_path_controller( ) || second_emitter->get_path_controller( )->get_path_count( ) == 0 )  )
	{
		fow_tracker->update_position( second_emitter->get_network_id( ), second_emitter->get_position( ), true );

		if ( fow_tracker->is_enabled( c_fow_tracker::e_enabled_mode::native ) )
			second_emitter->set_position( second_emitter->get_position());

#if defined( __TEST )
		//myhero->print_chat( 0x3, "[second_emitter]%s|%s", second_emitter->get_name_cstr( ), second_emitter->get_model_cstr( ) );
#endif
	}
}

auto on_gain_buff_fow( game_object_script sender, buff_instance_script buff ) -> void
{
	if ( !sender || !sender->is_valid( ) )
		return;

	if ( !buff || !buff->is_valid( ) )
		return;

	if ( sender->is_ally( ) )
	{
		switch ( buff->get_hash_name( ) )
		{
			default:
				break;
		}
	}
}

//auto on_lose_buff_fow( game_object_script sender, buff_instance_script buff ) -> void
//{
//	
//}

//auto on_update_fow( ) -> void
//{
//	static auto t_last_update = -8.f;
//	if ( gametime->get_time( ) - t_last_update <= 0.5f )
//		return;
//
//	t_last_update = gametime->get_time( );
//}

auto on_draw_fow( ) -> void
{
	if ( keyboard_state->is_pressed( keyboard_game::tab ) )
		return;

	static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
	static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

#if defined (__TEST)
	m_draw.erase( std::remove_if( m_draw.begin( ), m_draw.end( ), [ ]( s_debug_draw d ) {
		return gametime->get_time( ) > d.time;
	} ), m_draw.end( ) );

	for ( auto&& e : m_draw )
	{
		auto base_text = e.position;
		renderer->world_to_screen( base_text, base_text );

		draw_manager->add_text_on_screen( base_text, MAKE_COLOR( 255, 255, 255, 180 ), 26, e.text.c_str( ) );
		base_text.y += draw_manager->calc_text_size( 26, e.text.c_str( ) ).y;
		draw_manager->add_text_on_screen( base_text, MAKE_COLOR( 255, 255, 255, 180 ), 13, e.text_alt.c_str( ) );
		base_text.x += draw_manager->calc_text_size( 13, "%s ", e.text_alt.c_str( ) ).x;
		draw_manager->add_text_on_screen( base_text, MAKE_COLOR( 255, 255, 255, 180 ), 13, "%.1fs", e.time - gametime->get_time( ) );
	}
#endif
	static std::map< uint32_t, float > m_mob_info = { };
	for ( auto camp_id = 0; camp_id < neutral_camp_id::Max_Camps; camp_id++ )
	{
		if ( camp_id == neutral_camp_id::Crab_Bottom || camp_id == neutral_camp_id::Crab_Top )
			continue;

		for ( auto&& i : camp_manager->get_camp_minions( camp_id ) )
		{
			auto x = entitylist->get_object_by_network_id( i );
			if ( !x || !x->is_valid( ) )
				continue;

			auto name = x->get_name( );
			std::transform( name.begin( ), name.end( ), name.begin( ),
				[ ]( unsigned char c ) { return std::tolower( c ); } );

			if ( !A_::is_smite_monster( name ) )
				continue;

			auto it = m_mob_info.find( i );
			if ( it == m_mob_info.end( ) )
			{
				m_mob_info [ i ] = -20.f;
				it = m_mob_info.find( i );
			}

			if ( !x->is_visible( ) )
			{
				if ( gametime->get_time( ) - it->second >= 30.f && !x->is_dead( ) )
				{
					auto v_camp_pos = camp_manager->get_camp_position( camp_id );
					x->set_position( v_camp_pos );
					it->second = gametime->get_time( );
				}
			}
			else
				it->second = gametime->get_time( );

			auto should_glow = fow_tracker->monster_glow->get_bool( ) && !x->is_visible( ) && !x->is_dead( );

			if ( !should_glow )
				glow->remove_glow( x );
			else
				glow->apply_glow( x, fow_tracker->monster_glow_color->get_color( ), fow_tracker->monster_glow_thickness->get_int( ), fow_tracker->monster_glow_blur->get_int( ) );
		}
	}

	if ( fow_tracker->is_enabled( c_fow_tracker::e_enabled_mode::minimap ) )
	{
		for ( auto&& x : entitylist->get_enemy_heroes( ) )
		{
			if ( !x || x->is_dead( ) || x->is_visible( ) )
				continue;

			auto data = fow_tracker->get_data( x->get_network_id( ) );

			auto t_elapsed = gametime->get_time( ) - data.t_last_update;
			if ( !data.position.is_valid( ) || t_elapsed > ( fow_tracker->max_draw_time ? fow_tracker->max_draw_time->get_int( ) : 7 ) )
				continue;

			spell_tracker::draw_map_portrait( x, data.position, true, t_elapsed );
		}
	}

	if ( !fow_tracker->draw_circle_epic || !fow_tracker->draw_circle_epic->get_bool( ) || !fow_tracker->aggro_circle_color )
		return;

	if ( fow_tracker->is_objective_under_attack( neutral_camp_id::Herlad ) &&
		( camp_manager->get_camp_alive_status( neutral_camp_id::Baron ) ||
		camp_manager->get_camp_alive_status( neutral_camp_id::Herlad ) ) )
	{
		auto sender = camp_manager->get_camp_minions( camp_manager->get_camp_alive_status( neutral_camp_id::Baron ) ? neutral_camp_id::Baron : neutral_camp_id::Herlad );
		auto pos = vector::zero, pos_end = vector::zero;

		for ( auto&& x : sender )
		{
			auto monster = entitylist->get_object_by_network_id( x );
			if ( !monster || monster->is_dead( ) || monster->is_visible( ) )
				continue;

			pos = camp_manager->get_camp_position( camp_manager->get_camp_alive_status( neutral_camp_id::Baron ) ? neutral_camp_id::Baron : neutral_camp_id::Herlad );
			pos_end = pos.extend( vector::zero, monster->get_bounding_radius( ) * 6.f );
		}

		if ( pos.is_valid( ) && pos_end.is_valid( ))
		{
			draw_manager->draw_circle_on_minimap( pos, pos.distance( pos_end ), A_::animate_color( fow_tracker->aggro_circle_color->get_color( ), 120, 255 ), 2.f );

			draw_manager->draw_circle_on_minimap( pos, fmod( pos.distance( pos_end ) * ( gametime->get_time( ) + 1.f ), pos.distance( pos_end ) ),
				A_::set_alpha( fow_tracker->aggro_circle_color->get_color( ), 120 ), 2.f );
		}
	}

	if ( fow_tracker->is_objective_under_attack( neutral_camp_id::Dragon ) &&
		( camp_manager->get_camp_alive_status( neutral_camp_id::Dragon ) ) )
	{
		auto sender = camp_manager->get_camp_minions( neutral_camp_id::Dragon );
		auto pos = vector::zero, pos_end = vector::zero;

		for ( auto&& x : sender )
		{
			auto monster = entitylist->get_object_by_network_id( x );
			if ( !monster || monster->is_dead( ) || monster->is_visible( ) )
				continue;

			pos = camp_manager->get_camp_position( neutral_camp_id::Dragon );
			pos_end = pos.extend( vector::zero, monster->get_bounding_radius( ) * 6.f );
		}

		if ( pos.is_valid( ) && pos_end.is_valid( ) )
		{
			draw_manager->draw_circle_on_minimap( pos, pos.distance( pos_end ), A_::animate_color( fow_tracker->aggro_circle_color->get_color( ), 120, 255 ), 2.f );

			draw_manager->draw_circle_on_minimap( pos, fmod( pos.distance( pos_end ) * ( gametime->get_time( ) + 1.f ), pos.distance( pos_end ) ),
				A_::set_alpha( fow_tracker->aggro_circle_color->get_color( ), 120 ), 2.f );
		}
	}
}

auto c_fow_tracker::load( ) -> void
{
	auto load_camp_timers = [ & ]( ) 
	{
		auto update_camp_respawn = [ & ]( uint32_t i ) 
		{ 
			auto camp_respawn_t = fow_tracker->camp_respawn_t( i );
			if ( camp_respawn_t != 1.f &&
				std::find( std::begin( ignore_ids ), std::end( ignore_ids ), i ) == std::end( ignore_ids ) )
				camp_manager->update_camp_respawn_time( i, ( gametime->get_time( ) < camp_respawn_t && camp_respawn_t <= 120.f ) ? camp_respawn_t : gametime->get_time( ) + camp_respawn_t );
		};

		for ( uint32_t i = neutral_camp_id::Blue_Order; i < neutral_camp_id::Max_Camps; i++ )
		{
			//myhero->print_chat( 0x3, "load->[%d]->%.1f", i, camp_manager->get_camp_respawn_time( i ) ); -1
			auto minions = camp_manager->get_camp_minions( i );
			if ( !minions.empty( ) )
			{
				auto it_alive = std::find_if( minions.begin( ), minions.end( ), [ ]( uint32_t network_id ) {
					auto obj = entitylist->get_object_by_network_id( network_id );

					return obj && obj->is_valid( ) && A_::is_monster( obj ) && !obj->is_dead( );
				} );
				if ( it_alive == minions.end( ) )
					update_camp_respawn( i );
			}
			else
				update_camp_respawn( i );
		}
	};

	load_camp_timers( );
	event_handler<events::on_draw>::add_callback( on_draw_fow );

	event_handler<events::on_network_packet>::add_callback( on_network_packet_fow );
	event_handler<events::on_create_object>::add_callback( on_create_object_fow );
	event_handler<events::on_create_client_effect>::add_callback( on_effect_fow );
	event_handler<events::on_delete_object>::add_callback( on_delete_object_fow );
	event_handler<events::on_object_dead>::add_callback( on_death_object_fow );

	event_handler<events::on_buff_gain>::add_callback( on_gain_buff_fow );
	//event_handler<events::on_buff_lose>::add_callback( on_lose_buff_fow );

	event_handler<events::on_do_cast>::add_callback( on_docast_fow );
	//event_handler<events::on_update>::add_callback( on_update_fow, event_prority::lowest );

	event_handler<events::on_play_sound>::add_callback( on_play_sound_fow );
	event_handler<events::on_global_event>::add_callback( on_notify_fow );
}

auto c_fow_tracker::unload( ) -> void
{
	event_handler<events::on_draw>::remove_handler( on_draw_fow );

	event_handler<events::on_network_packet>::remove_handler( on_network_packet_fow );
	event_handler<events::on_create_object>::remove_handler( on_create_object_fow );
	event_handler<events::on_create_client_effect>::remove_handler( on_effect_fow );
	event_handler<events::on_delete_object>::remove_handler( on_delete_object_fow );
	event_handler<events::on_object_dead>::remove_handler( on_death_object_fow );

	event_handler<events::on_buff_gain>::remove_handler( on_gain_buff_fow );
	//event_handler<events::on_buff_lose>::remove_handler( on_lose_buff_fow );

	event_handler<events::on_do_cast>::remove_handler( on_docast_fow );
	//event_handler<events::on_update>::remove_handler( on_update_fow );

	event_handler<events::on_play_sound>::remove_handler( on_play_sound_fow );
	event_handler<events::on_global_event>::remove_handler( on_notify_fow );
}