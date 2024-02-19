#include "../plugin_sdk/plugin_sdk.hpp"
#include "champion.h"
#include "A_.h"

namespace champion
{
	namespace hero
	{
		TreeEntry* only_show = nullptr;

		TreeEntry* taric_e_magnet = nullptr;
		TreeEntry* taric_e_type = nullptr;
		TreeEntry* taric_e_magnet_combo_mode = nullptr;
		TreeEntry* taric_e_magnet_evade = nullptr;
		TreeEntry* taric_e_tower = nullptr;

		TreeEntry* cassiopeia_r_dodge = nullptr;

		TreeEntry* xayah_e = nullptr;
		TreeEntry* xayah_e_style;
		TreeEntry* xayah_e_hit_color;
		TreeEntry* xayah_e_color;

		TreeEntry* katarina_p_hit_color;
		TreeEntry* katarina_p_color;

		TreeEntry* use_lantern = nullptr;
		TreeEntry* use_lantern_range = nullptr;
		TreeEntry* ward_lantern = nullptr;

		TreeEntry* yasuo_e = nullptr;
		TreeEntry* kayn_e = nullptr;

		TreeEntry* katarina_p = nullptr;

		uint32_t* kayn_e_texture = nullptr;
		uint32_t kayn_e_nid = 0;
	}

#pragma region menu
	void create_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto hero_tab = main_tab->get_tab("hero");

		hero::only_show = hero_tab->add_checkbox("show", "Show all hero", false);

		auto taric = hero_tab->add_tab("taric", "Taric");
		{
			taric->add_separator("taric", " - - Taric - - ");
			hero::taric_e_magnet = taric->add_checkbox("taric_e_magnet", "Taric E magnet", false);
			taric->set_assigned_active(hero::taric_e_magnet);
			for (auto& team : entitylist->get_all_heroes())
			{
				if (team->get_champion() == champion_id::Taric)
					taric->set_assigned_texture(team->get_square_icon_portrait());
			}
			hero::taric_e_type = taric->add_combobox("taric_e_type", "Taric E magnet", { {"Target selector",nullptr },{"Close to me",nullptr },{"Close to mouse",nullptr } }, 0, true);
			hero::taric_e_type->add_property_change_callback( []( TreeEntry* )
				{
					orbwalker->set_movement( true );
					orbwalker->set_attack( true );
					evade->enable_evade( );
				} );
			
			hero::taric_e_magnet_combo_mode = taric->add_checkbox("taric_e_magnet_combo_mode", "  ^- Only with combo mode", true);
			hero::taric_e_magnet_evade = taric->add_checkbox("taric_e_magnet_evade", "  ^- E Magnet disable evade", true);
			hero::taric_e_tower = taric->add_checkbox("taric_e_tower", "  ^- Dont use magnet in turret", true);
		}

		auto cassiopeia = hero_tab->add_tab("cassiopeia", "Cassiopeia/Tryndamere");
		{
			cassiopeia->add_separator("cassiopeia", " - - Cassiopeia and Tryndamere - - ");
			for (auto& team : entitylist->get_all_heroes())
			{
				if (team->get_champion() == champion_id::Cassiopeia)
				{
					cassiopeia->set_assigned_texture(team->get_square_icon_portrait());
				}
				else if (team->get_champion() == champion_id::Tryndamere)
				{
					cassiopeia->set_assigned_texture(team->get_square_icon_portrait());
				}
			}
			hero::cassiopeia_r_dodge = cassiopeia->add_checkbox("cassiopeia_r", "Dodge Cassiopeia R / Tryndamere W", true);
			cassiopeia->set_assigned_active(hero::cassiopeia_r_dodge);
		}

		auto xayah = hero_tab->add_tab("xayah", "Xayah");
		{
			xayah->add_separator("xayah", " - - Xayah - - ");
			for (auto& team : entitylist->get_all_heroes())
			{
				if (team->get_champion() == champion_id::Xayah)
				{
					xayah->set_assigned_texture(team->get_square_icon_portrait());
				}
			}
			hero::xayah_e = xayah->add_checkbox("xayah_e", "Draw Xayah E", true);
			auto spell_xayah_e = database->get_spell_by_hash( spell_hash( "XayahPassive" ) );
			if ( spell_xayah_e && spell_xayah_e->get_icon_texture_by_index( 0 ) )
				hero::xayah_e->set_texture( spell_xayah_e->get_icon_texture_by_index( 0 ) );

			hero::xayah_e_style = xayah->add_combobox( "xayah_e_style", "Style", { { "Lines", nullptr }, { "Rectangles", nullptr },  { "Shapes", nullptr } }, 2 );
			//auto color = poly.is_inside( myhero->get_position( ) ) ? MAKE_COLOR( 255, 0, 0, 255 ) : MAKE_COLOR( 255, 255, 255, 255 );
			float color_hit [ 4 ] = { 1.f, 0.f, 0.f, 1.f };
			float color [ 4 ] = { 1.f, 1.f, 1.f, 1.f };
			hero::xayah_e_hit_color = xayah->add_colorpick( "xayah_e_hit_color", "Color hit", color_hit );
			hero::xayah_e_color = xayah->add_colorpick( "xayah_e_color", "Color", color );
			xayah->set_assigned_active(hero::xayah_e);

		}

		auto thresh = hero_tab->add_tab("thresh", "Thresh");
		{
			thresh->add_separator("thresh", " - - Thresh - - ");
			for (auto& team : entitylist->get_all_heroes())
			{
				if (team->get_champion() == champion_id::Thresh)
				{
					thresh->set_assigned_texture(team->get_square_icon_portrait());
				}
			}

			hero::use_lantern = thresh->add_hotkey("use_lantern", "Use lantern", TreeHotkeyMode::Hold, 'Z', false);
			hero::use_lantern_range = thresh->add_slider("use_lantern_range", "Use lantern range", 400, 0, 600);
			hero::ward_lantern = thresh->add_combobox("ward_lantern", "Ward enemy lantern", { {" logic",nullptr},{"Always",nullptr },{"None",nullptr } }, 0, true);
			thresh->set_assigned_active(hero::use_lantern);
		}

		auto yasuo = hero_tab->add_tab("yasuo", "Yasuo");
		{
			yasuo->add_separator("yasuo", " - - Yasuo - - ");
			for (auto& team : entitylist->get_all_heroes())
			{
				if (team->get_champion() == champion_id::Yasuo)
				{
					yasuo->set_assigned_texture(team->get_square_icon_portrait());
				}
			}
			hero::yasuo_e = yasuo->add_checkbox("yasuo_e", "Draw Yasuo E", true);
			yasuo->set_assigned_active(hero::yasuo_e);
		}

		//871182340 Kayn_Base_Primary_E_Warning
		auto kayn = hero_tab->add_tab( "kayn", "Kayn" );
		{
			kayn->add_separator( "kayn", " - - Kayn - - " );
			hero::kayn_e = kayn->add_checkbox( "kayn_e", "Draw Kayn E", true );
			for ( auto& team : entitylist->get_all_heroes( ) ) //Viego -> Kayn
			{
				if ( team->get_champion( ) == champion_id::Kayn )
				{
					kayn->set_assigned_texture( team->get_square_icon_portrait( ) );
					hero::kayn_e_texture = team->get_spell( spellslot::e )->get_icon_texture( );
					hero::kayn_e->set_texture( hero::kayn_e_texture );
				}
			}
			
			kayn->set_assigned_active( hero::kayn_e );
		}

		auto kata = hero_tab->add_tab("kata", "Katarina");
		{
			kata->add_separator("kata", " - - Katarina - - ");
			for (auto& team : entitylist->get_all_heroes())
			{
				if (team->get_champion() == champion_id::Katarina)
				{
					kata->set_assigned_texture(team->get_square_icon_portrait());
				}
			}
			hero::katarina_p = kata->add_checkbox("katarina_p", "Draw Dagger", true);
			auto spell_p_kata = database->get_spell_by_hash( spell_hash( "KatarinaPassive" ) );
			if ( spell_p_kata && spell_p_kata->get_icon_texture_by_index( 0 ) )
				hero::katarina_p->set_texture( spell_p_kata->get_icon_texture_by_index( 0 ) );

			float color_hit [ 4 ] = { 1.f, 0.f, 0.f, 1.f };
			float color [ 4 ] = { 1.f, 1.f, 1.f, 1.f };
			hero::katarina_p_hit_color = kata->add_colorpick( "katarina_p_hit_color", "Color hit", color_hit );
			hero::katarina_p_color = kata->add_colorpick( "katarina_p_color", "Color", color );

			kata->set_assigned_active(hero::katarina_p);
		}
	}
	void update_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto hero_tab = main_tab->get_tab("hero");
		auto show_taric = hero_tab->get_tab("taric");
		auto show_cassiopeia = hero_tab->get_tab("cassiopeia");
		auto show_xayah = hero_tab->get_tab("xayah");
		auto show_thresh = hero_tab->get_tab("thresh");
		auto show_yasuo = hero_tab->get_tab("yasuo");
		auto show_kata = hero_tab->get_tab("kata");
		auto show_kayn = hero_tab->get_tab("kayn");
		bool found_taric = false, found_cass = false, found_xayah = false, found_thresh = false, found_yasuo = false, found_kata = false, found_kayn = false;
		if (!hero::only_show->get_bool())
		{
			for (auto& team : entitylist->get_ally_heroes())
			{
				if ( team->get_champion( ) == champion_id::Taric )
					found_taric = true;
				else if ( team->get_champion( ) == champion_id::Thresh )
					found_thresh = true;
			}
			for (auto& enemy : entitylist->get_enemy_heroes())
			{
				switch ( enemy->get_champion( ) )
				{
				case champion_id::Cassiopeia: 
				case champion_id::Tryndamere:
					found_cass = true;
					break;
				case champion_id::Xayah: 
					found_xayah = true;
					break;
				case champion_id::Yasuo:
					found_yasuo = true;
					break;
				case champion_id::Katarina:
					found_kata = true;
					break;
				case champion_id::Thresh:
					found_thresh = true;
					break;
				default:
					break;
				case champion_id::Kayn:
					found_kayn = true;
					break;
				}
			}

			show_taric->is_hidden() = !found_taric;
			show_cassiopeia->is_hidden() = !found_cass;
			show_xayah->is_hidden() = !found_xayah;
			show_thresh->is_hidden() = !found_thresh;
			show_yasuo->is_hidden() = !found_yasuo;
			show_kata->is_hidden() = !found_kata;
			show_kayn->is_hidden() = !found_kayn;
		}
		if (hero::only_show->get_bool())
		{
			show_taric->is_hidden() = false;
			show_cassiopeia->is_hidden() = false;
			show_xayah->is_hidden() = false;
			show_thresh->is_hidden() = false;
			show_yasuo->is_hidden() = false;
			show_kata->is_hidden() = false;
			show_kayn->is_hidden() = false;
		}
	}
#pragma endregion

#pragma region taric
	game_object_script taric_e_target;

	vector taric_e_end, taric_magnet_pos, taric_target_pos;
	float taric_last_e_time, taric_evade_time;
	bool taric_orb, taric_use_evade, taric_evade_check;
	void taric_e_logic( )
	{
		if (!hero::taric_e_magnet->get_bool())
			return;

		static auto t_last_order = -9.f;
		if ( gametime->get_time( ) - t_last_order <= 0.05f )
			return;

		switch ( hero::taric_e_type->get_int( ) )
		{
		default:
			break;
		case 0:
			taric_e_target = target_selector->get_target( 575 + 300, damage_type::magical );
			break;

		case 1:
		{
			auto enemy = entitylist->get_enemy_heroes( );

			enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), []( game_object_script x )
				{
					return !x || !x->is_valid( ) || x->get_distance( myhero ) > 875 || x->is_dead( );
				} ), enemy.end( ) );

			std::sort( enemy.begin( ), enemy.end( ), []( game_object_script a, game_object_script b )
				{
					return a->get_distance( myhero ) < b->get_distance( myhero );
				} );
			if ( !enemy.empty( ) )
				taric_e_target = enemy.front( );
			break;
		}

		case 2:
		{
			auto enemy = entitylist->get_enemy_heroes( );

			enemy.erase( std::remove_if( enemy.begin( ), enemy.end( ), []( game_object_script x )
				{
					return !x || !x->is_valid( ) || x->get_distance( myhero ) > 875 || x->is_dead( );
				} ), enemy.end( ) );

			std::sort( enemy.begin( ), enemy.end( ), []( game_object_script a, game_object_script b )
				{
					auto hud_pos = hud->get_hud_input_logic( )->get_game_cursor_position( );
					return a->get_distance( hud_pos ) < b->get_distance( hud_pos );
				} );
			if ( !enemy.empty( ) )
				taric_e_target = enemy.front( );
		}
		}

		auto taric_e_115_pos = myhero->get_position( ) + taric_e_end * 115;
		auto taric_e_230_pos = myhero->get_position( ) + taric_e_end * 230;
		auto taric_e_345_pos = myhero->get_position( ) + taric_e_end * 345;
		auto taric_e_460_pos = myhero->get_position( ) + taric_e_end * 460;

		if ( taric_e_target && taric_e_target->is_ai_hero( ) && !taric_e_target->is_dead( ) && taric_e_target->is_valid( ) )
		{
			geometry::rectangle e_sector = geometry::rectangle(
				myhero->get_position( ),
				taric_e_460_pos,
				taric_e_target->get_bounding_radius( ) / 2 );
			auto poly = e_sector.to_polygon( );

			taric_target_pos = taric_e_target->get_path_controller( ) && taric_e_target->get_path_controller( )->is_dashing( ) ? taric_e_target->get_path_controller( )->get_end_vec( ) : prediction->get_prediction( taric_e_target, 0.1f ).get_unit_position( );

			auto e_115_distance = taric_target_pos.distance( taric_e_115_pos );
			auto e_230_distance = taric_target_pos.distance( taric_e_230_pos );
			auto e_345_distance = taric_target_pos.distance( taric_e_345_pos );
			auto e_460_distance = taric_target_pos.distance( taric_e_460_pos );

			float distance[4] = { e_115_distance, e_230_distance, e_345_distance, e_460_distance }; //Ð¡>´ó
			std::sort( distance, distance + 4 );

			if ( distance[0] == e_115_distance )
			{
				taric_magnet_pos = myhero->get_position( ) + ( taric_target_pos - ( taric_e_115_pos ) ).normalized( ) * e_115_distance;
			}
			else if ( distance[0] == e_230_distance )
			{
				taric_magnet_pos = myhero->get_position( ) + ( taric_target_pos - ( taric_e_230_pos ) ).normalized( ) * e_230_distance;
			}
			else if ( distance[0] == e_345_distance )
			{
				taric_magnet_pos = myhero->get_position( ) + ( taric_target_pos - ( taric_e_345_pos ) ).normalized( ) * e_345_distance;
			}
			else if ( distance[0] == e_460_distance )
			{
				taric_magnet_pos = myhero->get_position( ) + ( taric_target_pos - ( taric_e_460_pos ) ).normalized( ) * e_460_distance;
			}

			if ( taric_magnet_pos.is_valid( ) && gametime->get_time( ) < taric_last_e_time + 1.0f && !poly.is_inside( taric_target_pos ) )
			{
				bool use_combo = !hero::taric_e_magnet_combo_mode->get_bool( ) || orbwalker->combo_mode( );
				bool use_turret = !taric_magnet_pos.is_under_enemy_turret( ) || !hero::taric_e_tower->get_bool( );
				if ( use_turret && !taric_magnet_pos.is_wall( ) && use_combo )
				{
					myhero->issue_order( taric_magnet_pos, false, false );
					orbwalker->set_movement( false );
					orbwalker->set_attack( false );
					t_last_order = gametime->get_time( );
					taric_orb = true;
				}
			}
			else if ( taric_orb )
			{
				orbwalker->set_movement( true );
				orbwalker->set_attack( true );
				taric_orb = false;
			}
			if ( gametime->get_time( ) > taric_evade_time && gametime->get_time( ) > taric_last_e_time + 1.0f )
			{
				taric_use_evade = !evade->is_evade_disabled( );
			}
			if ( gametime->get_time( ) < taric_last_e_time + 1.0f && hero::taric_e_magnet_evade->get_bool())
			{
				evade->disable_evade( );
				taric_evade_check = true;
				taric_evade_time = gametime->get_time( ) + 0.5f;
			}
			else if ( taric_evade_check && hero::taric_e_magnet_evade->get_bool())
			{
				if ( taric_use_evade )
				{
					evade->enable_evade( );
				}
				taric_evade_check = false;
			}
		}
	}
#pragma endregion

#pragma region cassiopeia & tryndamere
	vector cassiopeia_start_pos, cassiopeia_end_pos;
	float cassiopeia_r_time;
	bool cassiopeia_orb;
	void cassiopeia_r_dodge_logic()
	{
		if (!hero::cassiopeia_r_dodge->get_bool()) return;
		if (gametime->get_time() > cassiopeia_r_time + 0.3f - ping->get_ping() / 1000.0f && cassiopeia_r_time + 0.5f > gametime->get_time())
		{
			constexpr static auto radius = 700.f;
			const auto perp = (cassiopeia_end_pos - cassiopeia_start_pos).normalized().perpendicular(); //´¹Ö±
			auto cassiopeia_r_left_pos = cassiopeia_end_pos + perp * (radius);
			auto cassiopeia_r_right_pos = cassiopeia_end_pos - perp * (radius);
			cassiopeia_r_left_pos = cassiopeia_r_left_pos.extend(cassiopeia_start_pos, 200.0f);
			cassiopeia_r_right_pos = cassiopeia_r_right_pos.extend(cassiopeia_start_pos, 200.0f);

			geometry::polygon q1_sector;

			q1_sector.add(cassiopeia_start_pos);
			q1_sector.add(cassiopeia_r_left_pos);
			q1_sector.add(cassiopeia_end_pos);
			q1_sector.add(cassiopeia_r_right_pos);

			auto pos = myhero->get_position().extend(cassiopeia_start_pos, -200);

			orbwalker->set_attack(false);
			orbwalker->set_movement(false);
			cassiopeia_orb = true;
			if (q1_sector.is_inside(myhero->get_position()))
			{
				myhero->issue_order(pos, false, false);
			}
		}
		else if (cassiopeia_orb)
		{
			orbwalker->set_attack(true);
			orbwalker->set_movement(true);
			cassiopeia_orb = false;
		}
	}

	//tryndamere dodge w
	float tryndamere_w_time;
	vector tryndamere_w_pos;
	bool tryndamere_orb;
	void tryndamere_w_dodge_logic()
	{
		if (!hero::cassiopeia_r_dodge->get_bool()) return;
		if (gametime->get_time() > tryndamere_w_time + 0.1f - ping->get_ping() / 1000.0f && tryndamere_w_time + 0.3f > gametime->get_time() && myhero->get_distance(tryndamere_w_pos) < 950)
		{
			orbwalker->set_attack(false);
			orbwalker->set_movement(false);
			tryndamere_orb = true;
			auto pos = myhero->get_position().extend(tryndamere_w_pos, 200);
			myhero->issue_order(pos, false, false);
		}
		else if (tryndamere_orb)
		{
			orbwalker->set_attack(true);
			orbwalker->set_movement(true);
			tryndamere_orb = false;
		}
	}
#pragma endregion

#pragma region xayah
	//xayah
	float xayah_use_e;
	struct xayah_e
	{
		game_object_script object;
		float start_time;
		float expire_time;

		xayah_e(game_object_script obj)
		{
			object = obj;
			start_time = gametime->get_time();
			expire_time = gametime->get_time() + 6.0f;
		}
	};
	std::vector<xayah_e> xayah_es;
	int count_xayah_e( )
	{
		auto count = 0;
		for ( xayah_e xayah_e : xayah_es )
		{
			if ( xayah_e.object && xayah_e.object->is_valid( ) && !xayah_e.object->is_dead( ) && xayah_e.object->get_emitter( ) &&
				xayah_e.object->get_emitter( )->is_valid( ) &&
				xayah_e.object->get_emitter( )->is_enemy( )
				)
			{
				if ( !xayah_e.object->get_position( ).is_valid( ) || xayah_e.object->get_emitter( )->get_position( ).is_valid( ) )
					continue;

				geometry::rectangle e_sector = geometry::rectangle(
					xayah_e.object->get_position( ), xayah_e.object->get_emitter( )->get_position( ), 80.0f );
				auto poly = e_sector.to_polygon( );
				if ( poly.is_inside( myhero->get_position( ) ) )
				{
					count++;
				}
			}
		}
		return count;
	}
	void draw_xayah( )
	{
		for ( xayah_e xayah_e : xayah_es )
		{
			if ( xayah_e.object && xayah_e.object->is_valid( ) && !xayah_e.object->is_dead( ) &&
				xayah_e.object->get_emitter( ) && xayah_e.object->get_emitter( )->is_valid( ) &&
				xayah_e.object->get_emitter( )->is_enemy( ) && hero::xayah_e->get_bool( ) ) //&& xayah_e.object->get_owner()->is_me()  
			{
				geometry::rectangle r_sector = geometry::rectangle(
					xayah_e.object->get_position( ), xayah_e.object->get_emitter( )->get_position( ), 80.0f );
				auto poly = r_sector.to_polygon( );
				auto color = poly.is_inside( myhero->get_position( ) ) ? hero::xayah_e_hit_color->get_color( ) : hero::xayah_e_color->get_int( );

				switch ( hero::xayah_e_style->get_int( ) + 1 )
				{
					default:
						break;

					case 1:
					{
						draw_manager->add_line( xayah_e.object->get_emitter( )->get_position( ), xayah_e.object->get_position( ), color, 2.0f );
						draw_manager->add_circle( xayah_e.object->get_position( ), 50, color, 2.f );
						break;
					}

					case 2:
					{
						auto&& point_1 = poly.points.at( 0 );
						auto&& point_2 = poly.points.at( 1 );
						auto&& point_3 = poly.points.at( 2 );
						auto&& point_4 = poly.points.at( 3 );

						draw_manager->add_line( point_1, point_2, color, 2.0f );
						draw_manager->add_line( point_2, point_3, color, 2.0f );
						draw_manager->add_line( point_3, point_4, color, 2.0f );
						draw_manager->add_line( point_4, point_1, color, 2.0f );
						break;
					}

					case 3:
						draw_manager->add_rectangle_3d( xayah_e.object->get_emitter( )->get_position( ), xayah_e.object->get_position( ), color, 90 * 2.f, 2.f, 90, .33f );
						break;
				}
			}
		}
	}
#pragma endregion

#pragma region thresh
	struct lantern
	{
		game_object_script object;
		int lantern_id;
		float start_time;
		float expire_time;

		lantern(game_object_script obj, int id)
		{
			object = obj;
			lantern_id = id;
			start_time = gametime->get_time();
			expire_time = gametime->get_time() + 6.0f;
		}
	};
	std::vector<lantern> lanterns;
	void use_lantern()
	{
		for (lantern lantern : lanterns)
		{
			if ( !lantern.object || !lantern.object->is_valid( ) || lantern.object->is_dead( ) )
				continue;

			if (myhero->get_distance(lantern.object) > 1000) 
				continue;

			if ( lantern.object->get_owner( ) || !lantern.object->get_owner( )->is_valid( ) )

			if (lantern.object->get_owner()->is_ally() && !lantern.object->get_owner()->is_me() && myhero->get_distance(lantern.object) < hero::use_lantern_range->get_int() + myhero->get_bounding_radius() && 
				hero::use_lantern->get_bool())
			{
				myhero->use_object(lantern.object);
			}
			if (lantern.object->get_owner()->is_enemy() && lantern.object->is_valid() && !lantern.object->is_dead() && A_::can_ward() && lantern.object->get_distance(myhero) < 600)
			{
				if (lantern.start_time + 0.1f > gametime->get_time()) 
					continue;

				auto ward_obj = A_::get_nearby_ward(lantern.object->get_position(), 50);
				if (ward_obj) 
					continue;

				if (hero::ward_lantern->get_int() == 1)
				{
					A_::cast_ward(lantern.object->get_position());
					continue;
				}
				else if (hero::ward_lantern->get_int() == 0)
				{
					auto make_lanter = lantern.object->get_owner();
					if (make_lanter->get_distance(lantern.object) > 300 && lantern.object->count_enemies_in_range(299) > 0)
					{
						A_::cast_ward( lantern.object->get_position( ) );
						continue;
					}
				}

			}
		}
	}
	void draw_lantern()
	{
		for (lantern lantern : lanterns)
		{
			if ( !lantern.object || !lantern.object->is_valid( ) || lantern.object->is_dead( ) || !lantern.object->get_owner( ) || !lantern.object->get_owner( )->is_valid( ) )
				continue;

			if ( gametime->get_time() < lantern.expire_time && myhero->get_distance(lantern.object) < 1000)
			{
				draw_manager->add_circle(lantern.object->get_position(), static_cast< float >( hero::use_lantern_range->get_int() ), MAKE_COLOR(0, 255, 100, 255), 1.f);
				auto pos = lantern.object->get_position().extend(myhero->get_position(), static_cast< float >( hero::use_lantern_range->get_int( )));
				draw_manager->add_line(myhero->get_position(), pos, MAKE_COLOR(0, 255, 100, 255), 1.0f);
			}
		}
	}
#pragma endregion

#pragma region katarina
	struct kata_p
	{
		game_object_script object;
		float expire_time;
		kata_p(game_object_script obj, float t)
		{
			object = obj;
			expire_time = t;
		}
	};
	std::vector<kata_p> kata_ps;
	void draw_kata_p()
	{
		if (!hero::katarina_p->get_bool()) 
			return;

		for (kata_p kata_p : kata_ps)
		{
			if ( !kata_p.object || !kata_p.object->is_valid( ) || kata_p.object->is_dead( ) )
				continue;

			if ( !kata_p.object->get_emitter( ) || !kata_p.object->get_emitter( )->is_valid( ) )
				continue;

			if ( kata_p.object->get_emitter()->is_enemy() ) 
			{
				auto pos = kata_p.object->get_position();
				auto color = myhero->get_distance(kata_p.object) < 340 + 140 ? hero::katarina_p_hit_color->get_color( ) : hero::katarina_p_color->get_color( );
				draw_manager->add_circle(pos, 340 + 140, color, 2.f);
				auto t = 360 / 5.3; //9.2
				auto angle_0 = 180 + (kata_p.expire_time - gametime->get_time()) * t;
				A_::draw_arc(pos, 140 + 10.f, 180, static_cast< int >( angle_0 ), MAKE_COLOR( 9, 132, 227, 255 ), 5.f );
				//myhero->print_chat( 0x3, "%.1f", angle_0 );
			}
		}
	}
#pragma endregion

#pragma region kayn 
//hero::kayn_e
	void kayn_e( )
	{
		if ( !hero::kayn_e->get_bool( ) || hero::kayn_e_nid == 0 )
			return;

		auto kayn_e_particle = entitylist->get_object_by_network_id( hero::kayn_e_nid );
		if ( !kayn_e_particle || !kayn_e_particle->is_valid( ) || !kayn_e_particle->get_position( ).is_valid( ) )
			return;

		auto w2s = vector::zero;
		renderer->world_to_screen( kayn_e_particle->get_position( ), w2s );

		if ( !w2s.is_valid( ) )
			return;

		auto size = myhero->get_bounding_radius( ) * 0.75f;
		draw_manager->add_image( hero::kayn_e_texture, w2s - vector( size/2.f, size/2.f ), vector( size, size ) );
	}
#pragma endregion

#pragma region yasuo
	void yasuo_e()
	{
		if (!hero::yasuo_e->get_bool()) return;
		auto lane_minions = entitylist->get_ally_minions();

		lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
			{
				return !x || !x->is_valid() || x->is_dead( ) || !x->is_visible_on_screen( ) || !x->has_buff( buff_hash( "YasuoE" ) );
			}), lane_minions.end());


		for (auto&& minion : lane_minions)
		{
			auto pos = vector(minion->get_position().x, minion->get_position().y, minion->get_position().z + 30);
			auto buff = minion->get_buff_time_left(buff_hash("YasuoE"));
			auto radius = minion->get_bounding_radius();
			//draw_manager->add_filled_circle(minion->get_position(), radius*1.8, MAKE_COLOR(255, 255, 255, 75));
			auto t = 360 / 10; //9.2
			auto angle_0 = 180 + (buff)*t;
			//auto angle_1 = 540;
			A_::draw_arc(pos, radius * 1.8f, 180, static_cast<int>( angle_0 ), MAKE_COLOR(255, 255, 255, 255), 2.f);
		}

	}
#pragma endregion
#define Xayah_Base_Passive_Dagger_Mark8s (1350590226)
#define Xayah_Base_Passive_Dagger_indicator_enemy8s (1995665761)

#define Kayn_Base_Primary_E_Warning (871182340)

#define Katarina_Skin10_W_Indicator_Ally (392220005)
#define Katarina_Base_W_Indicator_Enemy ( 4033407937 )

	void on_create_object(game_object_script sender)
	{
		if ( !sender || !sender->is_valid( ) )
			return;

		if ( sender->is_general_particle_emitter( ) && sender->get_emitter( ) && sender->get_emitter( )->is_valid( ) )
		{
			switch ( sender->get_emitter_resources_hash( ) )
			{
				default:
					break;

				case Kayn_Base_Primary_E_Warning:
					if ( sender->get_emitter( )->is_enemy( ) )
						hero::kayn_e_nid = sender->get_network_id( );
					break;

				case Xayah_Base_Passive_Dagger_Mark8s:
				case Xayah_Base_Passive_Dagger_indicator_enemy8s:
					xayah_es.push_back( xayah_e( sender ) );
					break;

				case Katarina_Skin10_W_Indicator_Ally:
				case Katarina_Base_W_Indicator_Enemy:
					kata_ps.push_back( kata_p( sender, gametime->get_time( ) + 5.3f ) );
					break;
			}
		}

		if ( buff_hash_real( sender->get_name_cstr( ) ) == buff_hash( "ThreshLantern" ) ) 
		{
			lanterns.push_back(lantern(sender, static_cast< int >( lanterns.size() + 1)));
		}
	}

	void on_delete_object(game_object_script sender)
	{
		if ( !sender || !sender->is_valid( ) )
			return;

		if ( sender->is_general_particle_emitter( ) && sender->get_emitter( ) && sender->get_emitter( )->is_valid( ) )
		{
			switch ( sender->get_emitter_resources_hash( ) )
			{
				default:
					break;

				case Xayah_Base_Passive_Dagger_Mark8s:
				case Xayah_Base_Passive_Dagger_indicator_enemy8s:
				{
					xayah_es.erase( std::remove_if( xayah_es.begin( ), xayah_es.end( ), [ sender ]( xayah_e x )
					{
						return !x.object || !x.object->is_valid( ) || x.object->get_network_id( ) == sender->get_network_id( );
					} ), xayah_es.end( ) );
					break;
				}

				case Katarina_Skin10_W_Indicator_Ally:
				case Katarina_Base_W_Indicator_Enemy:
				{
					kata_ps.erase( std::remove_if( kata_ps.begin( ), kata_ps.end( ), [ sender ]( kata_p x )
					{
						return !x.object || !x.object->is_valid( ) || x.object->get_network_id( ) == sender->get_network_id( );
					} ), kata_ps.end( ) );

					break;
				}
			}
		}

		if ( buff_hash_real( sender->get_name_cstr( ) ) == buff_hash( "ThreshLantern" ) )
		{
			lanterns.erase(std::remove_if(lanterns.begin(), lanterns.end(), [sender](lantern x)
				{
					return !x.object || !x.object->is_valid( ) || x.object->get_network_id() == sender->get_network_id();
				}), lanterns.end());
		}
	}

	void OnProcessSpellCast(game_object_script sender, spell_instance_script spell)
	{
		if ( !spell || !spell->get_spell_data( ) || !sender )
			return;

		auto spell_hash = spell->get_spell_data()->get_name_hash();
		if (sender->is_ai_hero() )
		{
			if ( sender->is_enemy( ) )
			{
				switch ( spell_hash )
				{
					default:
						break;

					case spell_hash( "XayahE" ):
						xayah_use_e = gametime->get_time( ) + 0.5f - ping->get_ping( ) / 1000.0f;
						break;

					case spell_hash( "CassiopeiaR" ):
						cassiopeia_r_time = gametime->get_time( );
						cassiopeia_start_pos = sender->get_position( );
						cassiopeia_end_pos = sender->get_position( ) + ( spell->get_end_position( ) - sender->get_position( ) ).normalized( ) * 820;
						break;

					case spell_hash( "TryndamereW" ):
						tryndamere_w_time = gametime->get_time( );
						tryndamere_w_pos = sender->get_position( );
						break;
				}
			}
			else
			{
				switch ( spell_hash )
				{
					default:
						break;

					case spell_hash( "TaricE" ):
					{
						if ( myhero->get_distance( sender ) < 800 )
						{
							taric_last_e_time = gametime->get_time( );
							taric_e_end = ( spell->get_end_position( ) - myhero->get_position( ) ).normalized( );
						}
						break;
					}
				}
			}
		}
	}

	void all_new_draw()
	{
		if (myhero->is_dead()) 
			return;

		draw_xayah();
		draw_lantern();
		yasuo_e();
		kayn_e( );
		draw_kata_p();
	}

	void all_logic()
	{
		update_menu();
		taric_e_logic();
		cassiopeia_r_dodge_logic();
		tryndamere_w_dodge_logic();
		use_lantern();
		event_handler< events::on_create_object >::add_callback(on_create_object);
		event_handler< events::on_delete_object >::add_callback(on_delete_object);
		event_handler< events::on_process_spell_cast >::add_callback(OnProcessSpellCast);
	}
	void all_unload()
	{
		event_handler< events::on_create_object >::remove_handler(on_create_object);
		event_handler< events::on_delete_object >::remove_handler(on_delete_object);
		event_handler<events::on_process_spell_cast>::remove_handler(OnProcessSpellCast);
	}
}