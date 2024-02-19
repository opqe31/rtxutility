#include "dash_tracker.h"
#include "A_.h"

c_dash_tracker* g_dash_tracker = new c_dash_tracker( );

c_dash_tracker::c_dash_tracker( )
{
	m_clamp_spells = 
	{
		
	};

	m_spell_slots = 
	{
		{ spellslot::q, "Q" },
		{ spellslot::w, "W" },
		{ spellslot::e, "E" },
		{ spellslot::r, "R" }
	};
}

auto c_dash_tracker::get_display_slot( spellslot spell_slot ) -> std::string
{
	return m_spell_slots.find( spell_slot ) != m_spell_slots.end( ) ? " " + m_spell_slots[ spell_slot ] : "";
}

auto c_dash_tracker::clamp( vector& v_from, vector& v_to, std::uint32_t hash ) -> void
{
	if ( m_clamp_spells.find( hash ) != m_clamp_spells.end( ) && v_from.distance( v_to ) > m_clamp_spells [ hash ] )
		v_to = v_from.extend( v_to, m_clamp_spells [ hash ] );
}

auto c_dash_tracker::get_color( std::map<std::uint32_t, TreeEntry*> color_map, std::uint32_t hash ) -> unsigned long
{
	auto color_entry = color_map.find( hash ) == color_map.end( ) ? color_map [ 0 ] : color_map [ hash ];
	return color_entry ? color_entry->get_color( ) : MAKE_COLOR( 223, 249, 251, 255 );
}

auto c_dash_tracker::on_process_spell_cast( game_object_script sender, spell_instance_script spell, std::map<std::uint32_t, TreeEntry*> color_map ) -> void
{
	if ( !sender || !sender->is_valid( ) || !sender->is_ai_hero( ) || !spell || !spell->get_spell_data( ) )
		return;

	auto hash = spell->get_spell_data( )->get_name_hash( );
	switch ( hash )
	{
		default:
			break;

		case spell_hash( "kaisae" ):
		{
			auto v_from = spell->get_start_position( ),
				v_to = vector::zero; // spell->get_end_position( );

			m_dashes.push_back( s_dash {
				v_from,
				v_to,
				gametime->get_time( ),

				sender->get_model( ) + get_display_slot( spell->get_spellslot( ) ),
				sender->is_enemy( ),

				spell->get_spell_data( )->get_icon_texture_by_index( 0 ),
				sender->get_square_icon_portrait( ),
				std::clamp( sender->get_bounding_radius( ), 50.f, 85.f ),
				get_color( color_map, hash )
				} );
			break;
		}

		case spell_hash( "summonerflash" ):
		{
			auto v_from = spell->get_start_position( ),
				v_to = spell->get_end_position( );

			clamp( v_from, v_to, hash );

			m_dashes.push_back( s_dash {
				v_from,
				v_to,
				gametime->get_time( ),

				sender->get_model( ) + " Flash",
				sender->is_enemy( ),
				
				spell->get_spell_data( )->get_icon_texture_by_index( 0 ),
				sender->get_square_icon_portrait( ),
				std::clamp( sender->get_bounding_radius( ), 50.f, 85.f ),
				get_color( color_map, hash )
				} );
			break;
		}

		case spell_hash( "alphastrike" ):
		{
			auto v_from = spell->get_start_position( ),
				v_to = spell->get_end_position( );

			clamp( v_from, v_to, hash );

			//if ( hash == spell_hash( "vaynetumble" ) && !sender->has_buff( buff_hash( "VayneInquisition" ) ) )
			//	break;

			m_dashes.push_back( s_dash {
				v_from,
				v_to,
				gametime->get_time( ),

				sender->get_model( ) + get_display_slot( spell->get_spellslot( ) ),
				sender->is_enemy( ),

				spell->get_spell_data( )->get_icon_texture_by_index( 0 ),
				sender->get_square_icon_portrait( ),
				std::clamp( sender->get_bounding_radius( ), 50.f, 85.f ),
				get_color( color_map, hash )
				} );

			break;
		}
	}
}

auto c_dash_tracker::on_tick( float time ) -> void
{
	if ( m_dashes.empty( ) )
		return;

	m_dashes.erase( std::remove_if( m_dashes.begin( ), m_dashes.end( ), [ ]( s_dash dash ) {
		return dash.size_set && dash.removal && dash.f_current_alpha < 0.01f && ( dash.current_size.x <= 0 || dash.current_size.y <= 0 );
	} ), m_dashes.end( ) );
}

auto c_dash_tracker::animate_height( float start_t, float end_t, float dest_pos, float current_pos, float ct, s_dash& dash ) -> float
{
	if ( dash.t_last_anim_3_1 == -1.f )
	{
		dash.t_last_anim_3_1 = 0.001f;
	}
	static auto easing_fn = getEasingFunction( EaseInSine );

	if ( gametime->get_time( ) - dash.t_last_anim_3 > 0.03f )
	{
		dash.t_last_anim_3_1 = easing_fn( dash.t_last_anim_3_1 );
		dash.t_last_anim_3 = gametime->get_time( );
	}

	ct = std::max( start_t, std::min( end_t, ct ) );
	//auto nomalized_time = ( ct - start_t ) / ( end_t - start_t );
	auto interpolated_position = current_pos + ( dest_pos - current_pos ) * dash.t_last_anim_3_1; //normalized time
	//myhero->print_chat( 0x3, "%.4f | %.4f | %.4f", current_pos, ( dest_pos - current_pos ), nomalized_time );

	return interpolated_position;
}

auto c_dash_tracker::on_draw( bool draw_allies, bool draw_enemies, float max_duration, unsigned long clr_arc, float sz ) -> void
{
	if ( m_dashes.empty( ) || ( !draw_allies && !draw_enemies ) )
		return;

	//auto dashes = m_dashes;
	for ( auto&& dash : m_dashes ) 
	{
		if ( dash.is_enemy && !draw_enemies )
			continue;

		if ( !dash.is_enemy && !draw_allies )
			continue;

		if ( dash.t_end_anim == -1.f )
			dash.t_end_anim = dash.t_start + 1.2f;

		if ( ( ( dash.color >> 24 ) & 0XFF ) <= 0 )
			continue;

		if ( !dash.size_set ) 
		{
			dash.current_size = vector( dash.bounding_radius, dash.bounding_radius );
			dash.size_set = true;
			dash.f_current_alpha = 0.001f;
			dash.t_end_anim_alpha = gametime->get_time( ) + 1.f;
		}

		//fade in alpha
		if ( !dash.removal && gametime->get_time( ) - dash.t_last_anim_0 >= 0.02f )
		{
			if ( dash.f_current_alpha < 1.f )
			{
				static auto easing_fn = getEasingFunction( EaseInSine );
				dash.f_current_alpha = dash.f_current_alpha > 0.99f ? 1.f : easing_fn( dash.f_current_alpha );
				dash.t_last_anim_0 = gametime->get_time( );
			}
			else
				dash.removal = true;
		}

		//shrink out size
		if ( dash.t_start + max_duration <= gametime->get_time( ) && dash.removal )
		{
			static auto easing_fn = getEasingFunction( EaseOutExpo );
			{
				dash.current_size.x *= easing_fn( dash.current_size.x / dash.bounding_radius );
				dash.current_size.y *= easing_fn( dash.current_size.y / dash.bounding_radius );
				dash.t_last_anim_1 = gametime->get_time( );
			}
		}

		//fade out alpha
		if ( dash.t_start + max_duration <= gametime->get_time( ) && dash.f_current_alpha > 0.01f && dash.removal && 
			gametime->get_time( ) - dash.t_last_anim_2 >= 0.02f )
		{
			static auto easing_fn = getEasingFunction( EaseOutSine );
			{
				dash.f_current_alpha = easing_fn( dash.f_current_alpha - 0.01f );
				dash.t_last_anim_2 = gametime->get_time( );
			}
		}

		static auto anim_range_z = 33.f;
		auto v_start = vector( dash.v_start.x, dash.v_start.y, animate_height( dash.t_start, dash.t_end_anim, dash.v_start.z, dash.v_start.z - anim_range_z, gametime->get_time( ), dash ) );
		auto v_end = vector( dash.v_end.x, dash.v_end.y, animate_height( dash.t_start, dash.t_end_anim, dash.v_end.z, dash.v_end.z - anim_range_z, gametime->get_time( ), dash ) );

		if ( dash.v_start.is_valid( ) )
		{
			//draw_manager->add_circle_with_glow( v_start, dash.color, dash.bounding_radius, 2.f, glow_data( 0.82f, 0.45f, 1.f, 0.33f ) );

			auto w2s_start = vector::zero,
				w2s_end = vector::zero;

			renderer->world_to_screen( v_start, w2s_start );
			renderer->world_to_screen( v_end, w2s_end );

			if ( !w2s_start.is_valid( ) || !w2s_end.is_valid( ) )
				continue;

			if ( dash.icon_spell )
			{
				auto size = dash.current_size * ( sz / 100.f );
				draw_manager->add_image( dash.icon_spell, w2s_start - size / 2.f, size, 90.f, vector::zero, {1.f, 1.f }, { 1.f, 1.f, 1.f, dash.f_current_alpha } );

				auto t_elapsed = gametime->get_time( ) - dash.t_start;
				auto v_arc_points = A_::arc_points( std::clamp( 1.f - t_elapsed / max_duration, 0.f, 1.f ), w2s_start, size.x / 2 );

				draw_manager->add_circle_on_screen( w2s_start, size.x / 2, MAKE_COLOR( 53, 59, 72, (int)( 220 * dash.f_current_alpha ) ), 3.f );
				if ( v_arc_points.size( ) > 2 )
				{
					for ( auto i = 0; i < v_arc_points.size( ) - 1; i++ ) 
						draw_manager->add_line_on_screen( v_arc_points [ i ], v_arc_points [ i + 1 ], A_::set_alpha( clr_arc, static_cast< uint32_t >( 255.f * dash.f_current_alpha ) ), 3.f );
				}

				if ( dash.icon_sender )
					draw_manager->add_image( dash.icon_sender, w2s_start + vector( 0, -size.y + size.y / max_duration ), size / 2.f, 90.f, vector::zero, { 1.f, 1.f }, { 1.f, 1.f, 1.f, dash.f_current_alpha } );
			}

			if ( v_end.is_valid( ) && v_start.distance( v_end ) > dash.bounding_radius )
			{
				auto v_start_line = v_start.extend( v_end, dash.bounding_radius );
				draw_manager->add_line( v_start_line, v_end, A_::set_alpha( dash.color, static_cast< uint32_t >( 255.f * dash.f_current_alpha ) ), 2.f );

				auto l_arrow = v_end + ( v_end - v_start ).rotated( degrees_to_radians( 180 - 45 ) ),
					r_arrow = v_end + ( v_end - v_start ).rotated( degrees_to_radians( 180 + 45 ) );

				draw_manager->add_line( v_end, v_end.extend( l_arrow, 45.f ), A_::set_alpha( dash.color, static_cast< uint32_t >( 255.f * dash.f_current_alpha ) ), 2.f );
				draw_manager->add_line( v_end, v_end.extend( r_arrow, 45.f ), A_::set_alpha( dash.color, static_cast<uint32_t>( 255.f * dash.f_current_alpha ) ), 2.f );
			}
		}
	}
}