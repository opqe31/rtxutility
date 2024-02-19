#include "main_hud.hpp"
#include "A_.h"

namespace spell_tracker
{
	namespace cfg
	{
#pragma region turret
		TreeEntry* turret_range_enabled = nullptr;
		TreeEntry* turret_range_enabled_ally = nullptr;
		TreeEntry* turret_allyt_color;

		TreeEntry* turret_range = nullptr;
		TreeEntry* turret_range_ally = nullptr;
		TreeEntry* turret_hit_color = nullptr;
		TreeEntry* turret_no_hit_color = nullptr;
		TreeEntry* turret_hit_other_color = nullptr;
		TreeEntry* turret_line_size = nullptr;
		TreeEntry* turret_animations = nullptr;

		TreeEntry* inside_glow_size;
		TreeEntry* inside_glow_power;
		TreeEntry* outside_glow_size;
		TreeEntry* outside_glow_power;

		TreeEntry* plates_state_ally;
		TreeEntry* plates_state_enemy;

		TreeEntry* plates_color_ally;
		TreeEntry* plates_color_enemy;
#pragma endregion
		
#pragma region inhibitor
		TreeEntry* ally_inhib;
		TreeEntry* enemy_inhib;

		TreeEntry* inhib_thickness;
		TreeEntry* ally_inhib_color;
		TreeEntry* enemy_inhib_color;
#pragma endregion

		bool* turret_active;
		bool* inhibitor_active;
	}

	namespace structures
	{
		auto load_menu( TreeTab* show_inhibitor, TreeTab* show_turret ) -> void
		{	
			{
				float color_plates [ 4 ] = { 250.f / 255.f, 177.f / 255.f, 160.f / 255.f, 1.f };
				show_turret->add_separator( "show_setting", " - - Show turret - - " );
				cfg::turret_range_enabled = show_turret->add_checkbox( "turret_range_enabled", "Show turret range", true, true );
				cfg::turret_range_enabled_ally = show_turret->add_checkbox( "turret_range_enabled_ally", "Show ally turret range", true, true );

				cfg::turret_range = show_turret->add_slider( "turret_range", "Enemy turret distance", 1500, 1500, 3000 );
				cfg::turret_range_ally = show_turret->add_slider( "turret_range_ally", "Ally turret distance", 1500, 1500, 3000 );

				show_turret->add_separator( "show_setting_other", " - - Plates - - " );
				cfg::plates_state_ally = show_turret->add_checkbox( "plates_state_ally", "Plates remaining time ally", true );
				cfg::plates_state_enemy = show_turret->add_checkbox( "plates_state_enemy", "Plates remaining time enemy", true );

				show_turret->add_separator( "show_setting_common", " - - Customizations - - " );
				cfg::turret_line_size = show_turret->add_slider( "turret_line_size", "Thickness", 3, 1, 20 );
				cfg::turret_animations = show_turret->add_combobox( "turret_animations_mode", "Animations", { { "Disabled", nullptr }, { "Alpha", nullptr }, { "Range", nullptr } }, 1 );

				show_turret->add_separator( "show_setting_colors", " - - Colors ally - - " );
				float color9 [ 4 ] = { 52.f / 255.f, 152.f / 255.f, 219 / 255.f, 1.f };
				cfg::turret_allyt_color = show_turret->add_colorpick( "turret_allyt_color0", "Ally turret range", color9 );
				cfg::plates_color_ally = show_turret->add_colorpick( "plates_color_ally", "Ally plates color", color_plates );

				show_turret->add_separator( "show_setting_colors_2", " - - Colors enemy - - " );
				float color1 [ 4 ] = { 255.f / 255.f, 121.f / 255.f, 121.f / 255.f, 1.f };
				cfg::turret_hit_color = show_turret->add_colorpick( "turret_hit_color0", "Turret hit me range", color1 );
				float color3 [ 4 ] = { 255.f / 255.f, 190.f / 255.f, 118.f / 255.f, 1.f };
				cfg::turret_no_hit_color = show_turret->add_colorpick( "turret_no_hit_color0", "Turret AFK range", color3 );
				float color5 [ 4 ] = { 22.f / 255.f, 160.f / 255.f, 133.f / 255.f, 1.f };
				cfg::turret_hit_other_color = show_turret->add_colorpick( "turret_hit_other_color0", "Turret hit other range", color5 );
				cfg::plates_color_enemy = show_turret->add_colorpick( "plates_color_enemy", "Enemy plates color", color_plates );

				show_turret->add_separator( "show_setting_colors_glow", " - - Glow - - " );
				//glow_data( 0.f, 0.f, 05.f, 0.5f )
				cfg::inside_glow_size = show_turret->add_slider( "inside_glow_size", "Inside glow size", 50, 0, 100 );
				cfg::inside_glow_power = show_turret->add_slider( "inside_glow_power", "Inside glow power", 50, 0, 100 );
				cfg::outside_glow_size = show_turret->add_slider( "outside_glow_size", "Outside glow size", 53, 0, 100 );
				cfg::outside_glow_power = show_turret->add_slider( "outside_glow_power", "Outside glow power", 70, 0, 100 );

#pragma region turret_callbacks
				for ( auto&& x : entitylist->get_ally_turrets( ) )
				{
					if ( x && x->is_valid( ) && x->get_square_icon_portrait( ) )
					{
						cfg::turret_range_enabled_ally->set_texture( x->get_square_icon_portrait( ) );
						break;
					}
				}

				for ( auto&& x : entitylist->get_enemy_turrets( ) )
				{
					if ( x && x->is_valid( ) && x->get_square_icon_portrait( ) )
					{
						cfg::turret_range_enabled->set_texture( x->get_square_icon_portrait( ) );
						break;
					}
				}

				cfg::turret_active = new bool( false );
				show_turret->set_assigned_active( cfg::turret_active );

				for ( auto&& x : entitylist->get_enemy_turrets( ) )
				{
					if ( x && x->get_square_icon_portrait( ) )
					{
						show_turret->set_assigned_texture( x->get_square_icon_portrait( ) );
						break;
					}
				}
				const auto on_turret_enabled_click = [ ]( TreeEntry* self )
				{
					TreeEntry* turret_range = nullptr;
					auto visuals_entry = menu->get_tab( "visuals" );
					if ( visuals_entry )
					{
						auto game_entry = visuals_entry->get_tab( "game" );
						if ( game_entry )
							turret_range = game_entry->get_entry( "turret_range" );
					}

					if ( turret_range )
						turret_range->set_bool( !self->get_bool( ) );

					cfg::turret_range->is_hidden( ) =
						cfg::turret_hit_color->is_hidden( ) =
						cfg::turret_no_hit_color->is_hidden( ) =
						cfg::turret_no_hit_color->is_hidden( ) =
						cfg::turret_hit_other_color->is_hidden( ) =
						cfg::turret_line_size->is_hidden( ) = !self->get_bool( );

					*cfg::turret_active = self->get_bool( ) || cfg::turret_range_enabled_ally->get_bool( );
				};
				cfg::turret_range_enabled->add_property_change_callback( on_turret_enabled_click );
				on_turret_enabled_click( cfg::turret_range_enabled );
				cfg::turret_range_enabled_ally->add_property_change_callback( [ ]( TreeEntry* self ) {
					*cfg::turret_active = self->get_bool( ) || cfg::turret_range_enabled->get_bool( );
				} );
#pragma endregion turret_callbacks
			}

			{
				float color_respawn [ 4 ] = { 250.f / 255.f, 177.f / 255.f, 160.f / 255.f, 1.f };

				show_inhibitor->add_separator( "show_setting", " - - Show inhibitor - - " );
				cfg::ally_inhib = show_inhibitor->add_checkbox( "ally_inhib", "Show ally inhibitor respawn time", true );
				cfg::enemy_inhib = show_inhibitor->add_checkbox( "enemy_inhib", "Show enemy inhibitor respawn time", true );

				show_inhibitor->add_separator( "show_setting_common", " - - Common - - " );
				cfg::inhib_thickness = show_inhibitor->add_slider( "inhib_thickness", "Thickness", 6, 1, 20 );
				show_inhibitor->add_separator( "show_setting_colors", " - - Colors - - " );
				cfg::ally_inhib_color = show_inhibitor->add_colorpick( "ally_inhib_color", "Ally color", color_respawn );
				cfg::enemy_inhib_color = show_inhibitor->add_colorpick( "enemy_inhib_color", "Enemy color", color_respawn );

#pragma region cb
				cfg::inhibitor_active = new bool( false );

				const auto& inhib_click = [ ]( TreeEntry* ) {
					*cfg::inhibitor_active = cfg::ally_inhib->get_bool( ) || cfg::enemy_inhib->get_bool( );
				};

				inhib_click( nullptr );
				cfg::ally_inhib->add_property_change_callback( inhib_click );
				cfg::enemy_inhib->add_property_change_callback( inhib_click );
				show_inhibitor->set_assigned_active( cfg::inhibitor_active );
#pragma endregion
			}
		}

#pragma region inhibitor
		struct s_inhibitor
		{
			uint32_t network_id = 0;

			float t_respawn = -1.f;
		};
		std::map< uint32_t, s_inhibitor> m_inhibitors = { };

		auto get_inhib_respawn_t( ) -> float
		{
			return 300.f;
		}

		auto on_inhibitor_death( uint32_t sender ) -> void
		{
			if ( m_inhibitors.find( sender ) == m_inhibitors.end( ) )
			{
				m_inhibitors [ sender ] = s_inhibitor {

					( uint32_t )sender,
					gametime->get_time( ) + get_inhib_respawn_t( )
				};
			}
			else
				m_inhibitors [ sender ].t_respawn = gametime->get_time( ) + get_inhib_respawn_t( );
		}

		auto show_inhibitor( ) -> void
		{
			for ( auto&& x : entitylist->get_all_inhibitors( ) )
			{
				if ( !x || !x->is_valid( ) || x->get_health( ) > 0 )
					continue;

				if ( x->is_ally( ) && !cfg::ally_inhib->get_bool( ) )
					continue;

				if ( x->is_enemy( ) && !cfg::enemy_inhib->get_bool( ) )
					continue;

				if ( m_inhibitors.find( x->get_network_id( ) ) == m_inhibitors.end( ) )
					continue;

				auto m_inhib = m_inhibitors [ x->get_network_id( ) ];
				if ( m_inhib.t_respawn < gametime->get_time( ) )
					continue;

				auto remaining_time = m_inhib.t_respawn - gametime->get_time( );
				auto pcent = std::clamp( 1.f - ( remaining_time / get_inhib_respawn_t( ) ), 0.f, 1.f );

				/*
					cfg::inhib_thickness = show_inhibitor->add_slider( "inhib_thickness", "Thickness", 6, 1, 20 );
					show_inhibitor->add_separator( "show_setting_colors", " - - Colors - - " );
					cfg::ally_inhib_color = show_inhibitor->add_colorpick( "ally_inhib_color", "Ally color", color_respawn );
					cfg::enemy_inhib_color = show_inhibitor->add_colorpick( "enemy_inhib_color", "Enemy color", color_respawn );
				*/

				//auto color_background = remaining_time > 30.f ? MAKE_COLOR( 53, 59, 72, 220 ) : A_::animate_color( MAKE_COLOR( 53, 59, 72, 220 ), 60.f, 220.f, 4.f, 2.f );
				auto color_base = x->is_ally( ) ? cfg::ally_inhib_color->get_color( ) : cfg::enemy_inhib_color->get_color( );
				auto color_main = remaining_time > 30.f ? color_base : A_::animate_color( color_base, 120.f, 255.f, 4.f, 2.f );

				//draw_manager->add_circle( turret->get_position( ), turret->get_bounding_radius( ) * 2.f * m_turret_animation [ turret->get_network_id( ) ].fl_animation_frame,
				//	color_background, cfg::turret_line_size->get_int( ) * 2 );

				A_::draw_gradient_arc( x->get_position( ), x->get_bounding_radius( ) /* * m_turret_animation [ turret->get_network_id( ) ].fl_animation_frame*/, 0.f, 360.f * pcent,
					color_main, color_main, static_cast< float >( cfg::inhib_thickness->get_int( ) ), vector( 0.0001f, 0.0001f, 0.0001f ) );
			}
		}
#pragma endregion

#pragma region turret
		struct s_turret_animation
		{
			float t_last_animation = -8.f;

			float fl_animation_frame = 0.001f;

			float t_attack_finish = -8.f;
			bool last_target_me;
		};

		auto show_turret( ) -> void
		{
			static auto turret_animation = std::map<uint32_t, s_turret_animation> { };
			auto _glow = glow_data( cfg::inside_glow_size->get_int( ) / 100.f, cfg::inside_glow_power->get_int( ) / 100.f,
				cfg::outside_glow_size->get_int( ) / 100.f, cfg::outside_glow_power->get_int( ) / 100.f );

			if ( cfg::turret_range_enabled_ally->get_bool( ) )
			{
				for ( auto&& turret : entitylist->get_ally_turrets( ) )
				{
					if ( !turret || !turret->is_valid( ) || turret->is_dead( ) )
						continue;

					auto&& it_anim = turret_animation.find( turret->get_network_id( ) );
					if ( it_anim == turret_animation.end( ) )
					{
						turret_animation [ turret->get_network_id( ) ] = s_turret_animation { };
						continue;
					}

					if ( !A_::is_on_screen( turret->get_position( ) ) || !turret->is_visible( ) || myhero->get_distance( turret ) > cfg::turret_range_ally->get_int( ) )
					{
						static auto easing_fn = getEasingFunction( easing_functions::EaseOutSine );

						if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.033f )
						{
							it_anim->second.fl_animation_frame =
								std::clamp( easing_fn( it_anim->second.fl_animation_frame ), 0.001f, 0.999f );
							it_anim->second.t_last_animation = gametime->get_time( );
						}

						if ( it_anim->second.fl_animation_frame <= 0.01f )
							continue;
					}
					else
					{
						static auto easing_fn = getEasingFunction( easing_functions::EaseInSine );

						if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.033f )
						{
							it_anim->second.fl_animation_frame =
								std::clamp( easing_fn( it_anim->second.fl_animation_frame ), 0.001f, 0.999f );
							it_anim->second.t_last_animation = gametime->get_time( );
						}
					}

					auto range = ( turret->get_name( ).find( "Shrine" ) != std::string::npos ? 1250.f + myhero->get_bounding_radius( ) : 750.f + turret->get_bounding_radius( ) );
					if ( cfg::turret_animations->get_int( ) == 2 )
						range *= it_anim->second.fl_animation_frame;

					if ( range <= turret->get_bounding_radius( ) / 4.f )
						continue;

#pragma region plates
					if ( turret->has_item( ItemId::Turret_Plating ) != spellslot::invalid )
					{
						if ( cfg::plates_state_ally->get_bool( ) )
						{
							auto remaining_time = 14.f * 60.f - gametime->get_time( );
							auto pcent = std::clamp( remaining_time / ( 14.f * 60.f ), 0.f, 1.f );

							auto plates_ally_clr = cfg::plates_color_ally->get_color( );
							auto fl_frame = 1.f;

							if ( cfg::turret_animations->get_int( ) == 1 )
								plates_ally_clr = A_::set_alpha( plates_ally_clr, static_cast< unsigned long >( A_::get_alpha( plates_ally_clr ) * it_anim->second.fl_animation_frame ) );
							else if ( cfg::turret_animations->get_int( ) == 2 )
								fl_frame = it_anim->second.fl_animation_frame;

							auto color_main = remaining_time > 30.f ? plates_ally_clr : A_::animate_color( plates_ally_clr, 120.f, 255.f, 4.f, 2.f );

							A_::draw_gradient_arc( turret->get_position( ), turret->get_bounding_radius( ) * 2.f * fl_frame, 0.f, 360.f * pcent,
								color_main, color_main, static_cast< float >( cfg::turret_line_size->get_int( ) * 2 ), turret->get_direction( ) );
						}
					}
#pragma endregion
				//draw_manager->add_circle( turret->get_position( ), range, cfg::turret_allyt_color->get_color( ), cfg::turret_line_size->get_int( ) );
				//draw_manager->add_filled_circle( turret->get_position( ), range, cfg::turret_allyt_color->get_color( ) );

					auto color = cfg::turret_allyt_color->get_color( );
					if ( cfg::turret_animations->get_int( ) == 1 )
						color = A_::set_alpha( color, static_cast< unsigned long >( it_anim->second.fl_animation_frame * A_::get_alpha( cfg::turret_allyt_color->get_color( ) ) ) );

					draw_manager->add_circle_with_glow( turret->get_position( ), color, range, static_cast< float >( cfg::turret_line_size->get_int( ) ), _glow );
				}
			}

			if ( cfg::turret_range_enabled->get_bool( ) )
			{
				for ( auto&& turret : entitylist->get_enemy_turrets( ) )
				{
					if ( !turret || !turret->is_valid( ) || turret->is_dead( ) )
						continue;

					auto it_anim = turret_animation.find( turret->get_network_id( ) );
					if ( it_anim == turret_animation.end( ) )
					{
						turret_animation [ turret->get_network_id( ) ] = s_turret_animation { };
						continue;
					}

					if ( !A_::is_on_screen( turret->get_position( ) ) || !turret->is_visible( ) || myhero->get_distance( turret ) > cfg::turret_range->get_int( ) )
					{
						static auto easing_fn = getEasingFunction( easing_functions::EaseOutSine );

						if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.02f )
						{
							it_anim->second.fl_animation_frame =
								std::clamp( easing_fn( it_anim->second.fl_animation_frame ), 0.001f, 0.999f );
							it_anim->second.t_last_animation = gametime->get_time( );
						}

						if ( it_anim->second.fl_animation_frame <= 0.01f )
							continue;
					}
					else
					{
						static auto easing_fn = getEasingFunction( easing_functions::EaseInSine );

						if ( gametime->get_time( ) - it_anim->second.t_last_animation >= 0.02f )
						{
							it_anim->second.fl_animation_frame = std::clamp( easing_fn( it_anim->second.fl_animation_frame ), 0.001f, 0.999f );
							it_anim->second.t_last_animation = gametime->get_time( );
						}
					}

					auto range = ( turret->get_name( ).find( "Shrine" ) != std::string::npos ?
						1250.f + myhero->get_bounding_radius( ) :
						750.f + myhero->get_bounding_radius( ) + turret->get_bounding_radius( ) );

					if ( cfg::turret_animations->get_int( ) == 2 )
						range *= it_anim->second.fl_animation_frame;

					if ( range <= turret->get_bounding_radius( ) / 4.f )
						continue;

#pragma region plates
					if ( turret->has_item( ItemId::Turret_Plating ) != spellslot::invalid )
					{
						if ( cfg::plates_state_enemy->get_bool( ) )
						{
							auto remaining_time = 14.f * 60.f - gametime->get_time( );
							auto pcent = std::clamp( remaining_time / ( 14.f * 60.f ), 0.f, 1.f );

							auto clr_plates_enemy = cfg::plates_color_enemy->get_color( );
							auto fl_anim = 1.f;
							if ( cfg::turret_animations->get_int( ) == 1 )
								clr_plates_enemy = A_::set_alpha( clr_plates_enemy, static_cast< unsigned long >( A_::get_alpha( clr_plates_enemy ) * it_anim->second.fl_animation_frame ) );
							else if ( cfg::turret_animations->get_int( ) == 2 )
								fl_anim = it_anim->second.fl_animation_frame;


							auto color_main = remaining_time > 30.f ? clr_plates_enemy : A_::animate_color( clr_plates_enemy, 120.f, 255.f, 4.f, 2.f );

							//draw_manager->add_circle( turret->get_position( ), turret->get_bounding_radius( ) * 2.f * it_anim->second.fl_animation_frame,
							//	color_background, cfg::turret_line_size->get_int( ) * 2 );

							A_::draw_gradient_arc( turret->get_position( ), turret->get_bounding_radius( ) * 2.f * fl_anim, 0.f, 360.f * pcent,
								color_main, color_main, static_cast< float >( cfg::turret_line_size->get_int( ) * 2 ), turret->get_direction( ) );
						}
					}
#pragma endregion

					auto attack = turret->get_active_spell( );
					if ( attack )
					{
						it_anim->second.t_attack_finish = attack->cast_start_time( ) + attack->get_attack_delay( ) + 0.05f;
						it_anim->second.last_target_me = attack->get_last_target_id( ) == myhero->get_id( );
					}

					auto is_winding_up = !( gametime->get_time( ) - it_anim->second.t_attack_finish >= 0.f );

					if ( attack && attack->get_spell_data( )->get_name_hash( ) == 89712619 && !is_winding_up )
					{
						if ( attack->get_last_target_id( ) != myhero->get_id( ) )
						{
							auto color = cfg::turret_hit_other_color->get_color( );
							if ( cfg::turret_animations->get_int( ) == 1 )
								color = A_::set_alpha( color, static_cast< unsigned long >( it_anim->second.fl_animation_frame * A_::get_alpha( color ) ) );

							draw_manager->add_circle_with_glow( turret->get_position( ), color, range,
								static_cast< float >( cfg::turret_line_size->get_int( ) ), _glow );
						}
						else
						{
							auto color = cfg::turret_hit_color->get_color( );
							if ( cfg::turret_animations->get_int( ) == 1 )
								color = A_::set_alpha( color, static_cast< unsigned long >( it_anim->second.fl_animation_frame * A_::get_alpha( color ) ) );

							draw_manager->add_circle_with_glow( turret->get_position( ), color, range,
								static_cast< float >( cfg::turret_line_size->get_int( ) ), _glow );
						}
					}
					if ( !is_winding_up && !attack )
					{
						auto color = cfg::turret_no_hit_color->get_color( );

						if ( cfg::turret_animations->get_int( ) == 1 )
							color = A_::set_alpha( color, static_cast< unsigned long >( it_anim->second.fl_animation_frame * A_::get_alpha( color ) ) );

						draw_manager->add_circle_with_glow( turret->get_position( ), color, range,
							static_cast< float >( cfg::turret_line_size->get_int( ) ), _glow );
					}
					else if ( is_winding_up )
					{
						if ( myhero->get_distance( turret ) <= range )
						{
							if ( it_anim->second.last_target_me )
							{
								auto color = cfg::turret_hit_color->get_color( );

								if ( cfg::turret_animations->get_int( ) == 1 )
									color = A_::set_alpha( color, static_cast< unsigned long >( it_anim->second.fl_animation_frame * A_::get_alpha( color ) ) );

								draw_manager->add_circle_with_glow( turret->get_position( ), color, range,
									static_cast< float >( cfg::turret_line_size->get_int( ) ), _glow );
							}
							else
							{
								auto color = cfg::turret_hit_other_color->get_color( );

								if ( cfg::turret_animations->get_int( ) == 1 )
									color = A_::set_alpha( color, static_cast< unsigned long >( it_anim->second.fl_animation_frame * A_::get_alpha( color ) ) );

								draw_manager->add_circle_with_glow( turret->get_position( ), color, range,
									static_cast< float >( cfg::turret_line_size->get_int( ) ), _glow );
							}
						}
						else
						{
							auto color = cfg::turret_hit_other_color->get_color( );

							if ( cfg::turret_animations->get_int( ) == 1 )
								color = A_::set_alpha( color, static_cast< unsigned long >( it_anim->second.fl_animation_frame * A_::get_alpha( color ) ) );

							draw_manager->add_circle_with_glow( turret->get_position( ), color, range,
								static_cast< float >( cfg::turret_line_size->get_int( ) ), _glow );
						}
					}
				}
			}
		}
#pragma endregion

		auto unload( ) -> void
		{
			delete cfg::turret_active; delete cfg::inhibitor_active;
		}
	}
}