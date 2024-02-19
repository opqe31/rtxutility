#include "A_.h"
#include "level_up.h"
#include <iostream>
#include <random>
#include "utility.h"

namespace A_
{
	// Declaration of menu objects
	TreeTab* main_tab = nullptr;

	namespace show_chat
	{
		TreeEntry* show_chat = nullptr;
	}
	void create_menu()
	{
		main_tab = menu->get_tab("utility");

		if (main_tab)
		{
			auto info = main_tab->get_tab("utility_info");
			if (info)
			{
				info->add_separator("Dev0", " - - Author - - ");

				show_chat::show_chat = main_tab->add_checkbox("_aio_important_tips", "Important tips", true);

				std::string version = std::to_string( BUILD_YEAR ) + "." + std::to_string( BUILD_MONTH ) + "." + std::to_string( BUILD_DAY );
				main_tab->add_separator( "Version1", "Version: " + version );
			}
		}
		if ( A_::utils_show_chat( ) )
		{
			std::string msg1 = utils_use_chinese( ) ?
				"<b><font color=\"#ffeaa7\">[] </font></b><b><font color=\"#DCD8C0\"> 實用工具 </font></b>" :
				"<b><font color=\"#ffeaa7\">[] </font></b><b><font color=\"#DCD8C0\"> Utils </font></b>";

			myhero->print_chat(0x0, msg1.c_str());

			/*std::string msg2 = utils_use_chinese( ) ?
				"<b><font color=\"#ffeaa7\">[Lune & ] </font></b><b><font color=\"#DCD8C0\"> Discord : _aio </font></b>" :
				"<b><font color=\"#ffeaa7\">[Lune & ] </font></b><b><font color=\"#DCD8C0\"> Discord : _aio </font></b>";
			myhero->print_chat(0x0, msg2.c_str());*/

			std::string msg3 = utils_use_chinese( ) ?
			myhero->print_chat(0x0, msg3.c_str());
		}
		level_up::load_preset(true);

	}
	bool utils_show_chat()
	{
		static auto obs_bypass = menu->get_tab( "visuals" ) ? menu->get_tab( "visuals" )->get_entry( "obs_bypass" ) : nullptr;
		if ( obs_bypass == nullptr || obs_bypass->get_bool( ) )
			return false;

		if ( show_chat::show_chat && show_chat::show_chat->get_bool( )  )
			return true;
		else
			return false;
	}

	bool obs_safe( )
	{
		static auto obs_bypass = menu->get_tab( "visuals" ) ? menu->get_tab( "visuals" )->get_entry( "obs_bypass" ) : nullptr;
		if ( obs_bypass == nullptr || !obs_bypass->get_bool( ) )
			return false;

		return true;
	}

	auto is_on_screen( vector v ) -> bool
	{
		auto w2s = vector::zero;
		renderer->world_to_screen( v, w2s );

		if ( !w2s.is_valid( ) )
			return false;

		return w2s.is_on_screen( );
	}

	bool use_chinese( )
	{
		const auto core_menu = menu->get_tab( "core" );
		if ( core_menu )
		{
			const auto langue_menu = core_menu->get_entry( "lang_hash" );
			if ( langue_menu && langue_menu->get_int( ) == 1630118516 )
				return true;
		}

		return false;
	}
	bool utils_use_chinese()
	{
		static auto obs_bypass = menu->get_tab( "visuals" ) ? menu->get_tab( "visuals" )->get_entry( "obs_bypass" ) : nullptr;
		if ( obs_bypass == nullptr || !obs_bypass->get_bool( ) )
			return false;

		if ( use_chinese( ) )
			return true;
		else
			return false;
	}
	void on_load()
	{
		create_menu();
	}

	auto randomize( const vector& current_position, float min, float max ) -> vector
	{
		static std::random_device rd;
		static std::mt19937 gen( rd( ) );

		std::uniform_real_distribution<float> dist( min, max );
		std::uniform_real_distribution<float> dist2( min, max );

		auto x = current_position.x + dist( gen );
		auto y = current_position.y + dist2( gen );
		auto z = current_position.z;

		return vector( x, y, z );
	}

	auto random_float( float min, float max ) -> float
	{
		static std::random_device rd;
		static std::mt19937 gen( rd( ) );

		std::uniform_real_distribution<float> dist( std::min( min, max ), std::max( min, max ) );

		return dist( gen );
	}

	auto explode_string( std::string input, char delimiter ) -> std::vector<std::string>
	{
		std::vector<std::string> result = { };
		std::string temp = "";

		if ( input.size( ) <= 0 )
			return result;

		for ( auto c : input )
		{
			if ( c == delimiter )
			{
				if ( temp.size( ) > 0 )
				{
					result.push_back( temp );
					temp = "";
				}
			}
			else
				temp.push_back( c );
		}

		if ( temp.size( ) > 0 )
			result.push_back( temp );

		return result;
	}

	std::vector<vector> arc_points( float percent, const vector center, float radius, std::optional<int> opt_num_points, float start_angle )
	{
		auto arc_points = std::vector<vector> { };

		auto end_angle = start_angle + ( 360.f * percent );

		start_angle = degrees_to_radians( start_angle );
		end_angle = degrees_to_radians( end_angle );

		auto num_points = opt_num_points.has_value( ) ? opt_num_points.value( ) : static_cast< int >( std::ceil( ( end_angle - start_angle ) / ( 2.f * M_PI ) * 100.f ) );

		for ( int i = 0; i <= num_points; i++ )
		{
			auto angle = start_angle + ( i * ( end_angle - start_angle ) / num_points );
			vector point;
			point.x = center.x + radius * std::cos( angle );
			point.y = center.y + radius * std::sin( angle );
			arc_points.push_back( point );
		}

		return arc_points;
	}

	//ward
	float cast_ward_time;
	const std::vector<ItemId> ward =
	{
		ItemId::Runesteel_Spaulders,
		ItemId::Pauldrons_of_Whiterock,
		ItemId::Frostfang,
		ItemId::Shard_of_True_Ice,
		ItemId::Harrowing_Crescent,
		ItemId::Black_Mist_Scythe,
		ItemId::Stealth_Ward,
		ItemId::Control_Ward
	};
	bool can_ward()
	{
		for (auto& itemId : ward)
		{
			auto slot = myhero->has_item(itemId);
			if (slot != spellslot::invalid && myhero->is_item_ready(itemId))
			{
				return true;
			}
		}

		return false;
	}
	void cast_ward(vector pos)
	{
		if (can_ward() && gametime->get_time() > cast_ward_time)
		{
			for (auto& itemId : ward)
			{
				auto slot = myhero->has_item(itemId);
				if (slot != spellslot::invalid && myhero->is_item_ready(itemId))
				{
					myhero->cast_spell(slot, pos);
					cast_ward_time = gametime->get_time() + 0.5f;
					return;
				}
			}
		}
	}
	game_object_script get_nearby_ward(vector pos, float range)
	{
		for (auto& object : entitylist->get_other_minion_objects())
		{
			if (!object->is_valid() || object->is_dead()) continue;

			auto hash = buff_hash_real( object->get_name_cstr( ) );
			if ( hash != buff_hash( "SightWard" ) && hash != buff_hash( "JammerDevice" ) && hash != buff_hash( "VisionWard" )) continue;
			if (object->get_distance(pos) >= range) continue;
			return object;
		}
		return nullptr;
	}

	auto is_jungler( game_object_script x ) -> bool
	{
		static std::map<uint32_t, bool> m_jungler = { };
		auto it_jg = m_jungler.find( x->get_network_id( ) );

		if ( it_jg == m_jungler.end( ) )
		{
			auto spell_s1 = x->get_spell( spellslot::summoner1 ),
				spell_s2 = x->get_spell( spellslot::summoner2 );

			m_jungler [ x->get_network_id( ) ] =
				( ( spell_s1 && spell_s1->get_name( ).find( "SummonerSmite" ) != std::string::npos ) || ( spell_s2 && spell_s2->get_name( ).find( "SummonerSmite" ) != std::string::npos ) );
		}
		else
			return it_jg->second;

		return false;
	};

	auto get_nexus_pos( bool enemy ) -> vector
	{
		static vector nexuses [ 2 ] = { vector::zero, vector::zero };
		static auto b_init = false;
		if ( !b_init )
		{
			for ( auto&& x : entitylist->get_all_nexus( ) )
			{
				if ( !x || !x->is_valid( ) )
					continue;

				nexuses [ x->is_enemy( ) ? 0 : 1 ] = x->get_position( );
			}

			b_init = true;
		}

		return nexuses[ enemy ? 0 : 1 ];
	}

	//draw
	void draw_arc(const vector& pos, float radius, int start_angle, int end_angle, unsigned long color, float thickness) {

		auto direction = (pos - (pos - vector(0, 100, 0))).normalized();
		float angle_step = 1.0f; // 设置角度步长，可以根据需要调整精度

		for (int i = start_angle; i <= end_angle; i += static_cast<int>( angle_step )) {

			float radians1 = i / 180.0f * M_PI;
			float radians2 = (i + angle_step) / 180.0f * M_PI;
			auto a = pos + direction.rotated(radians1) * radius;
			auto b = pos + direction.rotated(radians2) * radius;

			draw_manager->add_line(a, b, color, thickness);
		}
	}

	//不扣血
	bool has_unkillable_buff(game_object_script target)
	{
		return target->is_zombie() || target->has_buff({ buff_hash("UndyingRage"), buff_hash("ChronoShift"), buff_hash("KayleR"), buff_hash("KindredRNoDeathBuff") });;
	}

	//普攻無效
	bool has_untargetable_buff(game_object_script target)
	{
		return target->has_buff({ buff_hash("JaxCounterStrike"), buff_hash("ShenW") });;
	}

	//技能免疫 無效
	bool has_skill_protect_buff(game_object_script target)
	{
		return target->has_buff({
			buff_hash("itemmagekillerveil"),
			buff_hash("bansheesveil"),
			buff_hash("SivirE"),
			buff_hash("MorganaE"),
			buff_hash("MorganaW"),
			buff_hash("ekkorinvuln"),
			buff_hash("braumeshieldbuff"),
			buff_hash("virdunktargetself"),
			buff_hash("VladimirSanguinePool"),
			buff_hash("SionR"),
			buff_hash("malzharpassiveshield"),
			buff_hash("DrMundoPlmmunity"),
			buff_hash("corkiloadupcd"),
			buff_hash("unstoppableforcemarker"),
			buff_hash("jarvanivcataclysmsound") });
		//OrnnW EkkoR volibear->no *hecarimR KledR RammusR Malphite_Base_R_cas_trail SettR WarwickR yoneQ2 UdyrE2 IllaoiR
	}

	//技能免疫 無效
	bool has_Invincible_buff(game_object_script target)
	{
		return target->has_buff_type({
			buff_type::SpellImmunity,
			buff_type::SpellShield,

			});
	}

	bool has_crowd_control_buff(game_object_script target)
	{
		return target->has_buff_type({
			buff_type::Stun,
			buff_type::Knockup,
			buff_type::Knockback,
			buff_type::Asleep,
			buff_type::Berserk,
			buff_type::Charm,
			buff_type::Flee,
			buff_type::Fear,
			buff_type::Snare,
			buff_type::Suppression,
			buff_type::Polymorph,
			buff_type::Taunt
			});
	}

	//checkbox
	bool checkbox(std::map<std::uint32_t, TreeEntry*>& map, game_object_script target)
	{
		auto it = map.find(target->get_network_id());
		if (it == map.end())
			return false;

		return it->second->get_bool();
	}

	bool checkbox( std::map<champion_id, TreeEntry*>& map, game_object_script target )
	{
		if ( map.find( target->get_champion( ) ) == map.end( ) )
			return false;

		return map[ target->get_champion( )]->get_bool( );
	}

	int get_slider( std::map<std::uint32_t, TreeEntry*>& map, game_object_script target )
	{
		auto it = map.find( target->get_network_id( ) );
		if ( it == map.end( ) )
			return false;

		return it->second->get_int( );
	}

	//算小兵
	int32_t count_minions_in_range(float range)
	{
		int32_t count = 0;

		for (auto&& minion : entitylist->get_enemy_minions())
		{
			if (minion->is_valid( ) && !minion->is_dead() && minion->is_visible() && minion->is_targetable() && myhero->get_distance(minion) < range)
				count++;
		}

		return count;
	}
	int32_t count_monsters_in_range(float range)
	{
		int32_t count = 0;

		for (auto&& minion : entitylist->get_jugnle_mobs_minions())
		{
			if ( minion->is_valid( ) && !minion->is_dead() && minion->is_visible() && minion->is_targetable() && myhero->get_distance(minion) < range)
				count++;
		}

		return count;
	}

	auto draw_gradient_arc( vector center_point, float radius, float start_angle, float end_angle, unsigned long color1, unsigned long color2, 
		float thickness, vector v_from ) -> void
	{
		const int num_segments = 100;
		auto angle_step = ( end_angle - start_angle ) / num_segments;

		// Extract the RGBA components from color1 and color2
		auto r1 = ( color1 >> 16 ) & 0xFF,
			g1 = ( color1 >> 8 ) & 0xFF,
			b1 = color1 & 0xFF,
			a1 = ( color1 >> 24 ) & 0xFF;

		auto r2 = ( color2 >> 16 ) & 0xFF,
			g2 = ( color2 >> 8 ) & 0xFF,
			b2 = color2 & 0xFF,
			a2 = ( color2 >> 24 ) & 0xFF;

		// Calculate the color differences
		int dr = r2 - r1,
			dg = g2 - g1,
			db = b2 - b1,
			da = a2 - a1;

		vector v_from_vec = v_from;
		if ( !v_from_vec.is_valid( ) )
			v_from_vec = myhero->get_position( );

		// Draw the arc by adding line segments
		for ( auto i = 0; i < num_segments; i++ )
		{
			auto current_angle = start_angle + ( i * angle_step );
			auto next_angle = start_angle + ( ( i + 1 ) * angle_step );

			auto r = r1 + static_cast< unsigned char >( ( dr < 0 ) ? -( ( -dr * i ) / num_segments ) : ( ( dr * i ) / num_segments ) ),
				g = g1 + static_cast< unsigned char >( ( dg < 0 ) ? -( ( -dg * i ) / num_segments ) : ( ( dg * i ) / num_segments ) ),
				b = b1 + static_cast< unsigned char >( ( db < 0 ) ? -( ( -db * i ) / num_segments ) : ( ( db * i ) / num_segments ) ),
				a = a1 + static_cast< unsigned char >( ( da < 0 ) ? -( ( -da * i ) / num_segments ) : ( ( da * i ) / num_segments ) );

			unsigned long current_color = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;

			// Calculate the end point of the current line segment

			auto end_point_r = center_point.extend( v_from_vec, radius );
			vector start_point = center_point + ( center_point.extend( end_point_r, radius ) - center_point ).rotated( degrees_to_radians( current_angle ) );
			vector end_point = center_point + ( end_point_r - center_point ).rotated( degrees_to_radians( next_angle ) );

			// Add the line segment to the scene
			draw_manager->add_line( start_point, end_point, current_color, thickness );
		}
	}

	auto draw_corner_box( vector min, vector max, unsigned long color, bool animate, float length, float thickness ) -> void
	{
		auto l = length <= 0 ? std::abs( max.x - min.x ) / 3.f : length;

		if ( min.x > max.x )
			std::swap( min.x, max.x );

		if ( animate )
		{
			min = A_::animate_vector( min, max, l / 2.f, thickness );
			max = A_::animate_vector( max, min, l / 2.f, thickness );
		}

		//R_CORNER_TOP
		draw_manager->add_line_on_screen( max, vector( max.x - l, max.y ), color, thickness );
		draw_manager->add_line_on_screen( max, vector( max.x, max.y + l ), color, thickness );

		//L_CORNER_TOP
		draw_manager->add_line_on_screen( vector( min.x, max.y ), vector( min.x + l, max.y ), color, thickness );
		draw_manager->add_line_on_screen( vector( min.x, max.y ), vector( min.x, max.y + l ), color, thickness );

		//L_CORNER_BOT
		draw_manager->add_line_on_screen( vector( min.x, min.y ), vector( min.x + l, min.y ), color, thickness );
		draw_manager->add_line_on_screen( vector( min.x, min.y ), vector( min.x, min.y - l ), color, thickness );

		//R_CORNER_BOT
		draw_manager->add_line_on_screen( vector( max.x, min.y ), vector( max.x - l, min.y ), color, thickness );
		draw_manager->add_line_on_screen( vector( max.x, min.y ), vector( max.x, min.y - l ), color, thickness );
	}

	unsigned long set_alpha(unsigned long color, unsigned long alpha)
	{
		return (color & 0X00FFFFFF) | (alpha << 24);
	}

	auto get_alpha( unsigned long color ) -> unsigned long
	{
		return ( color >> 24 ) & 0XFF;
	}

	unsigned long set_alpha_percent( unsigned long color, float alpha )
	{
		auto current_alpha = get_alpha( color );

		return set_alpha( color, static_cast< unsigned long >( current_alpha * alpha ) );
	}

	auto animate_vector( vector min_vec, vector max_vec, float l, float period ) -> vector
	{
		auto phase = fmod( gametime->get_time( ), period ) / period;
		auto factor = ( cos( 2 * M_PI * phase ) + 1.f ) / 2.f;

		return min_vec.extend( max_vec, factor * l );
	}

	unsigned long animate_color(unsigned long color, float min_alpha, float max_alpha, float duration, float period)
	{
		auto alpha_range = max_alpha - min_alpha;

		auto phase = fmodf(gametime->get_time(), period) / period;
		auto alpha_value = 0.5f * alpha_range * sinf(2 * M_PI * phase) + 0.5f * alpha_range + min_alpha;
		return set_alpha(color, static_cast<uint32_t>( alpha_value) );
	}

	auto get_color( float pcent ) -> unsigned long
	{
		//rgb( 0, 184, 148 ) - rgb( 255, 121, 121 )
		auto real_pcent = std::max( 0.f, std::min( 100.f, pcent ) );

		return MAKE_COLOR( 255 - ( int )( real_pcent * 2.55f ), ( int )( real_pcent * 2.55f ), 20, 255 );
	}

	auto d3d_to_menu( unsigned long long color, float* rgba ) -> void
	{
		//((unsigned long)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))
		rgba [ 0 ] = ( color & 0xff ) / 255.f; //r
		rgba [ 1 ] = ( ( color >> 8 ) & 0xff ) / 255.f; //g
		rgba [ 2 ] = ( ( color >> 16 ) & 0xff ) / 255.f; //b
		rgba [ 3 ] = ( ( color >> 24 ) & 0xff ) / 255.f; //a
	}

	auto str_replace( std::string& str, const std::string& from, const std::string& to ) -> bool
	{
		auto start_pos = str.find( from );
		if ( start_pos == std::string::npos )
			return false;

		str.replace( start_pos, from.length( ), to );
		return true;
	}

	//d
	std::vector<uint32_t> inv_shield_hashes =
	{
		buff_hash( "NocturneShroudofDarkness" ),
		buff_hash( "itemmagekillerveil" ),
		buff_hash( "bansheesveil" ),
		buff_hash( "SivirE" ),
		buff_hash( "malzaharpassiveshield" )
	};

	std::vector<uint32_t> spell_immune_shield_hashes =
	{
		buff_hash( "NocturneShroudofDarkness" ),
		buff_hash( "itemmagekillerveil" ),
		buff_hash( "bansheesveil" ),
		buff_hash( "SivirE" ),
		buff_hash( "malzaharpassiveshield" ),

		buff_hash( "MorganaE" )
	};

	auto is_spell_immune( game_object_script x ) -> bool
	{
		return x->has_buff( spell_immune_shield_hashes );
	}

	auto can_be_killed( game_object_script x, bool check_spellshield ) -> bool
	{
		if ( x->has_buff( {
			buff_hash( "KindredRNoDeathBuff" ),
			buff_hash( "UndyingRage" ),
			buff_hash( "ChronoRevive" ),
			buff_hash( "ChronoShift" ),
			buff_hash( "FioraW" ),

			buff_hash( "TaricR" ),
			buff_hash( "KayleR" ),
			buff_hash( "PantheonE" ),
			buff_hash( "XinZhaoRRangedImmunity" )
			} ) || x->is_zombie( ) )
			return false;

		if ( check_spellshield && x->has_buff( inv_shield_hashes ) )
			return false;

		return true;
	}

	auto is_invulnerable( game_object_script target, float delay, bool jax ) -> bool
	{
		if ( target == nullptr || !target->is_valid( ) )
			return false;

		auto b_invulnerable = false;
		for ( auto&& buff : target->get_bufflist( ) )
		{
			if ( buff == nullptr || !buff->is_valid( ) || !buff->is_alive( ) )
				continue;

			if ( buff->get_type( ) == buff_type::Invulnerability && buff->get_remaining_time( ) + ( ping->get_ping( ) / 2000.f ) <= delay )
				b_invulnerable = true;

			switch ( buff->get_hash_name( ) )
			{
				case buff_hash( "JaxCounterStrike" ):
					if ( jax && myhero->get_champion( ) == champion_id::MasterYi )
					{
						b_invulnerable = buff->get_remaining_time( ) <= 0.75f;
						break;
					}
					else if ( jax )
						b_invulnerable = buff->get_remaining_time( ) <= delay;
					break;
				case buff_hash( "TaricR" ):
				case buff_hash( "KayleR" ):
				case buff_hash( "KindredRNoDeathBuff" ):
				case buff_hash( "UndyingRage" ):
				case buff_hash( "fioraw" ):
				case buff_hash( "PantheonE" ):
				case buff_hash( "XinZhaoRRangedImmunity" ):
					return buff->get_remaining_time( ) + ( ping->get_ping( ) / 1000.f ) <= delay;

				case buff_hash( "ChronoRevive" ):
				case buff_hash( "ChronoShift" ):
					if ( target->get_health_percent( ) <= 33 )
					{
						return buff->get_remaining_time( ) + ( ping->get_ping( ) / 1000.f ) <= delay;
					}
					break;
				case buff_hash( "VladimirSanguinePool" ):
				case buff_hash( "lissandrarself" ):
				{
					return buff->get_remaining_time( ) >= ping->get_ping( ) / 2000.f;
				}
			}
		}

		return b_invulnerable;
	}

	bool is_epic_monster( std::string name_lower )
	{
		return name_lower.find( "sru_baron" ) != std::string::npos || name_lower.find( "sru_dragon" ) != std::string::npos || name_lower.find( "sru_riftherald" ) != std::string::npos;
	}
	bool is_jungle_buff( std::string name_lower )
	{
		return name_lower.find( "sru_red" ) != std::string::npos || name_lower.find( "sru_blue" ) != std::string::npos;
	}

	bool is_smite_monster( std::string name )
	{
		if ( is_jungle_buff( name ) )
			return true;

		if ( is_epic_monster( name ) )
			return true;

		if ( name.find( "sru_gromp" ) != std::string::npos ||
			( name.find( "sru_murkwolf" ) != std::string::npos && name.find( "mini" ) == std::string::npos ) ||
			( name.find( "sru_razorbeak" ) != std::string::npos && name.find( "mini" ) == std::string::npos ) ||
			( name.find( "sru_krug" ) != std::string::npos && name.find( "mini" ) == std::string::npos ) ||
			name.find( "sru_crab" ) != std::string::npos
			)
			return true;

		return false;
	}

	bool is_monster( std::string name )
	{
		if ( is_jungle_buff( name ) )
			return true;

		if ( is_epic_monster( name ) )
			return true;

		if ( name.find( "sru_gromp" ) != std::string::npos ||
			( name.find( "sru_murkwolf" ) ) ||
			( name.find( "sru_razorbeak" ) ) ||
			( name.find( "sru_krug" ) ) ||
			name.find( "sru_crab" ) != std::string::npos
			)
			return true;

		return false;
	}

	bool is_monster( game_object_script x )
	{
		auto name = x->get_name( );
		std::transform( name.begin( ), name.end( ), name.begin( ),
			[ ]( unsigned char c ) { return std::tolower( c ); } );

		if ( is_jungle_buff( name ) )
			return true;

		if ( is_epic_monster( name ) )
			return true;

		if ( name.find( "sru_gromp" ) != std::string::npos ||
			( name.find( "sru_murkwolf" ) ) ||
			( name.find( "sru_razorbeak" ) ) ||
			( name.find( "sru_krug" ) ) ||
			name.find( "sru_crab" ) != std::string::npos
			)
			return true;

		return false;
	}

	auto arc_poly( vector start, vector direction, float angle, float radius, int quality ) -> geometry::polygon
	{
		auto start_pos = start;
		auto end_pos = ( direction - start ).normalized( );

		geometry::polygon poly = geometry::polygon( );

		auto out_radius = radius / cosf( 2.f * M_PI / quality );
		auto side1 = end_pos.rotated( -angle * 0.5f );

		for ( auto i = 0; i <= quality; i++ )
		{
			auto cdirection = side1.rotated( i * angle / quality ).normalized( );
			poly.add( vector(
				start_pos.x + out_radius * cdirection.x,
				start_pos.y + out_radius * cdirection.y
			) );
		}

		return poly;
	}
}