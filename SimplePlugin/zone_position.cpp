#include "main_hud.hpp"
#include "map_data.hpp"
#include "utility.h"
#include "fow_tracker.h"

struct s_zone_draw {
	std::vector < std::pair< std::string, unsigned long > > text;
	std::vector< vector > text_size;

	s_zone_draw( std::vector < std::pair< std::string, unsigned long > > vtext, int font_size  )
	{
		text = vtext;
		for ( auto&& p : text )
			text_size.push_back( draw_manager->calc_text_size( font_size, p.first.c_str( ) ) );
	}
};

auto spell_tracker::track_zones( bool draw_zones, bool draw, TreeEntry* priority_list, int font_size, TreeEntry* x, TreeEntry* y, bool lock, bool hidevis ) -> void
{
	static auto b_allowed_map = missioninfo->get_map_id( ) == game_map_id::SummonersRift;
	if ( !b_allowed_map )
		return;

	auto&& map = g_map_data->m_map_data;

	if ( map.empty( ) )
		return;

	if ( draw_zones )
	{
		for ( auto&& entry : map )
		{
			auto&& poly = entry.polygon;
			auto mec = mec::get_mec( poly.points );

			auto sz_text = draw_manager->calc_text_size( 18, entry.get_display_name( ).c_str( ) );
			auto mec_screen = mec.center;

			renderer->world_to_screen( mec.center, mec_screen );
			draw_manager->add_text_on_screen( mec_screen - sz_text / 2.f, utility::COLOR_WHITE, 18, entry.get_display_name( ).c_str( ) );

			if ( poly.points.size( ) > 1 )
			{
				for ( auto i = 0; i < poly.points.size( ); i++ )
				{
					auto&& start = poly.points [ i ];
					auto&& end = poly.points [ poly.points.size( ) - 1 == i ? 0 : i + 1 ];

					draw_manager->add_line( start, end, MAKE_COLOR( 255, 255, 255, 255 ), 2.f );
				}
			}
		}
	}

	if ( !draw )
		return;

	std::vector< s_zone_draw > m_draw = { };
	for ( auto&& x : entitylist->get_enemy_heroes( ) )
	{
		if ( !x || !x->is_valid( ) )
			continue;

		if ( x->is_visible( ) && hidevis )
			continue;

		if ( !priority_list->get_prority( x->get_network_id( ) ).second )
			continue;

		if ( x->is_dead( ) )
		{
			m_draw.push_back( s_zone_draw( { 
				{ x->get_model( ) + ": ", utility::COLOR_WHITE }, 
				{ "*DEAD*", utility::COLOR_RED } 
				}, font_size ) );
		}
		else
		{
			auto pos = fow_tracker->get_data( x->get_network_id( ) );
			if ( !pos.position.is_valid( ) )
				continue;

			auto opt_pair = g_map_data->get_position_and_distance( pos.position );
			if ( !opt_pair.has_value( ) )
				continue;

			auto d = pos.position.distance( myhero->get_position( ) );

			auto t_elapsed_fl = std::clamp( gametime->get_time( ) - pos.t_last_update, 0.f, 99.f );
			auto color_distance = utility::COLOR_LGREEN;
			if ( d <= 1000.f )
				color_distance = utility::COLOR_RED;
			else if ( d <= 4000.f )
				color_distance = utility::COLOR_ORANGE;

			std::string strdist;
			auto dist = static_cast< int >( std::ceilf( d / 100.f ) * 100.f );

			//strdist += std::to_string( static_cast< int >( opt_pair.value( ).second ) );
			strdist += /*" | " + */std::to_string( static_cast< int >( dist ) );

			auto text_miat = x->is_visible( ) ? "visible" : std::to_string( static_cast< int >( t_elapsed_fl ) );
			m_draw.push_back( s_zone_draw( {
				{ x->get_model( ) + ": ", utility::COLOR_WHITE },
				{ opt_pair.value( ).first, x->is_visible( ) ? utility::COLOR_LGREEN : utility::COLOR_YELLOW },
				{ " (", utility::COLOR_WHITE },
				{ text_miat, utility::COLOR_LGREEN },
				{ x->is_visible( ) ? ") [" : "s ago) [", utility::COLOR_WHITE },
				{ strdist, color_distance },
				{ "m]", utility::COLOR_WHITE },
				}, font_size ) );
		}
	}

	if ( m_draw.empty( ) )
		return;

	auto max_x = 0.f;
	for ( auto&& e : m_draw )
	{
		auto total_x = 0.f;
		for ( auto&& sz : e.text_size )
			total_x += sz.x;

		max_x = std::max( max_x, total_x );
	}

	auto vtotalsize = vector( max_x, m_draw.size( ) * ( 5.f + m_draw [ 0 ].text_size [ 0 ].y ) );
	vector new_safe_pos = vector(
		std::clamp( ( float )x->get_int( ), 0.f, renderer->screen_width( ) - vtotalsize.x ),
		std::clamp( ( float )y->get_int( ), 0.f, renderer->screen_height( ) - vtotalsize.y ) );

	if ( !lock )
	{
		auto cursor = game_input->get_cursor_pos( );
		static auto vdiff = vector::zero;
		static auto b_moving = false;

		auto vstart = vector( (float)x->get_int( ), ( float )y->get_int( ) );
		auto vend = vstart + vtotalsize;

		auto b_hover = cursor.x >= vstart.x && cursor.x <= vend.x && 
			cursor.y >= vstart.y && cursor.y <= vend.y;

		if ( b_hover )
			draw_manager->add_rect( vstart, vend, MAKE_COLOR( 255, 255, 255, 255 ), 1.f );	

		if ( keyboard_state->is_pressed( keyboard_game::mouse1 ) )
		{
			if ( b_hover || b_moving )
			{
				if ( !vdiff.is_valid( ) )
					vdiff = vector( cursor.x - vstart.x, cursor.y - vstart.y );

				new_safe_pos = vector(
					std::min( std::max( 0.f, cursor.x - vdiff.x ), renderer->screen_width( ) - vtotalsize.x ),
					std::min( std::max( 0.f, cursor.y - vdiff.y ), renderer->screen_height( ) - vtotalsize.y )
				);

				x->set_int( static_cast< uint32_t >( new_safe_pos.x ) );
				y->set_int( static_cast< uint32_t >( new_safe_pos.y ) );

				b_moving = true;
			}
		}
		else
		{
			b_moving = false;
			vdiff = vector::zero;
		}
	}

	x->set_int( static_cast<int32_t>( new_safe_pos.x ) );
	y->set_int( static_cast< int32_t >( new_safe_pos.y ) );

	auto&& start_y = static_cast<float>( new_safe_pos.y );
	for ( auto&& e : m_draw )
	{
		auto start_x = static_cast<float>( new_safe_pos.y );
		for ( auto i = 0; i < e.text.size( ); i++ )
		{
			draw_manager->add_text_on_screen( vector( start_x, start_y ), e.text [ i ].second, font_size, e.text [ i ].first.c_str( ) );
			start_x += e.text_size [ i ].x;
		}
		start_y += e.text_size [ 0 ].y + 5.f;
	}
}