#include "map_data.hpp"
#include "A_.h"

c_map_data* g_map_data = new c_map_data( );

auto c_map_data::load( ) -> void
{
	if ( missioninfo->get_map_id( ) != SummonersRift )
		return;

#pragma region base
#pragma region blue
	{
		s_map_data_entry entry = s_map_data_entry( "base", e_team_type::blue );

		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 300, 300 ) );

		auto arc = A_::arc_poly( vector( 1700, 1700 ), vector( 4000, 4000 ), degrees_to_radians( 140.f ), 3200 );
		for ( auto p : arc.points )
			entry.polygon.add( p );

		entry.polygon.add( vector( 300, 5000 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom inhibitor", e_team_type::blue );
		entry.polygon = geometry::polygon(  );
		entry.polygon.add( vector( 3453, 1237 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom nexus turret", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 2178, 1808 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom turret", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 4282, 1254 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid inhibitor", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 4282, 1254 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid turret", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 3652, 3696 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "nexus", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 1550, 1660 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "spawn", e_team_type::blue );
		entry.polygon = geometry::circle( vector( 450, 500 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top inhibitor", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 1170, 3570 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top nexus turret", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 1748, 2270 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top turret", e_team_type::blue );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 1170, 4288 ) );

		m_map_data.push_back( entry );
	}
#pragma endregion

#pragma region red
	{
		s_map_data_entry entry = s_map_data_entry( "base", e_team_type::red );
		entry.polygon = geometry::polygon( ); 
		entry.polygon.add( vector( 14535, 10006 ) );
		entry.polygon.add( vector( 14573, 14724 ) );
		entry.polygon.add( vector( 9734, 14699 ) );
		entry.polygon.add( vector( 9836, 13173 ) );
		entry.polygon.add( vector( 9834, 13069 ) );
		entry.polygon.add( vector( 9867, 12806 ) );
		entry.polygon.add( vector( 9901, 12471 ) );
		entry.polygon.add( vector( 9974, 12188 ) );
		entry.polygon.add( vector( 10020, 11903 ) );
		entry.polygon.add( vector( 10135, 11653 ) );
		entry.polygon.add( vector( 10227, 11363 ) );
		entry.polygon.add( vector( 10396, 10946 ) );
		entry.polygon.add( vector( 10497, 10743 ) );
		entry.polygon.add( vector( 10673, 10579 ) );
		entry.polygon.add( vector( 11066, 10427 ) );
		entry.polygon.add( vector( 11132, 10406 ) );
		entry.polygon.add( vector( 11409, 10295 ) );
		entry.polygon.add( vector( 11690, 10189 ) );
		entry.polygon.add( vector( 11897, 10136 ) );
		entry.polygon.add( vector( 12187, 10060 ) );
		entry.polygon.add( vector( 12394, 10037 ) );
		entry.polygon.add( vector( 12656, 10011 ) );
		entry.polygon.add( vector( 12981, 9990 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom inhibitor", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 13605, 11316 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom nexus turret", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 13053, 12612 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom turret", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 13625, 10573 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid inhibitor", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 11598, 11668 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid turret", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 11135, 11208 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "nexus", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 13243, 13235 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "spawn", e_team_type::red );
		entry.polygon = geometry::circle( vector( 14300, 14400 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top inhibitor", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 11262, 13677 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top nexus turret", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 12612, 13084 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top turret", e_team_type::red );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 10481, 13650 ) );

		m_map_data.push_back( entry );
	}
#pragma endregion
#pragma endregion

#pragma region botlane
	{
		s_map_data_entry entry = s_map_data_entry( "inhibitor zone", e_team_type::blue, e_zone_type::botlane );
		entry.polygon = geometry::rectangle( vector( 3900, 1250 ), vector( 6100, 1250 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "inner zone", e_team_type::blue, e_zone_type::botlane );
		entry.polygon = geometry::rectangle( vector( 6100, 1250 ), vector( 9880, 1250 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "outter turret", e_team_type::blue, e_zone_type::botlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 10504, 1030 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "zone", e_team_type::blue, e_zone_type::botlane );
		entry.polygon = geometry::rectangle( vector( 3900, 1250 ), vector( 9880, 1250 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "botlane generic", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 11000, 573 ) );
		entry.polygon.add( vector( 11045, 575 ) );
		entry.polygon.add( vector( 11305, 635 ) );
		entry.polygon.add( vector( 11575, 655 ) );
		entry.polygon.add( vector( 11825, 765 ) );
		entry.polygon.add( vector( 12145, 835 ) );
		entry.polygon.add( vector( 12395, 950 ) );
		entry.polygon.add( vector( 12645, 1085 ) );
		entry.polygon.add( vector( 12785, 1245 ) );
		entry.polygon.add( vector( 13045, 1430 ) );
		entry.polygon.add( vector( 13240, 1635 ) );
		entry.polygon.add( vector( 13425, 1860 ) );
		entry.polygon.add( vector( 13700, 2126 ) );
		entry.polygon.add( vector( 13805, 2395 ) );
		entry.polygon.add( vector( 13935, 2630 ) );
		entry.polygon.add( vector( 14065, 2895 ) );
		entry.polygon.add( vector( 14140, 3060 ) );
		entry.polygon.add( vector( 14300, 3310 ) );
		entry.polygon.add( vector( 14360, 3540 ) );
		entry.polygon.add( vector( 14450, 3750 ) );
		entry.polygon.add( vector( 14550, 4115 ) );
		entry.polygon.add( vector( 14575, 4400 ) );
		entry.polygon.add( vector( 14575, 4710 ) );
		entry.polygon.add( vector( 14535, 4800 ) );
		entry.polygon.add( vector( 14450, 4798 ) );
		entry.polygon.add( vector( 14450, 10000 ) );
		entry.polygon.add( vector( 12900, 10000 ) );
		entry.polygon.add( vector( 12900, 4762 ) );
		entry.polygon.add( vector( 12825, 4760 ) );
		entry.polygon.add( vector( 11825, 4155 ) );
		entry.polygon.add( vector( 10370, 3115 ) );
		entry.polygon.add( vector( 10149, 2025 ) );
		entry.polygon.add( vector( 5000, 2025 ) );
		entry.polygon.add( vector( 5000, 475 ) );
		entry.polygon.add( vector( 11000, 475 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom neutral zone", e_team_type::neutral, e_zone_type::botlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 10180, 550 ) );
		entry.polygon.add( vector( 10500, 560 ) );
		entry.polygon.add( vector( 10770, 565 ) );
		entry.polygon.add( vector( 11045, 575 ) );
		entry.polygon.add( vector( 11305, 635 ) );
		entry.polygon.add( vector( 11575, 655 ) );
		entry.polygon.add( vector( 11825, 765 ) );
		entry.polygon.add( vector( 12145, 835 ) );
		entry.polygon.add( vector( 12395, 950 ) );
		entry.polygon.add( vector( 12645, 1085 ) );
		entry.polygon.add( vector( 12785, 1245 ) );
		entry.polygon.add( vector( 13045, 1430 ) );
		entry.polygon.add( vector( 13240, 1635 ) );
		entry.polygon.add( vector( 13425, 1860 ) );
		entry.polygon.add( vector( 13700, 2126 ) );
		entry.polygon.add( vector( 13805, 2395 ) );
		entry.polygon.add( vector( 13935, 2630 ) );
		entry.polygon.add( vector( 14065, 2895 ) );
		entry.polygon.add( vector( 14140, 3060 ) );
		entry.polygon.add( vector( 14300, 3310 ) );
		entry.polygon.add( vector( 14360, 3540 ) );
		entry.polygon.add( vector( 14450, 3750 ) );
		entry.polygon.add( vector( 14550, 4115 ) );
		entry.polygon.add( vector( 14575, 4400 ) );
		entry.polygon.add( vector( 14575, 4710 ) );
		entry.polygon.add( vector( 14535, 4800 ) );
		entry.polygon.add( vector( 12825, 4760 ) );
		entry.polygon.add( vector( 11825, 4155 ) );
		entry.polygon.add( vector( 10370, 3115 ) );
		entry.polygon.add( vector( 10130, 1930 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom inhibitor zone", e_team_type::red, e_zone_type::botlane );
		entry.polygon = geometry::rectangle( vector( 13675, 10900 ), vector( 13675, 8400 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom inner turret", e_team_type::red, e_zone_type::botlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 13328, 8226 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom outter turret", e_team_type::red, e_zone_type::botlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 13866, 4505 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom inner zone", e_team_type::red, e_zone_type::botlane );
		entry.polygon = geometry::rectangle( vector( 13675, 8400 ), vector( 13675, 4880 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}
#pragma endregion

#pragma region jungle
	{
		s_map_data_entry entry = s_map_data_entry( "blue buff", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 3910, 7920 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "blue buff", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 3910, 7920 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "blue buff", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 10830, 7000 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "blue buff", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 10830, 7000 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "gromp", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 2090, 8428 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "gromp", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 2090, 8428 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "krugs", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 8400, 2740 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "krugs", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 8400, 2740 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "raptors", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 6830, 5458 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "raptors", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 6830, 5458 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "red buff", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 7890, 4120 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "red buff", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 7890, 4120 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top jungle", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		
		entry.polygon.add( vector( 5841, 6866 ) );
		entry.polygon.add( vector( 5183, 7505 ) );
		entry.polygon.add( vector( 5058, 8122 ) );
		entry.polygon.add( vector( 3806, 9045 ) );
		entry.polygon.add( vector( 3388, 9568 ) );
		entry.polygon.add( vector( 2939, 10882 ) );
		entry.polygon.add( vector( 2058, 10857 ) );
		entry.polygon.add( vector( 1904, 10578 ) );
		entry.polygon.add( vector( 1890, 9901 ) );
		entry.polygon.add( vector( 1896, 8235 ) );
		entry.polygon.add( vector( 1873, 7324 ) );
		entry.polygon.add( vector( 1813, 6685 ) );
		entry.polygon.add( vector( 1839, 6087 ) );
		entry.polygon.add( vector( 1849, 4874 ) );
		entry.polygon.add( vector( 2458, 4870 ) );
		entry.polygon.add( vector( 2978, 4793 ) );
		entry.polygon.add( vector( 3476, 4492 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "wolves", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 3781, 6444 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "wolves", e_team_type::blue, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 3781, 6444 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom jungle", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		
		entry.polygon.add( vector( 12907, 4891 ) );
		entry.polygon.add( vector( 12919, 5602 ) );
		entry.polygon.add( vector( 12962, 5865 ) );
		entry.polygon.add( vector( 12946, 6566 ) );
		entry.polygon.add( vector( 12896, 7473 ) );
		entry.polygon.add( vector( 12960, 8012 ) );
		entry.polygon.add( vector( 12975, 8755 ) );
		entry.polygon.add( vector( 12958, 9991 ) );
		entry.polygon.add( vector( 12358, 10023 ) );
		entry.polygon.add( vector( 11790, 10166 ) );
		entry.polygon.add( vector( 11205, 10389 ) );
		entry.polygon.add( vector( 10657, 9691 ) );
		entry.polygon.add( vector( 10201, 9215 ) );
		entry.polygon.add( vector( 9532, 8585 ) );
		entry.polygon.add( vector( 9361, 8325 ) );
		entry.polygon.add( vector( 9563, 7590 ) );
		entry.polygon.add( vector( 9650, 6545 ) );
		entry.polygon.add( vector( 10276, 6136 ) );
		entry.polygon.add( vector( 10863, 5745 ) );
		entry.polygon.add( vector( 11461, 5157 ) );
		entry.polygon.add( vector( 11790, 4704 ) );
		entry.polygon.add( vector( 12163, 4245 ) );
		entry.polygon.add( vector( 12853, 4178 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "gromp", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 12704, 6444 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "gromp", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 12704, 6444 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "krugs", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 6400, 12150 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "krugs", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 6400, 12150 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "raptors", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 7800, 9460 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "raptors", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 7800, 9460 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "red buff", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 6980, 10760 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "red buff", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 6980, 10760 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top jungle", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		
		entry.polygon.add( vector( 8458, 9450 ) );
		entry.polygon.add( vector( 9287, 10244 ) );
		entry.polygon.add( vector( 10281, 11281 ) );
		entry.polygon.add( vector( 10045, 11867 ) );
		entry.polygon.add( vector( 9925, 12427 ) );
		entry.polygon.add( vector( 9845, 13030 ) );
		entry.polygon.add( vector( 9279, 13024 ) );
		entry.polygon.add( vector( 8343, 13050 ) );
		entry.polygon.add( vector( 7694, 13118 ) );
		entry.polygon.add( vector( 7097, 13141 ) );
		entry.polygon.add( vector( 6553, 13056 ) );
		entry.polygon.add( vector( 5987, 13016 ) );
		entry.polygon.add( vector( 5202, 12826 ) );
		entry.polygon.add( vector( 4354, 12895 ) );
		entry.polygon.add( vector( 4596, 11401 ) );
		entry.polygon.add( vector( 4873, 11346 ) );
		entry.polygon.add( vector( 5243, 11180 ) );
		entry.polygon.add( vector( 5567, 10971 ) );
		entry.polygon.add( vector( 5703, 10644 ) );
		entry.polygon.add( vector( 5700, 10238 ) );
		entry.polygon.add( vector( 5587, 9929 ) );
		entry.polygon.add( vector( 5978, 9608 ) );
		entry.polygon.add( vector( 7260, 9406 ) );
		entry.polygon.add( vector( 7944, 9001 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "wolves", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 11000, 8380 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "wolves", e_team_type::red, e_zone_type::jungle );
		entry.polygon = geometry::circle( vector( 11000, 8380 ), 800 ).to_polygon( );

		m_map_data.push_back( entry );
	}
#pragma endregion

#pragma region midlane
	{
		s_map_data_entry entry = s_map_data_entry( "mid inhibitor zone", e_team_type::blue, e_zone_type::midlane );
		entry.polygon = geometry::rectangle( vector( 3490, 3500 ), vector( 4560, 4700 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid inner turret", e_team_type::blue, e_zone_type::midlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 5048, 4813 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid inner zone", e_team_type::blue, e_zone_type::midlane );
		entry.polygon = geometry::rectangle( vector( 4560, 4700 ), vector( 5760, 5950 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid outter turret", e_team_type::blue, e_zone_type::midlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 5846, 6396 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "midlane", e_team_type::blue, e_zone_type::midlane );
		entry.polygon = geometry::rectangle( vector( 3490, 3500 ), vector( 5760, 5950 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "midlane neutral", e_team_type::neutral, e_zone_type::midlane );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 6805, 5795 ) );
		entry.polygon.add( vector( 7350, 5391 ) );
		entry.polygon.add( vector( 9451, 7471 ) );
		entry.polygon.add( vector( 9017, 7921 ) );
		entry.polygon.add( vector( 9491, 8338 ) );
		entry.polygon.add( vector( 8468, 9501 ) );
		entry.polygon.add( vector( 7968, 9061 ) );
		entry.polygon.add( vector( 7508, 9400 ) );
		entry.polygon.add( vector( 5348, 7408 ) );
		entry.polygon.add( vector( 5779, 6960 ) );
		entry.polygon.add( vector( 5274, 6554 ) );
		entry.polygon.add( vector( 6245, 5345 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "midlane", e_team_type::neutral, e_zone_type::midlane );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 6805, 5795 ) );
		entry.polygon.add( vector( 7350, 5391 ) );
		entry.polygon.add( vector( 9451, 7471 ) );
		entry.polygon.add( vector( 9017, 7921 ) );
		entry.polygon.add( vector( 9491, 8338 ) );
		entry.polygon.add( vector( 8468, 9501 ) );
		entry.polygon.add( vector( 7968, 9061 ) );
		entry.polygon.add( vector( 7508, 9400 ) );
		entry.polygon.add( vector( 5348, 7408 ) );
		entry.polygon.add( vector( 5779, 6960 ) );
		entry.polygon.add( vector( 5274, 6554 ) );
		entry.polygon.add( vector( 6245, 5345 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "midlane zone", e_team_type::neutral, e_zone_type::midlane );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 3510, 4470 ) );
		entry.polygon.add( vector( 5820, 6860 ) );
		entry.polygon.add( vector( 5400, 7350 ) );
		entry.polygon.add( vector( 7500, 9150 ) );
		entry.polygon.add( vector( 7960, 8850 ) );
		entry.polygon.add( vector( 10360, 11250 ) );
		entry.polygon.add( vector( 11200, 10440 ) );
		entry.polygon.add( vector( 9040, 7980 ) );
		entry.polygon.add( vector( 9650, 7595 ) );
		entry.polygon.add( vector( 7430, 5680 ) );
		entry.polygon.add( vector( 6790, 6130 ) );
		entry.polygon.add( vector( 4430, 3715 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid inhibitor zone", e_team_type::red, e_zone_type::midlane );
		entry.polygon = geometry::rectangle( vector( 11330, 11400 ), vector( 10310, 10210 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid inner turret", e_team_type::red, e_zone_type::midlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 9768, 10114 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid inner zone", e_team_type::red, e_zone_type::midlane );
		entry.polygon = geometry::rectangle( vector( 10310, 10210 ), vector( 8980, 8920 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "mid outter turret", e_team_type::red, e_zone_type::midlane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 8955, 8510 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "midlane", e_team_type::red, e_zone_type::midlane );
		entry.polygon = geometry::rectangle( vector( 11330, 11400 ), vector( 8980, 8920 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}
#pragma endregion

#pragma region river
	{
		s_map_data_entry entry = s_map_data_entry( "baron", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 4950, 10400 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "baron zone", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::polygon( );
		
		entry.polygon.add( vector( 6640, 8974 ) );
		entry.polygon.add( vector( 6623, 10325 ) );
		entry.polygon.add( vector( 6631, 11153 ) );
		entry.polygon.add( vector( 5977, 11575 ) );
		entry.polygon.add( vector( 5582, 11789 ) );
		entry.polygon.add( vector( 4737, 12101 ) );
		entry.polygon.add( vector( 3827, 11739 ) );
		entry.polygon.add( vector( 2830, 10905 ) );
		entry.polygon.add( vector( 3053, 10260 ) );
		entry.polygon.add( vector( 3336, 9367 ) );
		entry.polygon.add( vector( 3883, 9042 ) );
		entry.polygon.add( vector( 4388, 8722 ) );
		entry.polygon.add( vector( 5214, 8318 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "baron pit", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::circle( vector( 4950, 10400 ), 650 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "bottom river", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 12532, 3329 ) );
		entry.polygon.add( vector( 12301, 3580 ) );
		entry.polygon.add( vector( 12151, 4180 ) );
		entry.polygon.add( vector( 12071, 4357 ) );
		entry.polygon.add( vector( 11914, 4590 ) );
		entry.polygon.add( vector( 11770, 4779 ) );
		entry.polygon.add( vector( 11527, 5026 ) );
		entry.polygon.add( vector( 11473, 5362 ) );
		entry.polygon.add( vector( 10904, 5759 ) );
		entry.polygon.add( vector( 10418, 6133 ) );
		entry.polygon.add( vector( 10568, 6549 ) );
		entry.polygon.add( vector( 10550, 6881 ) );
		entry.polygon.add( vector( 10426, 7173 ) );
		entry.polygon.add( vector( 9750, 7518 ) );
		entry.polygon.add( vector( 9082, 8039 ) );
		entry.polygon.add( vector( 8443, 7298 ) );
		entry.polygon.add( vector( 7428, 6596 ) );
		entry.polygon.add( vector( 6775, 6001 ) );
		entry.polygon.add( vector( 7513, 5425 ) );
		entry.polygon.add( vector( 8808, 5229 ) );
		entry.polygon.add( vector( 9320, 4965 ) );
		entry.polygon.add( vector( 9207, 4679 ) );
		entry.polygon.add( vector( 9110, 4414 ) );
		entry.polygon.add( vector( 9138, 4129 ) );
		entry.polygon.add( vector( 9323, 3837 ) );
		entry.polygon.add( vector( 9616, 3581 ) );
		entry.polygon.add( vector( 9994, 3427 ) );
		entry.polygon.add( vector( 10381, 3059 ) );
		entry.polygon.add( vector( 10704, 3067 ) );
		entry.polygon.add( vector( 11032, 3116 ) );
		entry.polygon.add( vector( 11637, 2694 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "dragon zone", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 10743, 3073 ) );
		entry.polygon.add( vector( 12150, 3651 ) );
		entry.polygon.add( vector( 12037, 4128 ) );
		entry.polygon.add( vector( 11821, 4660 ) );
		entry.polygon.add( vector( 11483, 5052 ) );
		entry.polygon.add( vector( 11514, 5498 ) );
		entry.polygon.add( vector( 11094, 5850 ) );
		entry.polygon.add( vector( 10317, 6166 ) );
		entry.polygon.add( vector( 9616, 6451 ) );
		entry.polygon.add( vector( 8644, 5624 ) );
		entry.polygon.add( vector( 8321, 5263 ) );
		entry.polygon.add( vector( 8376, 4651 ) );
		entry.polygon.add( vector( 8454, 4517 ) );
		entry.polygon.add( vector( 8407, 3914 ) );
		entry.polygon.add( vector( 8981, 3277 ) );
		entry.polygon.add( vector( 10031, 2859 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "dragon pit", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::circle( vector( 9900, 4400 ), 650 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top river", e_team_type::neutral, e_zone_type::river );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 6458, 7414 ) );
		entry.polygon.add( vector( 7413, 8197 ) );
		entry.polygon.add( vector( 8002, 8915 ) );
		entry.polygon.add( vector( 7261, 9395 ) );
		entry.polygon.add( vector( 5979, 9622 ) );
		entry.polygon.add( vector( 5468, 9981 ) );
		entry.polygon.add( vector( 5690, 10307 ) );
		entry.polygon.add( vector( 5616, 10697 ) );
		entry.polygon.add( vector( 5284, 11078 ) );
		entry.polygon.add( vector( 4760, 11184 ) );
		entry.polygon.add( vector( 4464, 11867 ) );
		entry.polygon.add( vector( 4313, 12918 ) );
		entry.polygon.add( vector( 3246, 12423 ) );
		entry.polygon.add( vector( 2241, 11594 ) );
		entry.polygon.add( vector( 2638, 11283 ) );
		entry.polygon.add( vector( 2842, 10879 ) );
		entry.polygon.add( vector( 3052, 10351 ) );
		entry.polygon.add( vector( 3178, 10071 ) );
		entry.polygon.add( vector( 3260, 9906 ) );
		entry.polygon.add( vector( 3434, 9516 ) );
		entry.polygon.add( vector( 3698, 9230 ) );
		entry.polygon.add( vector( 4077, 9056 ) );
		entry.polygon.add( vector( 4272, 8931 ) );
		entry.polygon.add( vector( 4534, 8726 ) );
		entry.polygon.add( vector( 4204, 8446 ) );
		entry.polygon.add( vector( 4268, 8092 ) );
		entry.polygon.add( vector( 4405, 7815 ) );
		entry.polygon.add( vector( 4526, 7665 ) );
		entry.polygon.add( vector( 5163, 7419 ) );
		entry.polygon.add( vector( 5505, 7223 ) );
		entry.polygon.add( vector( 5806, 6875 ) );

		m_map_data.push_back( entry );
	}
#pragma endregion

#pragma region toplane
	{
		s_map_data_entry entry = s_map_data_entry( "top inhibitor zone", e_team_type::blue, e_zone_type::toplane );
		entry.polygon = geometry::rectangle( vector( 1250, 3900 ), vector( 1250, 6100 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top inner turret", e_team_type::blue, e_zone_type::toplane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 1512, 6700 ) );
		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top inner zone", e_team_type::blue, e_zone_type::toplane );
		entry.polygon = geometry::rectangle( vector( 1250, 6100 ), vector( 1250, 9880 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top outter turret", e_team_type::blue, e_zone_type::toplane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 981, 10441 ) );
		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top zone", e_team_type::blue, e_zone_type::toplane );
		entry.polygon = geometry::rectangle( vector( 1250, 3900 ), vector( 1250, 9880 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top neutral zone", e_team_type::neutral, e_zone_type::toplane );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 600, 9880 ) );
		entry.polygon.add( vector( 675, 10700 ) );
		entry.polygon.add( vector( 750, 11200 ) );
		entry.polygon.add( vector( 830, 11900 ) );
		entry.polygon.add( vector( 980, 12450 ) );
		entry.polygon.add( vector( 1150, 12750 ) );
		entry.polygon.add( vector( 1360, 13060 ) );
		entry.polygon.add( vector( 1590, 13360 ) );
		entry.polygon.add( vector( 1890, 13550 ) );
		entry.polygon.add( vector( 2260, 13770 ) );
		entry.polygon.add( vector( 2680, 13900 ) );
		entry.polygon.add( vector( 3050, 14040 ) );
		entry.polygon.add( vector( 3450, 14150 ) );
		entry.polygon.add( vector( 3950, 14250 ) );
		entry.polygon.add( vector( 4450, 14300 ) );
		entry.polygon.add( vector( 4880, 14300 ) );
		entry.polygon.add( vector( 4880, 13050 ) );
		entry.polygon.add( vector( 4280, 13050 ) );
		entry.polygon.add( vector( 3470, 12810 ) );
		entry.polygon.add( vector( 3285, 12710 ) );
		entry.polygon.add( vector( 3070, 12490 ) );
		entry.polygon.add( vector( 3220, 12050 ) );
		entry.polygon.add( vector( 2500, 11480 ) );
		entry.polygon.add( vector( 2270, 11660 ) );
		entry.polygon.add( vector( 2190, 11585 ) );
		entry.polygon.add( vector( 2020, 11360 ) );
		entry.polygon.add( vector( 1910, 11125 ) );
		entry.polygon.add( vector( 1850, 9880 ) );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top inhibitor zone", e_team_type::red, e_zone_type::toplane );
		entry.polygon = geometry::rectangle( vector( 10900, 13675 ), vector( 8400, 13675 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top inner turret", e_team_type::red, e_zone_type::toplane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 7943, 13412 ) );
		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top inner zone", e_team_type::red, e_zone_type::toplane );
		entry.polygon = geometry::rectangle( vector( 8400, 13675 ), vector( 4880, 13675 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top outter turret", e_team_type::red, e_zone_type::toplane );
		entry.polygon = geometry::polygon( );
		entry.polygon.add( vector( 4318, 13876 ) );
		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "top zone", e_team_type::red, e_zone_type::toplane );
		entry.polygon = geometry::rectangle( vector( 10900, 13675 ), vector( 4880, 13675 ), 775 ).to_polygon( );

		m_map_data.push_back( entry );
	}

	{
		s_map_data_entry entry = s_map_data_entry( "toplane generic", e_team_type::neutral, e_zone_type::toplane );
		entry.polygon = geometry::polygon( );

		entry.polygon.add( vector( 600, 5000 ) );
		entry.polygon.add( vector( 600, 11000 ) );
		entry.polygon.add( vector( 675, 10700 ) );
		entry.polygon.add( vector( 750, 11200 ) );
		entry.polygon.add( vector( 830, 11900 ) );
		entry.polygon.add( vector( 980, 12450 ) );
		entry.polygon.add( vector( 1150, 12750 ) );
		entry.polygon.add( vector( 1360, 13060 ) );
		entry.polygon.add( vector( 1590, 13360 ) );
		entry.polygon.add( vector( 1890, 13550 ) );
		entry.polygon.add( vector( 2260, 13770 ) );
		entry.polygon.add( vector( 2680, 13900 ) );
		entry.polygon.add( vector( 3050, 14040 ) );
		entry.polygon.add( vector( 3450, 14150 ) );
		entry.polygon.add( vector( 3950, 14250 ) );
		entry.polygon.add( vector( 4450, 14300 ) );
		entry.polygon.add( vector( 4920, 14300 ) );
		entry.polygon.add( vector( 4900, 14300 ) );
		entry.polygon.add( vector( 8280, 14300 ) );
		entry.polygon.add( vector( 9850, 14300 ) );
		entry.polygon.add( vector( 9850, 13050 ) );
		entry.polygon.add( vector( 8320, 13050 ) );
		entry.polygon.add( vector( 4850, 13050 ) );
		entry.polygon.add( vector( 4280, 13050 ) );
		entry.polygon.add( vector( 3470, 12810 ) );
		entry.polygon.add( vector( 3285, 12710 ) );
		entry.polygon.add( vector( 3070, 12490 ) );
		entry.polygon.add( vector( 3050, 12490 ) );
		entry.polygon.add( vector( 3220, 12050 ) );
		entry.polygon.add( vector( 2500, 11480 ) );
		entry.polygon.add( vector( 2270, 11660 ) );
		entry.polygon.add( vector( 2190, 11585 ) );
		entry.polygon.add( vector( 2020, 11360 ) );
		entry.polygon.add( vector( 1910, 11125 ) );
		entry.polygon.add( vector( 1850, 9625 ) );
		entry.polygon.add( vector( 1850, 5000 ) );

		m_map_data.push_back( entry );
	}
#pragma endregion

	for ( auto&& e : m_map_data )
		e.init( );
}

auto c_map_data::get_position_and_distance( vector pos ) -> std::optional< std::pair<std::string, float> >
{
	if ( m_map_data.empty( ) )
		return std::nullopt;

	auto inside = m_map_data;
	inside.erase( std::remove_if( inside.begin( ), inside.end( ), [ pos ]( s_map_data_entry e ) {
		if ( e.polygon.points.size( ) > 1 )
			return !e.polygon.is_inside( pos );
		else
			return true;
	} ), inside.end( ) );
	
	//zones
	if ( inside.empty( ) )
		inside = m_map_data;
	
	std::sort( inside.begin( ), inside.end( ), [ pos ]( s_map_data_entry e1, s_map_data_entry e2 ) {
		return e1.center.distance( pos ) < e2.center.distance( pos );
	} );

	auto&& closest = inside.front( );
	return std::make_pair( closest.get_display_name( ), closest.center.distance( myhero ) );
}