#include "notify.h"
#include "A_.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdarg>
#include <functional>

#include "cooldown_manager.hpp"
#include "utility.h"

c_notify* g_notify = new c_notify( );

auto c_notify::add_text( float duration, std::vector<s_notify_text> text ) -> void
{
	if ( e_notify_side->get_int( ) != 2 )
		m_notify.push_back( s_notify_entry ( 
			text,
			( duration <= 0.f ? static_cast<float>( e_notify_duration->get_int( ) ) : duration ),
			e_notify_side->get_int( )
			) );
	else
		m_notify.insert( m_notify.begin( ), s_notify_entry(
			text,
			( duration <= 0.f ? static_cast< float >( e_notify_duration->get_int( ) ) : duration ),
			e_notify_side->get_int( )
		) );
}

auto c_notify::add( uint32_t* icon_l, uint32_t* icon_r, float duration, std::vector<s_notify_text> text ) -> void
{
	if ( e_notify_side->get_int( ) != 2 )
		m_notify.push_back( s_notify_entry(
			text,
			( duration <= 0.f ? static_cast< float >( e_notify_duration->get_int( ) ) : duration ),
			e_notify_side->get_int( ),
			icon_l,
			icon_r
		) );
	else
		m_notify.insert( m_notify.begin( ), s_notify_entry(
			text,
			( duration <= 0.f ? static_cast< float >( e_notify_duration->get_int( ) ) : duration ),
			e_notify_side->get_int( ),
			icon_l,
			icon_r
		) );
}

auto c_notify::is_r_enabled( champion_id id ) -> bool
{
	auto it = m_r_whitelist.find( id );

	return it != m_r_whitelist.end( ) && it->second->get_bool( );
}

auto c_notify::draw( ) -> void
{
	if ( !myhero )
		return;

#pragma region track_cd_pre
	for ( auto&& x : entitylist->get_all_heroes( ) )
	{
		if ( !x || !x->is_valid( ) || !x->is_visible( ) || x->is_me( ) )
			continue;

		auto spell_r = x->get_spell( spellslot::r );
		auto spell_s1 = x->get_spell( spellslot::summoner1 ), 
			spell_s2 = x->get_spell( spellslot::summoner2 );

		if ( !spell_r || !spell_s1 || !spell_s2 )
			continue;

		std::string s1_name = "Summoner spell", s2_name = "Summoner spell";

		auto it_s1 = m_sum_names.find( spell_s1->get_name_hash( ) ), 
			it_s2 = m_sum_names.find( spell_s2->get_name_hash( ) );

		if ( spell_s1->get_name( ).find( "SummonerSmite" ) != std::string::npos )
			s1_name = "Smite";
		else if ( it_s1 != m_sum_names.end( ) )
			s1_name = it_s1->second;

		if ( spell_s2->get_name( ).find( "SummonerSmite" ) != std::string::npos )
			s2_name = "Smite";
		else if ( it_s2 != m_sum_names.end( ) )
			s2_name = it_s2->second;

		auto opt_cd_r = g_cooldown->spell_cd( x, spellslot::r );
		auto opt_cd_s1 = g_cooldown->sum_cd( x, spell_s1->get_name_hash( ) ),
			opt_cd_s2 = g_cooldown->sum_cd( x, spell_s2->get_name_hash( ) );

		auto cd_r = opt_cd_r.has_value( ) ? opt_cd_r.value( ) : spell_r->cooldown( );
		auto cd_s1 = opt_cd_s1.has_value( ) ? opt_cd_s1.value( ) : spell_s1->cooldown( );
		auto cd_s2 = opt_cd_s2.has_value( ) ? opt_cd_s2.value( ) : spell_s2->cooldown( );

		auto opt_cd_r_start = g_cooldown->spell_cd_start( x, spellslot::r );
		auto cd_r_start = opt_cd_r_start.has_value( ) ? opt_cd_r_start.value( ) : spell_r->cooldown_start( );

		auto opt_cd_s1_start = g_cooldown->sum_cd_start( x, spell_s1->get_name_hash( ) );
		auto cd_s1_start = opt_cd_s1_start.has_value( ) ? opt_cd_s1_start.value( ) : spell_s1->cooldown_start( );

		auto opt_cd_s2_start = g_cooldown->sum_cd_start( x, spell_s2->get_name_hash( ) );
		auto cd_s2_start = opt_cd_s2_start.has_value( ) ? opt_cd_s2_start.value( ) : spell_s2->cooldown_start( );

		auto it_notifier = m_cd_notify.find( x->get_network_id( ) );
		if ( it_notifier == m_cd_notify.end( ) )
		{
			m_cd_notify [ x->get_network_id( ) ] = s_spell_track {
				cd_r_start,
				true,
				-100.f,

				cd_s1_start,
				true,
				-100.f,

				cd_s2_start,
				true,
				-100.f,
			};
			continue;
		}

		{
			if ( ( ( x->is_ally( ) && e_notify_ally->get_prority( buff_hash( "ally_sums_ready" ) ).second ) ||
				( x->is_enemy( ) && e_notify_enemy->get_prority( buff_hash( "ally_sums_ready" ) ).second )
				) && cd_s1 <= 10.f && cd_s1 > 0.f &&
				!it_notifier->second.s1_notified && 
				std::find( std::begin( m_blacklisted_spells ), std::end( m_blacklisted_spells ), spell_r->get_name_hash( ) ) == std::end( m_blacklisted_spells ) &&
				!spell_r->get_spell_data( )->is_no_cooldown( ) && is_r_enabled( x->get_champion( ) ) )
			{
				if ( gametime->get_time( ) - it_notifier->second.t_last_s1_notify >= 7.f && 
					std::find( std::begin( m_blacklisted_spells ), std::end( m_blacklisted_spells ), spell_s1->get_name_hash( ) ) == std::end( m_blacklisted_spells ) )
				{
					add( x->get_square_icon_portrait( ), spell_s1->get_icon_texture( ), -1.f, {
						c_notify::s_notify_text( e_use_champion_names->get_bool( ) ? x->get_name( ) : x->get_model( ), c_notify::e_color_mode::active_text ),
						c_notify::s_notify_text( "'s ", c_notify::e_color_mode::text ),
						c_notify::s_notify_text( s1_name, c_notify::e_color_mode::active_text ),
						c_notify::s_notify_text( " is ready in ", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + cd_s1, " is " ),
						c_notify::s_notify_text( "ready", c_notify::e_color_mode::active_text, 0, gametime->get_time( ) + cd_s1 ),
						c_notify::s_notify_text( " s", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + cd_s1 ),
						} );

					it_notifier->second.s1_notified = true;
					it_notifier->second.t_last_s1_notify = gametime->get_time( );
				}
			}

			if ( ( ( x->is_ally( ) && e_notify_ally->get_prority( buff_hash( "ally_sums_ready" ) ).second ) || 
				( x->is_enemy( ) && e_notify_enemy->get_prority( buff_hash( "ally_sums_ready" ) ).second ) 
				) && cd_s2 <= 10.f && cd_s2 > 0.f &&
				!it_notifier->second.s2_notified )
			{
				if ( gametime->get_time( ) - it_notifier->second.t_last_s2_notify >= 7.f && 
					std::find( std::begin( m_blacklisted_spells ), std::end( m_blacklisted_spells ), spell_s1->get_name_hash( ) ) == std::end( m_blacklisted_spells ) )
				{
					add( x->get_square_icon_portrait( ), spell_s2->get_icon_texture( ), -1.f, {
						c_notify::s_notify_text( e_use_champion_names->get_bool( ) ? x->get_name( ) : x->get_model( ), c_notify::e_color_mode::active_text ),
						c_notify::s_notify_text( "'s ", c_notify::e_color_mode::text ),
						c_notify::s_notify_text( s2_name, c_notify::e_color_mode::active_text ),
						c_notify::s_notify_text( " is ready in ", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + cd_s2, " is " ),
						c_notify::s_notify_text( "ready", c_notify::e_color_mode::active_text, 0, gametime->get_time( ) + cd_s2 ),
						c_notify::s_notify_text( " s", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + cd_s2 ),
						} );

					it_notifier->second.s2_notified = true;
					it_notifier->second.t_last_s2_notify = gametime->get_time( );
				}
			}

			if ( ( ( x->is_ally( ) && e_notify_ally->get_prority( buff_hash( "ally_r_ready" ) ).second ) ||
				( x->is_enemy( ) && e_notify_enemy->get_prority( buff_hash( "ally_r_ready" ) ).second )
				) && cd_r <= 10.f && cd_r > 0.f &&
				!it_notifier->second.r_notified )
			{
				if ( gametime->get_time( ) - it_notifier->second.t_last_r_notify >= 7.f )
				{
					add( x->get_square_icon_portrait( ), spell_r->get_icon_texture( ), -1.f, {
						c_notify::s_notify_text( e_use_champion_names->get_bool( ) ? x->get_name( ) : x->get_model( ), c_notify::e_color_mode::active_text ),
						c_notify::s_notify_text( "'s ", c_notify::e_color_mode::text ),
						c_notify::s_notify_text( "R", c_notify::e_color_mode::active_text ),
						c_notify::s_notify_text( " is ready in ", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + cd_r, " is " ),
						c_notify::s_notify_text( "ready", c_notify::e_color_mode::active_text, 0, gametime->get_time( ) + cd_r ),
						c_notify::s_notify_text( " s", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + cd_r, "" ),
						} );

					it_notifier->second.r_notified = true;
					it_notifier->second.t_last_r_notify = gametime->get_time( );
				}
			}
		}

		if ( cd_r_start != it_notifier->second.r_cd_start )
		{
			it_notifier->second.r_cd_start = cd_r_start;
			it_notifier->second.r_notified = false;
		}

		if ( cd_s1_start != it_notifier->second.s1_cd_start )
		{
			it_notifier->second.s1_cd_start = cd_s1_start;
			it_notifier->second.s1_notified = false;
		}

		if ( cd_s2_start != it_notifier->second.s2_cd_start )
		{
			it_notifier->second.s2_cd_start = cd_s2_start;
			it_notifier->second.s2_notified = false;
		}
	}
#pragma endregion

	if ( keyboard_state->is_pressed( keyboard_game::tab ) )
		return;

	if ( m_notify.empty( ) )
		return;
	
	m_notify.erase( std::remove_if( m_notify.begin( ), m_notify.end( ), [ ]( s_notify_entry s ) {
		return s.text_size.is_valid( ) && s.time_end > 0.f && s.time_end <= gametime->get_time( ) && s.alpha <= 0.f;
	} ), m_notify.end( ) );

	const auto padding = vector( 15.f, 15.f );

	auto start_y = 50.f;

	static auto fade_out_alpha = getEasingFunction( easing_functions::EaseOutSine );
	static auto fade_in_alpha = getEasingFunction( easing_functions::EaseInSine );
	static auto move_in_pos = getEasingFunction( easing_functions::EaseInSine );
	static auto adjust_y_pos = getEasingFunction( easing_functions::EaseInSine );

	auto count = 1;
	for ( auto&& e : m_notify )
	{
		vector total_text_size = vector::zero;
		if ( !e.text_size.is_valid( ) )
		{
			e.font_sz = e_text_size->get_int( );
			
			for ( auto&& text_entry : e.text )
			{
				text_entry.text_real = text_entry.text;
				text_entry.text_size = draw_manager->calc_text_size( e.font_sz, text_entry.text.c_str( ) );

				total_text_size.x += text_entry.text_size.x;
				total_text_size.y = std::max( total_text_size.y, text_entry.text_size.y );

				switch ( text_entry.color_mode )
				{
					default:
						break;

					case e_color_mode::active_text:
						text_entry.color = e_text_color_active->get_color( );
						break;
					case e_color_mode::text:
						text_entry.color = e_text_color->get_color( );
						break;
				}
			}

			e.text_size = total_text_size;

			switch ( e.side )
			{
				default:
					e.pos = e.v_base_pos = vector( -e.text_size.x - 50.f, start_y );
					e.v_delta = vector( e.text_size.x + 50.f * 2.f, 0.f );
					break;
				case 1:
				{
					auto sz = e.text_size.x + 50.f;

					if ( e.icon1 )
						sz += e.text_size.y + padding.x;

					if ( e.icon2 )
						sz += e.text_size.y + padding.x;

					e.pos = e.v_base_pos = vector( static_cast<float>( renderer->screen_width( ) ), start_y );
					e.v_delta = vector( sz, 0.f );
					break;
				}
				case 2:
				{
					start_y = -( e.text_size.y + padding.y * 3.f );
					e.pos = e.v_base_pos = vector( renderer->screen_width( ) / 2.f - e.text_size.x / 2.f, start_y );
					break;
				}
			}
		}

		total_text_size = vector::zero;
		for ( auto&& t : e.text )
		{
			if ( t.expired_o_text( ) )
				t.text = t.text_real = t.alt_text;

			if ( t.valid_timer( ) )
			{
				auto t_remaining = std::clamp( t.timer_end - gametime->get_time( ), 0.f, 15.f );

				std::stringstream stream;
				stream << std::fixed << std::setprecision( 1 ) << t_remaining;

				t.text_real = stream.str( );
			}
			else
				t.text_real = t.text;

			if ( t.text_real.empty( ) )
				continue;

			if ( t.timer_end != -1.f || t.duration_end != -1.f )
				t.text_size = draw_manager->calc_text_size( e.font_sz, t.text_real.c_str( ) );

			total_text_size.x += t.text_size.x;
			total_text_size.y = std::max( total_text_size.y, t.text_size.y );
		}
		e.text_size = total_text_size;

#pragma region update_pos_right
		if ( e.side == 1 )
		{
			auto sz = e.text_size.x + 50.f;

			if ( e.icon1 )
				sz += e.text_size.y + padding.x;

			if ( e.icon2 )
				sz += e.text_size.y + padding.x;

			e.v_delta = vector( sz, 0.f );
		}
#pragma endregion

#pragma region animate
		if ( e.side == 0 )
		{
			if ( e.text_alpha < 1.f )
			{
				if ( e.text_alpha < 1.f && gametime->get_time( ) - e.t_last_text_alpha_anim >= 0.02f )
				{
					e.text_alpha = e.text_alpha >= 0.99f ? 1.f : fade_in_alpha( e.text_alpha );
					e.t_last_text_alpha_anim = gametime->get_time( );
				}
			}

			if ( e.text_alpha >= 1.f && e.time_end <= 0.f )
				e.time_end = e.time + gametime->get_time( );

			if ( e.fl_delta < 1.f )
			{
				if ( gametime->get_time( ) - e.t_last_delta_change >= 0.03f )
				{
					e.fl_delta = e.fl_delta > 0.99f ? 1.f : move_in_pos( e.fl_delta );
					e.t_last_delta_change = gametime->get_time( );
				}
				e.pos.x = e.v_base_pos.x + e.v_delta.x * e.fl_delta;
			}
			else
			{
				if ( gametime->get_time( ) >= e.time_end && e.alpha > 0.f && gametime->get_time( ) - e.t_last_alpha_anim >= 0.09f )
				{
					e.alpha = e.alpha <= 0.01f ? 0.f : fade_out_alpha( e.alpha - 0.001f );
					e.t_last_alpha_anim = gametime->get_time( );
				}
			}
		}
		else if ( e.side == 1 )
		{
			if ( e.fl_delta < 1.f )
			{
				e.alpha = 1.f;
				if ( gametime->get_time( ) - e.t_last_delta_change >= 0.03f )
				{
					e.fl_delta = e.fl_delta > 0.99f ? 1.f : move_in_pos( e.fl_delta );
					e.t_last_delta_change = gametime->get_time( );
				}
				e.pos.x = e.v_base_pos.x - e.v_delta.x * e.fl_delta;
			}
			else
			{
				if ( e.text_alpha < 1.f && gametime->get_time( ) - e.t_last_text_alpha_anim >= 0.02f )
				{
					e.text_alpha = e.text_alpha >= 0.99f ? 1.f : fade_in_alpha( e.text_alpha );
					e.t_last_text_alpha_anim = gametime->get_time( );
				}

				if ( e.time_end <= 0.f )
					e.time_end = e.time + gametime->get_time( );
				else
				{
					if ( gametime->get_time( ) >= e.time_end && e.alpha > 0.f && gametime->get_time( ) - e.t_last_alpha_anim >= 0.09f )
					{
						e.alpha = e.alpha <= 0.01f ? 0.f : fade_out_alpha( e.alpha - 0.001f );
						e.t_last_alpha_anim = gametime->get_time( );
					}
				}
			}
		}
		else if ( e.side == 2 )
		{
			if ( e.fl_delta < 1.f )
			{
				e.alpha = 1.f;
				if ( gametime->get_time( ) - e.t_last_delta_change >= 0.03f )
				{
					e.fl_delta = e.fl_delta > 0.99f ? 1.f : move_in_pos( e.fl_delta );
					e.t_last_delta_change = gametime->get_time( );
				}
				e.pos.y = e.v_base_pos.y + e.v_delta.y * e.fl_delta;
			}
			else
			{
				if ( e.text_alpha < 1.f && gametime->get_time( ) - e.t_last_text_alpha_anim >= 0.02f )
				{
					e.text_alpha = e.text_alpha >= 0.99f ? 1.f : fade_in_alpha( e.text_alpha );
					e.t_last_text_alpha_anim = gametime->get_time( );
				}

				if ( e.time_end > 0.f )
				{
					if ( gametime->get_time( ) >= e.time_end && e.alpha > 0.f && gametime->get_time( ) - e.t_last_alpha_anim >= 0.09f )
					{
						e.alpha = e.alpha <= 0.01f ? 0.f : fade_out_alpha( e.alpha - 0.001f );
						e.t_last_alpha_anim = gametime->get_time( );
					}
				}

				auto dest = 50.f + ( e.text_size.y + padding.y * 3.f ) * ( count - 1 );
				if ( e.pos.y == dest )
				{
					if ( e.time_end <= 0.f )
						e.time_end = e.time + gametime->get_time( );
				}
				else
				{
					if ( gametime->get_time( ) - e.t_last_step_change >= 0.008f )
					{
						e.fl_step = e.fl_step > 0.99f ? 1.f : move_in_pos( e.fl_step );
						e.t_last_step_change = gametime->get_time( );

						auto step = 0.6f + 3.f * e.fl_step;

						if ( e.pos.y < dest )
							e.pos.y = e.pos.y + step > dest ? dest : e.pos.y + step;
						else
							e.pos.y = e.pos.y - step < dest ? dest : e.pos.y - step;

						if ( e.pos.y == dest )
							e.fl_step = 0.001f;
					}	
				}
			}
		}
#pragma endregion

#pragma region move_y_on_collapse
		if ( e.side == 0 || e.side == 1 )
		{
			if ( e.pos.y > start_y )
			{
				if ( gametime->get_time( ) - e.t_step_removal_anim >= 0.03f && e.fl_y_adjustment < 1.f )
				{
					e.fl_y_adjustment = e.fl_y_adjustment > 0.99f ? 1.f : adjust_y_pos( e.fl_y_adjustment );
					e.t_step_removal_anim = gametime->get_time( );
				}

				e.pos.y = e.pos.y - 5.f * e.fl_y_adjustment < start_y ? start_y : e.pos.y - 5.f * e.fl_y_adjustment;
			}
		}
#pragma endregion

#pragma region sizes
		auto start_pos = e.pos;
		auto extra_size = vector::zero;
		if ( e.icon1 )
		{
			start_pos.x += padding.x;
			
			start_pos.x += e.text_size.y + padding.x / 2.f;
		}

		if ( e.icon2 )
		{
			extra_size.x += padding.x;

			extra_size.x += e.text_size.y + padding.x / 2.f;
		}

		extra_size.y = 0.f;
		auto v_rect_start = e.pos - padding;
		auto v_rect_end = start_pos + e.text_size + padding + extra_size;
		auto v_rect_height = v_rect_end.y - v_rect_start.y;

		draw_manager->add_filled_rect( v_rect_start, v_rect_end,
			A_::set_alpha( e_background_color->get_color( ), 
				std::min( static_cast< unsigned int >( e.alpha * 255.f ), 
					( e_background_color->get_color( ) >> 24 ) & 0XFF ) ), 90.f );
#pragma endregion

#pragma region icons
		if ( e.icon1 )
		{
			auto v0 = vector( e.text_size.y + padding.x * 1.5f, padding.x / 2.f );
			auto sz_icon = vector( e.text_size.y + padding.y, e.text_size.y + padding.y );

			draw_manager->add_image( e.icon1, start_pos - v0, 
				sz_icon, 90.f, { 0.f, 0.f }, { 1.f, 1.f }, { 1.f, 1.f, 1.f, e.alpha < 0.99f ? e.alpha : e.text_alpha } );

			draw_manager->add_circle_on_screen( start_pos - v0 + sz_icon / 2.f, sz_icon.x / 2.f, 
				MAKE_COLOR( 53, 59, 72, static_cast<int>( ( e.alpha < 0.99f ? e.alpha : e.text_alpha ) * 255.f ) ), 3.f );

			auto v_arc_points = A_::arc_points( std::clamp( ( e.time_end - gametime->get_time( ) ) / e.time, 0.f, 1.f ), start_pos - v0 + sz_icon / 2.f, sz_icon.x / 2.f );
			if ( v_arc_points.size( ) > 2 )
			{
				for ( auto i = 0; i < v_arc_points.size( ) - 1; i++ )
					draw_manager->add_line_on_screen( v_arc_points [ i ], v_arc_points [ i + 1 ], A_::set_alpha( e_text_color_active->get_color( ), static_cast< int >( ( e.alpha < 0.99f ? e.alpha : e.text_alpha ) * 255.f ) ), 3.f );
			}
		}

		if ( e.icon2 )
		{
			auto v0 = vector( e.text_size.x + padding.x * 0.5f, -padding.x / 2.f );
			auto sz_icon = vector( e.text_size.y + padding.y, e.text_size.y + padding.y );

			draw_manager->add_image( e.icon2, start_pos + v0, sz_icon, 90.f, { 0.f, 0.f }, { 1.f, 1.f }, { 1.f, 1.f, 1.f, e.alpha < 0.99f ? e.alpha : e.text_alpha } );

			draw_manager->add_circle_on_screen( start_pos + v0 + sz_icon / 2.f, sz_icon.x / 2.f, 
				MAKE_COLOR( 53, 59, 72, static_cast< int >( ( e.alpha < 0.99f ? e.alpha : e.text_alpha ) * 255.f ) ), 3.f );
		}
#pragma endregion

		if ( e.text_alpha > 0.01f )
		{
			auto v_text = start_pos;
			for ( auto&& t : e.text )
			{
				if ( t.text_real.empty( ) )
					continue;

				draw_manager->add_text_on_screen( v_text,
					A_::set_alpha( t.color, std::min( static_cast< unsigned int >( ( e.alpha < 0.99f ? e.alpha : e.text_alpha ) * 255.f ), ( t.color >> 24 ) & 0XFF ) ),
					e.font_sz, t.text_real.c_str( ) );

				v_text.x += t.text_size.x;
			}
		}

		start_y += e.text_size.y + padding.y * 3.f;
		count++;

		if ( count >= 6 /*&& e.side == 2*/ )
			break;
	}
}

auto c_notify::process_spell_cast( game_object_script sender, spell_instance_script spell ) -> void
{
	if ( !sender->is_ai_hero( ) )
		return;

	auto data = spell->get_spell_data( );
	if ( !data )
		return;

	auto is_sum = ( spell->get_spellslot( ) == spellslot::summoner1 || spell->get_spellslot( ) == spellslot::summoner2 );
	auto is_r = spell->get_spellslot( ) == spellslot::r && !data->is_no_cooldown( ) && 
		std::find( std::begin( m_blacklisted_spells ), std::end( m_blacklisted_spells ), data->get_name_hash( ) ) == std::end( m_blacklisted_spells ) &&
		is_r_enabled( sender->get_champion( ) );

	if ( data->get_name_hash( ) == spell_hash( "SummonerFlashPerksHextechFlashtraptionV2" ) )
		return;

	if ( is_r )
	{
		auto it_m = m_last_process_cast.find( data->get_name_hash( ) );
		auto t_last_notify = it_m != m_last_process_cast.end( ) ? it_m->second : -99.f;

		if ( gametime->get_time( ) - t_last_notify <= 3.f )
			return;

		m_last_process_cast [ data->get_name_hash( ) ] = gametime->get_time( );
	}

	std::string spell_name = "R";
	auto it_name = m_sum_names.find( data->get_name_hash( ) );
	if ( spell->get_spellslot( ) == spellslot::summoner1 || spell->get_spellslot( ) == spellslot::summoner2 )
	{
		if ( data->get_name( ).find( "SummonerSmite" ) != std::string::npos )
			spell_name = "Smite";
		else
			spell_name = it_name != m_sum_names.end( ) ? it_name->second : "sum 1";
	}

	auto icon = sender->get_spell( spell->get_spellslot( ) ) ? sender->get_spell( spell->get_spellslot( ) )->get_icon_texture( ) : nullptr;

	if ( sender->is_ally( ) 
#ifndef __TEST
	&& !sender->is_me( ) 
#endif
		)
	{
		if ( is_sum && e_notify_ally->get_prority( buff_hash( "ally_sums_cast" ) ).second )
		{
			add( sender->get_square_icon_portrait( ), icon, -1.f, {
				c_notify::s_notify_text( e_use_champion_names->get_bool( ) ? sender->get_name( ) : sender->get_model( ), c_notify::e_color_mode::active_text ),
				c_notify::s_notify_text( " has casted ", c_notify::e_color_mode::text ),
				c_notify::s_notify_text( spell_name, c_notify::e_color_mode::active_text )
				} );
		}

		if ( is_r && e_notify_ally->get_prority( buff_hash( "ally_r_cast" ) ).second )
		{
			add( sender->get_square_icon_portrait( ), icon, -1.f, {
				c_notify::s_notify_text( e_use_champion_names->get_bool( ) ? sender->get_name( ) : sender->get_model( ), c_notify::e_color_mode::active_text ),
				c_notify::s_notify_text( " has casted ", c_notify::e_color_mode::text ),
				c_notify::s_notify_text( spell_name, c_notify::e_color_mode::active_text )
				} );
		}
	}
	else if ( sender->is_enemy( ) )
	{
		if ( is_sum && e_notify_enemy->get_prority( buff_hash( "ally_sums_cast" ) ).second )
		{
			add( sender->get_square_icon_portrait( ), icon, -1.f, {
				c_notify::s_notify_text( e_use_champion_names->get_bool( ) ? sender->get_name( ) : sender->get_model( ), c_notify::e_color_mode::active_text ),
				c_notify::s_notify_text( " has casted ", c_notify::e_color_mode::text ),
				c_notify::s_notify_text( spell_name, c_notify::e_color_mode::active_text )
				} );
		}

		if ( is_r && e_notify_enemy->get_prority( buff_hash( "ally_r_cast" ) ).second )
		{
			add( sender->get_square_icon_portrait( ), icon, -1.f, {
				c_notify::s_notify_text( e_use_champion_names->get_bool( ) ? sender->get_name( ) : sender->get_model( ), c_notify::e_color_mode::active_text ),
				c_notify::s_notify_text( " has casted ", c_notify::e_color_mode::text ),
				c_notify::s_notify_text( spell_name, c_notify::e_color_mode::active_text )
				} );
		}
	}
}

namespace notify_callbacks
{
	auto on_draw( ) -> void
	{
		g_notify->draw( );
	}

	auto on_process_spell_cast( game_object_script sender, spell_instance_script spell ) -> void
	{
		if ( !sender || !sender->is_valid( ) || !spell )
			return;

		g_notify->process_spell_cast( sender, spell );
	}

	auto on_vote( const on_vote_args& args ) -> void
	{
		if ( args.vote_type != on_vote_type::surrender )
			return;

		if ( !g_notify->e_notify_vote->get_bool( ) )
			return;

		auto&& sender = args.sender;
		if ( !sender || !sender->is_valid( ) )
			return;

		g_notify->add( sender->get_square_icon_portrait( ), nullptr, -1.f, {
			c_notify::s_notify_text( g_notify->e_use_champion_names->get_bool( ) ? sender->get_name( ) : sender->get_model( ), c_notify::e_color_mode::active_text ),
			c_notify::s_notify_text( " has voted ", c_notify::e_color_mode::text ),
			c_notify::s_notify_text( args.success ? "YES" : "NO", c_notify::e_color_mode::custom_color, !args.success ? utility::COLOR_RED : utility::COLOR_LGREEN )
			} );
	}
}

auto c_notify::load( ) -> void
{
	auto main_tab = menu->get_tab( "utility" );
	if ( !main_tab )
		return;

	auto notifier_tab = main_tab->get_tab( "notifier" );
	if ( !notifier_tab )
		return;

	notifier_tab->add_separator( "notify_sep", " - -  Notify  - - " );
	e_notify_vote = notifier_tab->add_checkbox( "e_notify_vote", "Surrender vote", true );

	e_notify_ally = notifier_tab->add_prority_list( "e_notify_ally", "Ally", 
		{
			{ buff_hash( "ally_sums_ready" ), "Summoner spell near ready", false, nullptr },
			{ buff_hash( "ally_sums_cast" ), "Summoner spell cast", false, nullptr },

			{ buff_hash( "ally_r_ready" ), "Ultimate near ready", false, nullptr },
			{ buff_hash( "ally_r_cast" ), "Ultimate cast", false, nullptr }
		}, false );

	e_notify_enemy = notifier_tab->add_prority_list( "e_notify_enemy", "Enemy",
		{
			{ buff_hash( "ally_sums_ready" ), "Summoner spell near ready", true, nullptr },
			{ buff_hash( "ally_sums_cast" ), "Summoner spell cast", true, nullptr },

			{ buff_hash( "ally_r_ready" ), "Ultimate near ready", true, nullptr },
			{ buff_hash( "ally_r_cast" ), "Ultimate cast", true, nullptr }
		}, false );
	e_use_champion_names = notifier_tab->add_checkbox( "e_use_champion_names", "Use champion names", false );
	auto r_wl_tab = notifier_tab->add_tab( "r_wl_tab", "R whitelist" );
	auto heroes = entitylist->get_all_heroes( );
	std::sort( heroes.begin( ), heroes.end( ), [ ]( game_object_script x1, game_object_script x2 ) {
		return x1 && x1->is_valid( ) && x1->is_enemy( ) && x2 && x2->is_valid( ) && !x2->is_enemy( );
	} );

	auto b_sep = false;
	e_sep_wl_r_allies = r_wl_tab->add_separator( "r_wl_tab_enemies", " - -  Enemies  - - " );
	for ( auto&& x : heroes )
	{
		if ( x && x->is_valid( ) && !x->is_me( ) )
		{
			if ( !b_sep && x->is_ally( ) )
			{
				r_wl_tab->add_separator( "r_wl_tab_allies", " - -  Allies  - - " );
				b_sep = true;
			}
			m_r_whitelist [ x->get_champion( ) ] = r_wl_tab->add_checkbox( "r_wl_" + std::to_string( static_cast< int >( x->get_champion( ) ) ), x->get_model( ), true );
			m_r_whitelist [ x->get_champion( ) ]->set_texture( x->get_square_icon_portrait( ) );
		}
	}

	auto e_notify_ally_click = [ ]( TreeEntry* s ) {
		auto enabled_ally_r = s->get_prority( buff_hash( "ally_r_ready" ) ).second || s->get_prority( buff_hash( "ally_r_cast" ) ).second;

		for ( auto&& x : entitylist->get_all_heroes( ) )
		{
			if ( x && x->is_valid( ) && !x->is_me( ) && x->is_ally( ) )
			{
				auto it = g_notify->m_r_whitelist.find( x->get_champion( ) );
				if ( it != g_notify->m_r_whitelist.end( ) )
					it->second->is_hidden( ) = !enabled_ally_r;
			}
		}

		g_notify->e_sep_wl_r_allies->is_hidden( ) = !enabled_ally_r;
	};
	e_notify_ally_click( e_notify_ally );
	e_notify_ally->add_property_change_callback( e_notify_ally_click );

	e_notify_jungle = notifier_tab->add_prority_list( "e_notify_jungle", "Jungle (in FoW)",
		{
			{ buff_hash( "dead_small" ), "Small monster death", false, nullptr },
			{ buff_hash( "dead_big" ), "Big monster death", true, nullptr },

			{ buff_hash( "objective_attack" ), "Objective attack", true, nullptr },
		}, false );

	e_notify_duration = notifier_tab->add_slider( "e_notify_duration", "Duration (s)", 6, 2, 15 );

	notifier_tab->add_separator( "colors_sep", " - -  Colors  - - " );
	static float clr_active_color [ 4 ] = { 52.f / 255.f, 73.f / 255.f, 94.f / 255.f, 0.48f };
	static float clr_keybind_active_color [ 4 ] = { 29.f / 255.f, 209.f / 255.f, 161.f / 255.f, 1.f };
	static float clr_text_color [ 4 ] = { 245.f / 255.f, 246.f / 255.f, 250.f / 255.f, 0.95f };

	e_notify_side = notifier_tab->add_combobox( "e_notify_side", "Side", { { "Left", nullptr }, { "Right", nullptr }, { "Center", nullptr } }, 0 );
	e_text_size = notifier_tab->add_slider( "text_size", "Text size", static_cast< uint32_t >( 18.f * renderer->get_dpi_factor( ) ), 12, 50 );
	e_text_color = notifier_tab->add_colorpick( "e_text_color", "Text color", clr_text_color );
	e_text_color_active = notifier_tab->add_colorpick( "e_text_color_active", "Active color", clr_keybind_active_color );
	e_background_color = notifier_tab->add_colorpick( "e_background_color", "Background color", clr_active_color );

	notifier_tab->add_button( "btn_preset_purple", "Purple color preset" )->add_property_change_callback( [ ]( TreeEntry* ) {
		//static float clr_active_color [ 4 ] = { 47, 49, 165, 255 };
		static float clr_background_color [ 4 ] = { 47.f / 255.f, 49.f / 255.f, 145.f / 255.f, 0.48f };
		static float clr_active_text_color [ 4 ] = { 179.f / 255.f, 55.f / 255.f, 113.f / 255.f, 255.f / 255.f };
		static float clr_text_color [ 4 ] = { 245.f / 255.f, 246.f / 255.f, 250.f / 255.f, 255.f / 255.f };

		g_notify->e_text_color->set_color( clr_text_color );
		g_notify->e_text_color_active->set_color( clr_active_text_color );
		g_notify->e_background_color->set_color( clr_background_color );
	} );

	auto icon = draw_manager->load_texture_from_file( L"/main_icon.png" );
	add( icon ? icon->texture : nullptr, myhero->get_square_icon_portrait( ), 4.f,
		{ 
			c_notify::s_notify_text( "Loaded ", c_notify::e_color_mode::text ),
			c_notify::s_notify_text( "", c_notify::e_color_mode::active_text ),
			c_notify::s_notify_text( "Utility", c_notify::e_color_mode::text ),
		} );

#if defined ( __TEST )
	/*for ( auto i = 0; i <= 20; i++ )
		add( icon ? icon->texture : nullptr, myhero->get_square_icon_portrait( ), 8.f, {
			c_notify::s_notify_text( "Someone", c_notify::e_color_mode::active_text ),
			c_notify::s_notify_text( "'s ", c_notify::e_color_mode::text ),
			c_notify::s_notify_text( "some spell", c_notify::e_color_mode::active_text ),
			c_notify::s_notify_text( " is ready in ", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + 6.f, " is " ),
			c_notify::s_notify_text( "ready", c_notify::e_color_mode::active_text, 0, gametime->get_time( ) + 6.f ),
			c_notify::s_notify_text( " s", c_notify::e_color_mode::text, 0, -1.f, gametime->get_time( ) + 6.f ),
			} );*/
#endif

	event_handler<events::on_draw>::add_callback( notify_callbacks::on_draw );
	event_handler<events::on_process_spell_cast>::add_callback( notify_callbacks::on_process_spell_cast );
	event_handler<events::on_vote>::add_callback( notify_callbacks::on_vote );
}

auto c_notify::unload( ) -> void
{
	event_handler<events::on_draw>::remove_handler( notify_callbacks::on_draw );
	event_handler<events::on_process_spell_cast>::remove_handler( notify_callbacks::on_process_spell_cast );
	event_handler<events::on_vote>::remove_handler( notify_callbacks::on_vote );
}