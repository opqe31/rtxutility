#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"
#include <optional>

class c_map_data 
{
public:
	enum e_zone_type 
	{
		base,
		botlane,
		jungle,

		midlane,
		river,
		toplane
	};

	enum e_team_type
	{
		red,
		blue,

		neutral
	};

	struct s_map_data_entry
	{
		geometry::polygon polygon;

		const char* _internal_name;
		e_zone_type zone_type;
		e_team_type team_type;
		bool is_enemy = false;

		vector center = vector::zero;

		s_map_data_entry( const char* strname, e_team_type team, e_zone_type type = e_zone_type::base )
		{
			_internal_name = strname;
			zone_type = type;

			team_type = team;

			static auto my_team_red = myhero->get_team( ) == game_object_team::chaos;
			is_enemy = ( my_team_red && team != e_team_type::red ) || ( !my_team_red && team == e_team_type::red );
		}

		auto get_display_name( bool to_uppercase = true ) -> std::string
		{
			auto name = std::string( _internal_name );
			if ( team_type == e_team_type::neutral )
				name [ 0 ] = std::toupper( name [ 0 ] );
			else
				name = ( is_enemy ? "Enemy " : "Ally " ) + name;

			if ( to_uppercase )
			{
				std::transform( name.begin( ), name.end( ), name.begin( ),
					[ ]( unsigned char c ) { return std::toupper( c ); } );
			}

			return name;
		}

		auto init( ) -> void
		{
			if ( !polygon.points.empty( ) )
			{
				if ( polygon.points.size( ) == 1 )
					center = polygon.points.front( );
				else
				{
					auto mec = mec::get_mec( polygon.points );
					center = mec.center;
				}
			}
		}
	};

	std::vector< s_map_data_entry > m_map_data = { };

	void load( );
	std::optional< std::pair<std::string, float> > get_position_and_distance( vector pos );
};

extern c_map_data* g_map_data;