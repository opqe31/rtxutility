#include "side.hpp"
#include "A_.h"
#include "cooldown_manager.hpp"

#include "utility.h"

#include <sstream>
#include <iomanip>

c_side_hud* g_side_hud = new c_side_hud( );

auto c_side_hud::create_menu( ) -> void
{
	auto main_tab = menu->get_tab( "utility" );
	if ( !main_tab )
		return;

	auto side_tab = main_tab->get_tab( "sidehud" );
	if ( !side_tab )
		return;

	side_tab->add_separator( "side_sep_main", " - -  Side HUD  - - " );
	enabled = side_tab->add_checkbox( "enabled", "Enabled", true );
	hud_size = side_tab->add_slider( "hud_size", "Size", static_cast<int32_t>( std::clamp( renderer->get_dpi_factor( ) * 100.f, 25.f, 350.f ) ), 25, 350 );
	sidehud_side = side_tab->add_combobox( "sidehud_side", "Side", { { "Right", nullptr }, { "Left", nullptr }, { "Bottom", nullptr } }, 0 );
	pos_x = side_tab->add_slider( "pos_x", "pos_x", 100, 0, static_cast< int32_t >( renderer->screen_width( ) ) );
	pos_y = side_tab->add_slider( "pos_y", "pos_y", 100, 0, static_cast< int32_t >( renderer->screen_width( ) ) );

	side_tab->add_separator( "side_sep_pings", " - -  Pings  - - " );
	ping_key = side_tab->add_hotkey( "ping_key", "Ping key", TreeHotkeyMode::Hold, 0x02, false ); ping_key->set_tooltip( "Hover something -> Press the key -> You have successfuly pinged!" );
	spell_ready_ping_type = side_tab->add_combobox( "spell_ready_ping_type", "Spell ready ping type", { { "Game ping", nullptr }, { "Chat message", nullptr } }, 0 );

	spellready_message_text = side_tab->add_text_input( "spellready_message_text", " ^ - Text", "{champion} has {spell}" ); 
	button_champion_1 = side_tab->add_button( "button_champion_1", "{champion}" );
	button_spell_1 = side_tab->add_button( "button_spell_1", "{spell}" );

	button_champion_1->add_property_change_callback( [ ]( TreeEntry* ) {
		g_side_hud->spellready_message_text->set_string( g_side_hud->spellready_message_text->get_string( ) + "{champion}" );
	} );
	button_spell_1->add_property_change_callback( [ ]( TreeEntry* ) {
		g_side_hud->spellready_message_text->set_string( g_side_hud->spellready_message_text->get_string( ) + "{spell}" );
	} );

	spell_cooldown_type = side_tab->add_combobox( "spell_cooldown_type", "Spell cooldown ping type", { { "Game ping", nullptr }, { "Chat message", nullptr } }, 0 );
	spellcooldown_message_text = side_tab->add_text_input( "spellcooldown_message_text", " ^ - Text", "{champion} {spell}, {seconds}s" );

	button_champion_2 = side_tab->add_button( "button_champion_2", "{champion}" );
	button_spell_2 = side_tab->add_button( "button_spell_2", "{spell}" );
	button_seconds_2 = side_tab->add_button( "button_seconds_2", "{seconds}" );
	button_m_2 = side_tab->add_button( "button_m_2", "{m}" );
	button_s_2 = side_tab->add_button( "button_s_2", "{s}" );

	button_champion_2->add_property_change_callback( [ ]( TreeEntry* ) {
		g_side_hud->spellcooldown_message_text->set_string( g_side_hud->spellcooldown_message_text->get_string( ) + "{champion}" );
	} );
	button_spell_2->add_property_change_callback( [ ]( TreeEntry* ) {
		g_side_hud->spellcooldown_message_text->set_string( g_side_hud->spellcooldown_message_text->get_string( ) + "{spell}" );
	} );
	button_seconds_2->add_property_change_callback( [ ]( TreeEntry* ) {
		g_side_hud->spellcooldown_message_text->set_string( g_side_hud->spellcooldown_message_text->get_string( ) + "{seconds}" );
	} );
	button_m_2->add_property_change_callback( [ ]( TreeEntry* ) {
		g_side_hud->spellcooldown_message_text->set_string( g_side_hud->spellcooldown_message_text->get_string( ) + "{m}" );
	} );
	button_s_2->add_property_change_callback( [ ]( TreeEntry* ) {
		g_side_hud->spellcooldown_message_text->set_string( g_side_hud->spellcooldown_message_text->get_string( ) + "{s}" );
	} );

	spellcooldown_message_text->set_tooltip( "{champion} = will be replaced with champion name, {spell} with spell name, {seconds} with total spell cooldown (in seconds), {m} - minutes, {s} - seconds" );
	spellready_message_text->set_tooltip( "{champion} = will be replaced with champion name, {spell} with spell name" );

	auto tab_spellnames = side_tab->add_tab( "tab_spellnames", "Spell names" );
	{
		tab_spellnames->add_separator( "spellnames_sep", " - -  Spell names  - - " );
		spellname_r = tab_spellnames->add_text_input( "spellname_r", "R", "R" );
		const std::map<uint32_t, std::pair<std::string, std::string> > m_spells =
		{
			{ spell_hash( "SummonerSmite" ), { "Smite", "Smite" } },
			{ spell_hash( "SummonerExhaust" ), { "Exhaust", "Exhaust" } },
			{ spell_hash( "SummonerHaste" ), { "Ghost", "Ghost" } },
			{ spell_hash( "SummonerBoost" ), { "Cleanse", "Cleanse" } },
			{ spell_hash( "SummonerDot" ), { "Ignite", "Ignite" } },
			{ spell_hash( "SummonerHeal" ), { "Heal", "H" } },
			{ spell_hash( "SummonerBarrier" ), { "Barrier", "B" } },
			{ spell_hash( "SummonerFlash" ), { "Flash", "F" } }
		};

		for ( auto&& p : m_spells )
			spellname_other[ p.first ] = tab_spellnames->add_text_input( "spellname_" + p.second.first, p.second.first, p.second.second );
	}
	auto champion_spellnames = side_tab->add_tab( "tab_champnames", "Champion names" );
	{
		champion_spellnames->add_separator( "champ_sep", " - -  Champion names  - - " );
		for ( auto&& x : entitylist->get_all_heroes( ) )
		{
			if ( !x || !x->is_valid( ) )
				continue;

			if ( cfg_champion_names.find( x->get_champion( ) ) != cfg_champion_names.end( ) )
				continue;

			auto str_default_name = x->get_model( );
			auto it_default = m_champion_map.find( x->get_champion( ) );
			if ( it_default != m_champion_map.end( ) )
				str_default_name = it_default->second;

			cfg_champion_names [ x->get_champion( ) ] = champion_spellnames->add_text_input( "name_" + x->get_model( ), x->get_model( ), str_default_name );
		}
	}

	auto spell_ready_ping_type_click = [ ]( TreeEntry* s ) {
		g_side_hud->spellready_message_text->is_hidden( ) = g_side_hud->button_champion_1->is_hidden( ) = g_side_hud->button_spell_1->is_hidden( ) = s->get_int( ) != 1;
	};
	spell_ready_ping_type_click( spell_ready_ping_type );
	spell_ready_ping_type->add_property_change_callback( spell_ready_ping_type_click );

	auto spell_cooldown_type_click = [ ]( TreeEntry* s ) {
		g_side_hud->spellcooldown_message_text->is_hidden( ) = 
			g_side_hud->button_champion_2->is_hidden( ) = 
			g_side_hud->button_spell_2->is_hidden( ) = 
			g_side_hud->button_seconds_2->is_hidden( ) = 
			g_side_hud->button_m_2->is_hidden( ) = 
			g_side_hud->button_s_2->is_hidden( ) =
			s->get_int( ) == 0;
	};
	spell_cooldown_type_click( spell_cooldown_type );
	spell_cooldown_type->add_property_change_callback( spell_cooldown_type_click );

	side_tab->add_separator( "elements_sep", " - -  Elements  - - " );
	show_elements = side_tab->add_prority_list( "show_elements", "Elements", {
		{ 'r', "R", true, nullptr },
		{ 's', "Summoner spells", true, nullptr },
		{ 'l', "Level", false, nullptr },
		{ 'h', "Health", true, nullptr },
		{ 'm', "Mana", true, nullptr }
		}, false );

#if !defined( __TEST )
	pos_x->is_hidden( ) = pos_y->is_hidden( ) = true;
#endif
}

auto c_side_hud::get_champion_name( game_object_script x ) -> std::string
{
	auto it = cfg_champion_names.find( x->get_champion( ) );

	return it != cfg_champion_names.end( ) ? it->second->get_string( ) : x->get_model( );
}

auto c_side_hud::on_draw( ) -> void
{
	if ( !myhero || !enabled || !enabled->get_bool( ) )
		return;

	if ( keyboard_state->is_pressed( keyboard_game::tab ) )
		return;

	static std::map<uint32_t, s_champion_data> m_last_visible = { };
	auto v_start = vector( static_cast<float>( pos_x->get_int( ) ), static_cast<float>( pos_y->get_int( ) ) );
	auto v_size = vector( 50.f * hud_size->get_int( ) / 100.f, 50.f * hud_size->get_int( ) / 100.f );
	auto v_size_padding = 7.f * hud_size->get_int( ) / 100.f;

	auto heroes = entitylist->get_enemy_heroes( );
	heroes.erase( std::remove_if( heroes.begin( ), heroes.end( ), [ ]( game_object_script x ) {
		return !x || !x->is_valid( );
	} ), heroes.end( ) );

	if ( heroes.empty( ) )
		return;

	auto total_sz = vector::zero;
	auto v_max = vector::zero, v_min = vector::zero;

	switch ( sidehud_side->get_int( ) )
	{
		default:
			total_sz.x = v_size.x * heroes.size( ) + v_size_padding * std::max( static_cast< size_t >( 0 ), static_cast< size_t >( heroes.size( ) - 1 ) );
			total_sz.y = v_size.y;

			v_min = vector( 0.f, 0.f );
			v_max = vector( renderer->screen_width( ) - total_sz.x, renderer->screen_height( ) - v_size.y );
			break;

		case 1:
			total_sz.x = v_size.x * heroes.size( ) + v_size_padding * std::max( static_cast< size_t >( 0 ), static_cast< size_t >( heroes.size( ) - 1 ) ); 
			total_sz.y = v_size.y;

			v_min = vector( total_sz.x - v_size.x, 0.f );
			v_max = vector( renderer->screen_width( ) - v_size.x, renderer->screen_height( ) - v_size.y );
			break;

		case 2:
			total_sz.x = v_size.x;
			total_sz.y = ( v_size.y * heroes.size( ) + v_size_padding * std::max( static_cast< size_t >( 0 ), static_cast< size_t >( heroes.size( ) - 1 ) ) );

			v_min = vector( 0.f, 0.f );
			v_max = vector( renderer->screen_width( ) - total_sz.x, renderer->screen_height( ) - total_sz.y );
			break;
	}

	auto v_start_hover = sidehud_side->get_int( ) != 1 ? v_start : ( v_start - vector( total_sz.x - v_size.x, 0.f ) );
	auto v_end_hover = sidehud_side->get_int( ) != 1 ? ( v_start + total_sz ) : ( v_start + vector( v_size.x, total_sz.y ) );

	static auto b_moving = false;
	static auto v_diff = vector::zero;

	auto cursor = game_input->get_cursor_pos( );
	auto b_hover = cursor.x >= v_start_hover.x && cursor.x <= v_end_hover.x && 
		cursor.y >= v_start_hover.y && cursor.y <= v_end_hover.y;

#pragma region move_logic
	if ( keyboard_state->is_pressed( keyboard_game::mouse1 ) )
	{
		if ( b_hover || b_moving )
		{
			if ( !v_diff.is_valid( ) )
				v_diff = vector( cursor.x - v_start.x, cursor.y - v_start.y );

			auto new_safe_pos = vector(
				std::clamp( cursor.x - v_diff.x, v_min.x, v_max.x ),
				std::clamp( cursor.y - v_diff.y, v_min.y, v_max.y )
			);

			v_start.x = new_safe_pos.x;
			v_start.y = new_safe_pos.y;

			b_moving = true;
		}
	}
	else
	{
		b_moving = false;
		v_diff = vector::zero;
	}
#pragma endregion

	pos_x->set_int( static_cast< int32_t >( std::clamp( v_start.x, v_min.x, v_max.x ) ) );
	pos_y->set_int( static_cast< int32_t >( std::clamp( v_start.y, v_min.y, v_max.y ) ) );

	static auto last_key_state = ping_key->get_bool( );
	auto is_clicked = last_key_state != ping_key->get_bool( ) && ping_key->get_bool( ) && ping_key->get_hotkey_type( ) == TreeHotkeyMode::Hold;
	last_key_state = ping_key->get_bool( );
	
	for ( auto&& x : heroes )
	{
		if ( !x || !x->is_valid( ) )
			continue;

		auto it_vis = m_last_visible.find( x->get_network_id( ) );
		if ( it_vis == m_last_visible.end( ) )
		{
			m_last_visible [ x->get_network_id( ) ] = s_champion_data
			{
				gametime->get_time( ),
				x->is_visible( ) ? x->get_health_percent( ) : 100.f
			};

			continue;
		}

		if ( x->is_visible( ) )
		{
			it_vis->second.fl_last_health = x->get_health_percent( );
			it_vis->second.fl_last_mana = x->get_mana( ) / x->get_max_mana( );

			if ( it_vis->second.fl_animvalue_vis < 0.01f )
				it_vis->second.fl_last_visible = gametime->get_time( );
		}

		const auto is_dead = x->is_dead( );
		if ( is_dead )
			it_vis->second.fl_last_visible = gametime->get_time( );

		static auto easing_fn_in = getEasingFunction( easing_functions::EaseInSine );
		static auto easing_fn_out = getEasingFunction( easing_functions::EaseOutSine );

#pragma region animations
		if ( is_dead )
		{
			if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_dead >= 0.02f )
			{
				it_vis->second.fl_animvalue_dead = std::clamp( easing_fn_in( it_vis->second.fl_animvalue_dead ), 0.001f, 0.999f );
				it_vis->second.t_last_animvalue_dead = gametime->get_time( );
			}
		}
		else
		{
			if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_dead >= 0.02f )
			{
				it_vis->second.fl_animvalue_dead = std::clamp( easing_fn_out( it_vis->second.fl_animvalue_dead ), 0.001f, 0.999f );
				it_vis->second.t_last_animvalue_dead = gametime->get_time( );
			}
		}

		if ( !x->is_visible( ) || is_dead )
		{
			if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_vis >= 0.02f )
			{
				it_vis->second.fl_animvalue_vis = std::clamp( easing_fn_in( it_vis->second.fl_animvalue_vis ), 0.001f, 0.999f );
				it_vis->second.t_last_animvalue_vis = gametime->get_time( );
			}
		}
		else
		{
			if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_vis >= 0.02f )
			{
				it_vis->second.fl_animvalue_vis = std::clamp( easing_fn_out( it_vis->second.fl_animvalue_vis ), 0.001f, 0.999f );
				it_vis->second.t_last_animvalue_vis = gametime->get_time( );
			}
		}
#pragma endregion

		auto fl_color = 1.f;
		auto size_r = v_size / 2.4f;

		if ( it_vis->second.fl_animvalue_dead > 0.01f )
			fl_color -= 0.75f * it_vis->second.fl_animvalue_dead;
		else if ( it_vis->second.fl_animvalue_vis > 0.01f )
			fl_color -= 0.75f * it_vis->second.fl_animvalue_vis;

		draw_manager->add_image( x->get_square_icon_portrait( ), v_start, v_size, 90.f, { 0.f, 0.f }, { 1.f, 1.f },
			{ fl_color, fl_color, fl_color, 1.f } );
		draw_manager->add_circle_on_screen( v_start + v_size / 2.f, v_size.x / 2.f, MAKE_COLOR( 53, 59, 72, 255 ), 3.f * hud_size->get_int( ) / 100.f );

		auto p_show_r = show_elements->get_prority( 'r' );
		auto p_show_sums = show_elements->get_prority( 's' );
		auto angle_deg_r = p_show_r.first < p_show_sums.first ? 45.f : 90.f + 45.f;

		vector point_r =
		{
			v_start.x + v_size.x / 2.f + v_size.x / 2.f * std::cos( degrees_to_radians( angle_deg_r ) ),
			v_start.y + v_size.y / 2.f + v_size.y / 2.f * std::sin( degrees_to_radians( angle_deg_r ) )
		};

		auto get_point_sum = [ & ]( int i ) -> vector
		{
			auto angle_deg_sum = p_show_sums.first > p_show_r.first ? -90.f - 45.f * ( i == 0 ? 1 : 3 ) : ( -270.f - 45.f * ( i == 0 ? 1 : 3 ) );
			return
			{
				v_start.x + v_size.x / 2.f + v_size.x / 2.f * std::cos( degrees_to_radians( angle_deg_sum ) ),
				v_start.y + v_size.y / 2.f + v_size.y / 2.f * std::sin( degrees_to_radians( angle_deg_sum ) )
			};
		};

		bool b_hover_spell [ 3 ] =
		{
			geometry::circle( get_point_sum( 0 ), size_r.x / 2.f ).to_polygon( ).is_inside( cursor ),
			geometry::circle( get_point_sum( 1 ), size_r.x / 2.f ).to_polygon( ).is_inside( cursor ),
			geometry::circle( point_r, size_r.x / 2.f ).to_polygon( ).is_inside( cursor )
		};

		auto b_hover_portrait = !b_hover_spell[ 2 ] && !b_hover_spell[ 1 ] && !b_hover_spell[ 0 ] && geometry::circle( v_start + v_size / 2.f, v_size.x / 2.f ).to_polygon( ).is_inside( cursor );
		if ( b_hover_portrait )
		{
			draw_manager->add_circle_on_screen( v_start + v_size / 2.f, v_size.x / 2.f, utility::COLOR_LGREEN, 1.f * hud_size->get_int( ) / 100.f );
			if ( is_clicked && gametime->get_time( ) - fl_last_ping >= 0.5f )
				myhero->send_hero_ping( x );
		}

#pragma region health & mana
		auto p_health = show_elements->get_prority( 'h' ), 
			p_mana = show_elements->get_prority( 'm' );

		auto sz_healthbar = v_size / 7.f;
		auto sz_manabar = sz_healthbar / 2.f;

		auto fl_anim_health = is_dead ? 1.f - it_vis->second.fl_animvalue_dead : 1.f - ( it_vis->second.fl_animvalue_vis * 0.9f );
		if ( p_health.second )
		{
			constexpr const auto min_health_clamp = 0.1f;
			auto clr_health = A_::set_alpha( A_::get_color( it_vis->second.fl_last_health ), static_cast< unsigned long > ( 255.f * fl_anim_health ) );
			if ( sidehud_side->get_int( ) != 2 )
			{
				draw_manager->add_filled_rect( v_start + vector( 0.f, v_size.y + v_size_padding ), v_start + vector( v_size.x, v_size.y + v_size_padding + sz_healthbar.y ),
					MAKE_COLOR( 53, 59, 72, static_cast< int >( 180.f * fl_anim_health ) ), 10.f );

				draw_manager->add_filled_rect( v_start + vector( 0.f, v_size.y + v_size_padding ),
					v_start + vector( ( v_size.x ) * std::clamp( it_vis->second.fl_last_health / 100.f, min_health_clamp, 1.f ), v_size.y + v_size_padding + v_size.y / 7.f ),
					clr_health, 10.f );
			}
			else
			{
				draw_manager->add_filled_rect( v_start + vector( v_size.x + v_size_padding, 0.f ), v_start + vector( v_size.y + v_size_padding + sz_healthbar.y, v_size.y ),
					MAKE_COLOR( 53, 59, 72, static_cast< int >( 180.f * fl_anim_health ) ), 10.f );

				draw_manager->add_filled_rect( v_start + vector( v_size.x + v_size_padding, 0.f ), 
					v_start + vector( v_size.x + v_size_padding + sz_healthbar.x, v_size.y * std::clamp( it_vis->second.fl_last_health / 100.f, min_health_clamp, 1.f ) ),
					clr_health, 10.f );
			}
		}

		auto fl_anim_mana = is_dead ? 1.f - it_vis->second.fl_animvalue_dead : 1.f - it_vis->second.fl_animvalue_vis;
		if ( p_mana.second && x->get_max_mana( ) > 0.f && fl_anim_mana > 0.01f )
		{
			constexpr const auto min_mana_clamp = 0.1f;
			if ( sidehud_side->get_int( ) != 2 )
			{
				draw_manager->add_filled_rect( v_start + vector( 0.f, v_size.y + v_size_padding + sz_healthbar.y + 4.f ), v_start + vector( v_size.x, v_size.y + v_size_padding + sz_healthbar.y + sz_manabar.y + 4.f ),
					MAKE_COLOR( 53, 59, 72, static_cast< int >( 180.f * fl_anim_mana ) ), 10.f );

				draw_manager->add_filled_rect( v_start + vector( 0.f, v_size.y + v_size_padding + sz_healthbar.y + 4.f ), v_start + vector( v_size.x * std::clamp( it_vis->second.fl_last_mana, min_mana_clamp, 1.f ), v_size.y + v_size_padding + sz_healthbar.y + sz_manabar.y + 4.f ),
					MAKE_COLOR( 52, 152, 219, static_cast< int >( 255.f * fl_anim_mana ) ), 10.f );
			}
			else
			{
				draw_manager->add_filled_rect( v_start + vector( v_size.x + v_size_padding + sz_healthbar.x + 6.f, 0.f ), v_start + vector( v_size.x + v_size_padding + sz_healthbar.x + sz_manabar.x + 6.f, v_size.y ),
					MAKE_COLOR( 53, 59, 72, static_cast< int >( 180.f * fl_anim_mana ) ), 10.f );

				draw_manager->add_filled_rect( v_start + vector( v_size.x + v_size_padding + sz_healthbar.x + 6.f, 0.f ), 
					v_start + vector( v_size.x + v_size_padding + sz_healthbar.x + sz_manabar.x + 6.f, v_size.y * std::clamp( it_vis->second.fl_last_mana, min_mana_clamp, 1.f ) ),
					MAKE_COLOR( 53, 152, 219, static_cast< int >( 255.f * fl_anim_mana ) ), 10.f );
			}
		}
#pragma endregion

		if ( is_dead )
		{
			auto clr_dead_cross = MAKE_COLOR( 231, 76, 60, static_cast< int >( 120.f * it_vis->second.fl_animvalue_dead ) );
			auto clr_respawn_text = MAKE_COLOR( 236, 240, 241, static_cast< int >( 255.f * it_vis->second.fl_animvalue_dead ) );

			draw_manager->add_line_on_screen( v_start + vector( v_size_padding, v_size_padding ) * 2.f, v_start + v_size - vector( v_size_padding, v_size_padding ) * 2.f, clr_dead_cross,
				4.f * hud_size->get_int( ) / 100.f );
			draw_manager->add_line_on_screen( v_start + vector( v_size_padding * 2.f, v_size.y - v_size_padding * 2.f ), v_start + vector( v_size.x - v_size_padding * 2.f, v_size_padding * 2.f ), clr_dead_cross,
				4.f * hud_size->get_int( ) / 100.f );

			auto t_respawn = std::clamp( x->get_respawn_time_remaining( ), 0.f, 99.f );
			auto sz_respawn = draw_manager->calc_text_size( static_cast<int>( v_size.x / 2.f ), "%.0f", t_respawn );
			draw_manager->add_text_on_screen( v_start + v_size / 2.f - sz_respawn / 2.f, clr_respawn_text, static_cast< int >( v_size.x / 2.f ), "%.0f", t_respawn );
		}

		if ( ( !x->is_visible( ) || it_vis->second.fl_animvalue_vis > 0.01f ) && !is_dead )
		{
			auto clr_mia = MAKE_COLOR( 241, 196, 15, static_cast< int >( 255.f * it_vis->second.fl_animvalue_vis ) );

			auto text_invis = std::min( 99.f, gametime->get_time( ) - it_vis->second.fl_last_visible );

			auto sz_time = draw_manager->calc_text_size( static_cast< int >( v_size.x / 2.f ), "%.0f", text_invis );
			draw_manager->add_text_on_screen( v_start + v_size / 2.f - sz_time / 2.f, clr_mia, static_cast< int >( v_size.x / 2.f ), "%.0f", text_invis );
		}

#pragma region sums, r & lvl
		auto get_sum_print_name = [ & ]( std::string name, uint32_t hash ) -> std::string
		{
			auto it = name.find( "SummonerSmite" ) != std::string::npos ?
				spellname_other.find( spell_hash( "SummonerSmite" ) ) :
				spellname_other.find( hash );

			return it != spellname_other.end( ) ? it->second->get_string( ) : "";
		};

		//sums
		if ( p_show_sums.second )
		{
			for ( auto i = 0; i <= 1; i++ )
			{
				auto spell = x->get_spell( ( spellslot )( ( int )spellslot::summoner1 + i ) );
				if ( !spell || !spell->is_learned( ) )
					continue;

				auto opt_cd = g_cooldown->sum_cd( x, spell->get_name_hash( ) );
				auto cd = opt_cd.has_value( ) ? opt_cd.value( ) : spell->cooldown( );
				auto is_on_cooldown = cd > 0.f;

				if ( !is_on_cooldown )
				{
					if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_spell [ i ] >= 0.02f )
					{
						it_vis->second.fl_animvalue_spell [ i ] = std::clamp( easing_fn_in( it_vis->second.fl_animvalue_spell [ i ] ), 0.001f, 0.999f );
						it_vis->second.t_last_animvalue_spell [ i ] = gametime->get_time( );
					}
				}
				else
				{
					if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_spell [ i ] >= 0.02f )
					{
						it_vis->second.fl_animvalue_spell [ i ] = std::clamp( easing_fn_out( it_vis->second.fl_animvalue_spell [ i ] ), 0.001f, 0.999f );
						it_vis->second.t_last_animvalue_spell [ i ] = gametime->get_time( );
					}
				}


				auto img = spell->get_icon_texture_by_index( 0 );
				if ( img )
				{
					auto fl_color = std::clamp( it_vis->second.fl_animvalue_spell [ i ], 0.15f, 1.f );
					auto fl_alpha_txt = 1.f - it_vis->second.fl_animvalue_spell [ i ];

					auto point_sum = get_point_sum( i );

					draw_manager->add_image( img, point_sum - size_r / 2.f, size_r, 90.f, { 0.f, 0.f }, { 1.f, 1.f }, { fl_color, fl_color, fl_color, 1.f } );
					draw_manager->add_circle_on_screen( point_sum, size_r.x / 2.f, b_hover_spell [ i ] ? MAKE_COLOR( 46, 204, 113, 255 ) : MAKE_COLOR( 53, 59, 72, 255 ), b_hover ? 2.f : 1.f );

					if ( b_hover_spell [ i ] && is_clicked && gametime->get_time( ) - fl_last_ping >= 0.5f )
					{
						if ( !is_on_cooldown )
						{
							if ( spell_ready_ping_type->get_int( ) == 0 )
							{
								myhero->send_spell_ping( x, static_cast< int32_t >( static_cast< int >( spellslot::summoner1 ) + i ) );
							}
							else
							{
								auto text = spellready_message_text->get_string( );
								auto it_championname = this->get_champion_name( x );
								auto spell_name = get_sum_print_name( spell->get_name( ), spell->get_name_hash( ) );

								if ( !text.empty( ) && !it_championname.empty( ) && !spell_name.empty( ) &&
									A_::str_replace( text, "{champion}", it_championname ) &&
									A_::str_replace( text, "{spell}", spell_name ) )
								{
									myhero->send_chat( text.c_str( ) );
								}
							}
							fl_last_ping = gametime->get_time( );
						}
						else
						{
							if ( spell_cooldown_type->get_int( ) == 0 )
							{
								myhero->send_spell_ping( x, static_cast< int32_t >( static_cast< int >( spellslot::summoner1 ) + i ) );
							}
							else
							{
								auto text = spellcooldown_message_text->get_string( );
								auto it_championname = this->get_champion_name( x );
								auto spell_name = get_sum_print_name( spell->get_name( ), spell->get_name_hash( ) );

								std::stringstream stream;
								stream << std::fixed << std::setprecision( 0 ) << cd;
								std::string str_spell_cd = stream.str( );

								auto minutes = static_cast< int >( std::floorf( cd / 60.f ) );
								auto seconds = static_cast< int >( cd ) % 60;

								if ( cd >= 1.f && !str_spell_cd.empty( ) && !spell_name.empty( ) && !text.empty( ) && !it_championname.empty( ) &&
									A_::str_replace( text, "{champion}", it_championname ) &&
									A_::str_replace( text, "{spell}", spell_name ) &&
									( A_::str_replace( text, "{seconds}", str_spell_cd ) || 
										( A_::str_replace( text, "{s}", std::to_string( seconds ) ) && A_::str_replace( text, "{m}", std::to_string( minutes ) ) ) )
									)
								{
									myhero->send_chat( text.c_str( ) );
								}
								fl_last_ping = gametime->get_time( );
							}
						}
					}

					if ( fl_alpha_txt > 0.01f )
					{
						auto font_sz_cd = static_cast< int >( size_r.x / 2.f );
						auto sz_cd = draw_manager->calc_text_size( font_sz_cd, cd <= 10.f ? "%.1f" : "%.0f", cd );
						draw_manager->add_text_on_screen( point_sum - sz_cd / 2.f, MAKE_COLOR( 236, 240, 241, static_cast< int >( 255.f * fl_alpha_txt ) ), font_sz_cd, cd <= 10.f ? "%.1f" : "%.0f", cd );
					}
				}
			}
		}

		//r
		if ( p_show_r.second )
		{
			auto spell = x->get_spell( spellslot::r );
			if ( spell && spell->is_learned( ) )
			{
				auto opt_cd = g_cooldown->spell_cd( x, spellslot::r );
				auto cd = opt_cd.has_value( ) ? opt_cd.value( ) : spell->cooldown( );
				auto is_on_cooldown = cd > 0.f;
				if ( !is_on_cooldown )
				{
					if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_spell [ 2 ] >= 0.02f )
					{
						it_vis->second.fl_animvalue_spell [ 2 ] = std::clamp( easing_fn_in( it_vis->second.fl_animvalue_spell [ 2 ] ), 0.001f, 0.999f );
						it_vis->second.t_last_animvalue_spell [ 2 ] = gametime->get_time( );
					}
				}
				else
				{
					if ( gametime->get_time( ) - it_vis->second.t_last_animvalue_spell [ 2 ] >= 0.02f )
					{
						it_vis->second.fl_animvalue_spell [ 2 ] = std::clamp( easing_fn_out( it_vis->second.fl_animvalue_spell [ 2 ] ), 0.001f, 0.999f );
						it_vis->second.t_last_animvalue_spell [ 2 ] = gametime->get_time( );
					}
				}
				{
					auto img = spell->get_icon_texture_by_index( 0 );
					if ( img )
					{
						auto fl_color = std::clamp( it_vis->second.fl_animvalue_spell [ 2 ], 0.15f, 1.f );
						auto fl_alpha_txt = 1.f - it_vis->second.fl_animvalue_spell [ 2 ];

						draw_manager->add_image( img, point_r - size_r / 2.f, size_r, 90.f, { 0.f, 0.f }, { 1.f, 1.f }, { fl_color, fl_color, fl_color, 1.f } );
						draw_manager->add_circle_on_screen( point_r, size_r.x / 2.f, b_hover_spell [ 2 ] ? MAKE_COLOR( 46, 204, 113, 255 ) : MAKE_COLOR( 53, 59, 72, 255 ), b_hover ? 2.f : 1.f );

						if ( b_hover_spell[ 2 ] && is_clicked && gametime->get_time( ) - fl_last_ping >= 0.5f )
						{
							if ( !is_on_cooldown )
							{
								if ( spell_ready_ping_type->get_int( ) == 0 )
								{
									myhero->send_spell_ping( x, static_cast< int32_t >( spellslot::r ) );
								}
								else
								{
									auto text = spellready_message_text->get_string( );
									auto it_championname = this->get_champion_name( x );
									auto text_r = spellname_r->get_string( );
									if ( !text.empty( ) && !text_r.empty( ) && !it_championname.empty( ) &&
										A_::str_replace( text, "{champion}", it_championname ) &&
										A_::str_replace( text, "{spell}", text_r ) )
									{
										myhero->send_chat( text.c_str( ) );
									}
								}
								fl_last_ping = gametime->get_time( );
							}
							else
							{
								if ( spell_cooldown_type->get_int( ) == 0 )
								{
									myhero->send_spell_ping( x, static_cast< int32_t >( spellslot::r ) );
								}
								else
								{
									auto text = spellcooldown_message_text->get_string( );
									auto it_championname = this->get_champion_name( x );

									std::stringstream stream;
									stream << std::fixed << std::setprecision( 0 ) << cd;
									std::string str_spell_cd = stream.str( );

									if ( cd >= 1.f && !str_spell_cd.empty( ) && !text.empty( ) && !it_championname.empty() &&
										A_::str_replace( text, "{champion}", it_championname ) &&
										A_::str_replace( text, "{spell}", "R" ) &&
										A_::str_replace( text, "{seconds}", str_spell_cd ) )
									{
										myhero->send_chat( text.c_str( ) );
									}
									fl_last_ping = gametime->get_time( );
								}
							}
						}

						if ( fl_alpha_txt > 0.01f )
						{
							auto font_sz_cd = static_cast< int >( size_r.x / 2.f );
							auto sz_cd = draw_manager->calc_text_size( font_sz_cd, cd <= 10.f ? "%.1f" : "%.0f", cd );
							draw_manager->add_text_on_screen( point_r - sz_cd / 2.f, MAKE_COLOR( 236, 240, 241, static_cast< int >( 255.f * fl_alpha_txt ) ), font_sz_cd, cd <= 10.f ? "%.1f" : "%.0f", cd );
						}
					}
				}
			}
		}

		//lvl
		if ( show_elements->get_prority( 'l' ).second )
		{
			auto angle_deg = p_show_r.first < p_show_sums.first ? 45.f - 90.f : 45.f - 180.f;
			vector point_r =
			{
				v_start.x + v_size.x / 2.f + v_size.x / 2.f * std::cos( degrees_to_radians( angle_deg ) ),
				v_start.y + v_size.y / 2.f + v_size.y / 2.f * std::sin( degrees_to_radians( angle_deg ) )
			};

			draw_manager->add_filled_circle_on_screen( point_r, size_r.x / 3.f, MAKE_COLOR( 53, 59, 72, 255 ) );
			{
				auto font_sz_lvl = static_cast< int >( size_r.x / 2.f );
				auto sz_lvl = draw_manager->calc_text_size( font_sz_lvl, "%d", x->get_level( ) );
				draw_manager->add_text_on_screen( point_r - sz_lvl / 2.f, MAKE_COLOR( 236, 240, 241, 255 ), font_sz_lvl, "%d", x->get_level( ) );
			}

			//auto b_hover = geometry::circle( point_r, size_r.x / 2.f ).to_polygon( ).is_inside( cursor );
			//if ( b_hover && is_clicked && gametime->get_time( ) - fl_last_ping >= 0.5f )
			//{
			//can't ping level?
			//}
		}
#pragma endregion

		//RLB
		switch ( sidehud_side->get_int( ) )
		{
			default: //R
				v_start.x += v_size.x + v_size_padding;
				break;

			case 1: //L
				v_start.x -= ( v_size.x + v_size_padding );
				break;

			case 2: //B
			{
				v_start.y += ( v_size.x + v_size_padding );
				break;
			}
		}	
	}
}

namespace sidehud_callbacks
{
	auto on_draw( ) -> void
	{
		g_side_hud->on_draw( );
	}
}

auto c_side_hud::load( ) -> void
{
	create_menu( );

	event_handler<events::on_draw>::add_callback( sidehud_callbacks::on_draw );
}

auto c_side_hud::unload( ) -> void
{
	event_handler<events::on_draw>::remove_handler( sidehud_callbacks::on_draw );
}