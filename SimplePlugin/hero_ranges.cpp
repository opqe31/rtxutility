#include "main_hud.hpp"
#include "A_.h"
#include "cooldown_manager.hpp"

namespace spell_tracker
{
	namespace hero_ranges
	{
		namespace cfg
		{
			const std::vector<std::string> m_aa_entries = { "enemy", "ally", "me" };
			std::map<spellslot, std::string> m_spells =
			{
				{ spellslot::q, "q" },
				{ spellslot::w, "w" },
				{ spellslot::e, "e" },
				{ spellslot::r, "r" }
			};

			float default_rng_clr_2 [ 4 ] = { 55.f / 255.f, 66.f / 255.f, 250.f / 255.f, 1.f };
			float default_rng_clr_1 [ 4 ] = { 181.f / 255.f, 52.f / 255.f, 113.f / 255.f, 1.f };

			TreeEntry* draw_ranges;
			TreeEntry* in_depth_ranges;

			struct s_override_spell
			{
				TreeEntry* enabled;

				TreeEntry* glow_type;

				TreeEntry* thickness;
				TreeEntry* glow[ 4 ];

				TreeEntry* color_1;
				TreeEntry* color_2;

				TreeEntry* arc_angle;

				auto get_glow_data( ) -> glow_data
				{
					return glow_data
					{
						glow [ 0 ]->get_int( ) / 100.f,
						glow [ 1 ]->get_int( ) / 100.f,
						glow [ 2 ]->get_int( ) / 100.f,
						glow [ 3 ]->get_int( ) / 100.f
					};
				}
			};

			namespace aa
			{
				TreeEntry* enabled;
				std::map<uint32_t, s_override_spell> m_config = { };
			}

			namespace spell
			{
				TreeEntry* enabled;
				TreeEntry* draw_on_cooldown;
				TreeEntry* cooldown_alpha;
				std::map<uint32_t, std::map< uint32_t, s_override_spell> > m_config = { };
			}
		}

		auto load_menu( TreeTab* aa_tracker ) -> void
		{
			aa_tracker->add_separator( "s_ranges", " - -  General  - - " );
			cfg::draw_ranges = aa_tracker->add_checkbox( "draw_ranges", "Enabled", true );
			cfg::in_depth_ranges = aa_tracker->add_checkbox( "in_depth_ranges", "In-depth", true );
			aa_tracker->set_assigned_active( cfg::draw_ranges );

			aa_tracker->add_separator( "aa_ranges", " - -  AA ranges  - - " );
			{
				cfg::aa::enabled = aa_tracker->add_checkbox( "aa_enabled_general", "Enabled", false );
				for ( auto&& entry_name : cfg::m_aa_entries )
				{
					auto display_name = entry_name; display_name [ 0 ] = std::toupper( display_name [ 0 ] );

					auto tab = aa_tracker->add_tab( "aa_ranges_" + entry_name, display_name );
					tab->add_separator( "sep_general", " - -  General  - - " );
					cfg::s_override_spell config = cfg::s_override_spell( );

					std::vector<std::pair<std::string, void*>> elements = 
					{ 
						{ "Glow", nullptr },
						{ "Glow gradient", nullptr }
					};

					if ( entry_name == "enemy" )
					{
						elements.push_back( { "Arc", nullptr } );
						elements.push_back( { "Arc gradient", nullptr } );
					}

					config.enabled = tab->add_checkbox( "enabled", "Enabled", false );

					tab->add_separator( "sep_customization", " - -  Customization  - - " );
					config.glow_type = tab->add_combobox( "glow_type", "Glow type", elements, 1 );
					config.arc_angle = tab->add_slider( "arc_angle", "Arc angle", 180, 0, 340 );
					config.thickness = tab->add_slider( "thickness", "Thickness", 4, 1, 5 );

					static std::vector<const char*> m_glow_entry_names =
					{
						"Glow inside size",
						"Glow inside power",
						"Glow outside size",
						"Glow outside power"
					};

					tab->add_separator( "sep_glow", " - -  Glow  - - " );
					static constexpr int32_t m_default_glow[ 4 ] = { 0, 0, 100, 33 };
					for ( auto i = 0; i < 4; i++ )
					{
						config.glow [ i ] = tab->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names[ i ], m_default_glow[ i ], 0, 100 );
						config.glow [ i ] = tab->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names [ i ], m_default_glow [ i ], 0, 100 );
						config.glow [ i ] = tab->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names [ i ], m_default_glow [ i ], 0, 100 );
						config.glow [ i ] = tab->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names [ i ], m_default_glow [ i ], 0, 100 );
					}

					tab->add_separator( "sep_colors", " - -  Colors  - - " );
					config.color_1 = tab->add_colorpick( "color1", "Color 1", cfg::default_rng_clr_1 );
					config.color_2 = tab->add_colorpick( "color2", "Color 2", cfg::default_rng_clr_2 );

					cfg::aa::m_config [ buff_hash_real( entry_name.c_str( ) ) ] = config;
					tab->set_assigned_active( config.enabled );
				}

				auto glow_type_click = [ ]( TreeEntry* ) 
				{
					for ( auto&& entry : cfg::aa::m_config )
					{
						entry.second.arc_angle->is_hidden( ) = entry.second.glow_type->get_int( ) < 2;
						entry.second.color_2->is_hidden( ) = entry.second.glow_type->get_int( ) % 2 == 0;
					}
				};

				glow_type_click( nullptr );
				for ( auto&& x : cfg::aa::m_config )
					x.second.glow_type->add_property_change_callback( glow_type_click );
			}

			aa_tracker->add_separator( "spell_ranges", " - -  Spell ranges  - - " );
			{
				cfg::spell::enabled = aa_tracker->add_checkbox( "spell_enabled_general", "Enabled", false );
				cfg::spell::draw_on_cooldown = aa_tracker->add_checkbox( "spell_draw_on_cooldown", "Draw if on cooldown", false );
				cfg::spell::cooldown_alpha = aa_tracker->add_slider( "spell_cooldown_alpha", " ^ - Alpha", 60, 0, 255 );

				auto draw_if_cd_click = [ ]( TreeEntry* s ) {
					cfg::spell::cooldown_alpha->is_hidden( ) = !s->get_bool( );
				};
				draw_if_cd_click( cfg::spell::draw_on_cooldown );
				cfg::spell::draw_on_cooldown->add_property_change_callback( draw_if_cd_click );

				for ( auto&& entry_name : cfg::m_aa_entries )
				{
					auto display_name = entry_name; display_name [ 0 ] = std::toupper( display_name [ 0 ] );

					auto base_tab = aa_tracker->add_tab( "spell_ranges_" + entry_name, display_name );
					base_tab->add_separator( "sep_general", " - -  General  - - " );

					std::vector<game_object_script> list = { myhero };
					std::string base_key = "me";
					if ( entry_name == "enemy" )
					{
						base_key = "enemy";
						list = entitylist->get_enemy_heroes( );
					}
					else if ( entry_name == "ally" )
					{
						base_key = "ally";
						list = entitylist->get_ally_heroes( );
					}

					for ( auto&& x : list )
					{
						if ( !x || !x->is_valid( ) )
							continue;

						if ( base_key == "ally" && x->is_me( ) )
							continue;

						auto tab = base_tab->add_tab( base_key + "_" + x->get_model( ), x->get_model( ) );
						tab->set_texture( x->get_square_icon_portrait( ) );

						for ( uint32_t i = static_cast< uint32_t >( spellslot::q ); i <= static_cast< uint32_t >( spellslot::r ); i++ )
						{
							auto slot = static_cast< spellslot >( i );
							auto spell = x->get_spell( slot );
							if ( !spell )
								continue;

							auto sdata = spell->get_spell_data( );
							if ( !sdata )
								continue;

							auto range = *sdata->CastRangeDisplayOverride( );
							if ( range <= 0.f )
								continue;

							auto tab_spell = tab->add_tab( cfg::m_spells [ slot ], cfg::m_spells [ slot ] );
							{
								tab_spell->set_texture( spell->get_icon_texture( ) );
								cfg::s_override_spell config = cfg::s_override_spell( );

								std::vector<std::pair<std::string, void*>> elements =
								{
									{ "Glow", nullptr },
									{ "Glow gradient", nullptr }
								};

								if ( entry_name == "enemy" )
								{
									elements.push_back( { "Arc", nullptr } );
									elements.push_back( { "Arc gradient", nullptr } );
								}

								tab_spell->add_separator( "sep_spellslot", " - -  " + cfg::m_spells [ slot ] + "  - - " );
								config.enabled = tab_spell->add_checkbox( "enabled", "Enabled", false );


								tab_spell->add_separator( "sep_customization", " - -  Customization  - - " );
								config.glow_type = tab_spell->add_combobox( "glow_type", "Glow type", elements, 1 );
								config.arc_angle = tab_spell->add_slider( "arc_angle", "Arc angle", 180, 0, 340 );
								config.thickness = tab_spell->add_slider( "thickness", "Thickness", 4, 1, 5 );

								static std::vector<const char*> m_glow_entry_names =
								{
									"Glow inside size",
									"Glow inside power",
									"Glow outside size",
									"Glow outside power"
								};

								tab_spell->add_separator( "sep_glow", " - -  Glow  - - " );
								static constexpr int32_t m_default_glow [ 4 ] = { 0, 0, 100, 33 };
								for ( auto i = 0; i < 4; i++ )
								{
									config.glow [ i ] = tab_spell->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names [ i ], m_default_glow [ i ], 0, 100 );
									config.glow [ i ] = tab_spell->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names [ i ], m_default_glow [ i ], 0, 100 );
									config.glow [ i ] = tab_spell->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names [ i ], m_default_glow [ i ], 0, 100 );
									config.glow [ i ] = tab_spell->add_slider( "glow_" + std::to_string( i ), m_glow_entry_names [ i ], m_default_glow [ i ], 0, 100 );
								}

								tab_spell->add_separator( "sep_colors", " - -  Colors  - - " );
								config.color_1 = tab_spell->add_colorpick( "color1", "Color 1", cfg::default_rng_clr_1 );
								config.color_2 = tab_spell->add_colorpick( "color2", "Color 2", cfg::default_rng_clr_2 );


								tab_spell->set_assigned_active( config.enabled );
								cfg::spell::m_config [ buff_hash_real( base_key.c_str( ) ) ][ i ] = config;
							}
						}
					}
				}

				auto glow_type_click = [ ]( TreeEntry* ) 
				{
					for ( auto&& entry : cfg::spell::m_config )
					{
						for ( auto&& entry2 : entry.second )
						{
							entry2.second.arc_angle->is_hidden( ) = entry2.second.glow_type->get_int( ) < 2;
							entry2.second.color_2->is_hidden( ) = entry2.second.glow_type->get_int( ) % 2 == 0;
						}
					}
				};

				glow_type_click( nullptr );
				for ( auto&& entry : cfg::spell::m_config )
					for ( auto&& entry2 : entry.second )
						entry2.second.glow_type->add_property_change_callback( glow_type_click );
			}
		}

		auto draw_ranges( bool is_env_draw ) -> void
		{
			if ( !cfg::draw_ranges->get_bool( ) )
				return;

			if ( is_env_draw != cfg::in_depth_ranges->get_bool( ) )
				return;

			auto draw_aa_range = [ ]( game_object_script x, uint32_t hash )
			{
				auto it_aa = cfg::aa::m_config.find( hash );
				if ( it_aa == cfg::aa::m_config.end( ) || !it_aa->second.enabled->get_bool( ) )
					return;

				auto mode = it_aa->second.glow_type->get_int( );
				auto range = x->get_attack_range( ) + x->get_bounding_radius( );

				if ( mode == 0 )
				{
					draw_manager->add_circle_with_glow_gradient( x->get_position( ),
						it_aa->second.color_1->get_color( ),
						it_aa->second.color_1->get_color( ),
						range,
						static_cast< float >( it_aa->second.thickness->get_int( ) ),
						it_aa->second.get_glow_data( ) );

				}
				else if ( mode == 1 )
				{
					draw_manager->add_circle_with_glow_gradient( x->get_position( ),
						it_aa->second.color_1->get_color( ),
						it_aa->second.color_2->get_color( ),
						range,
						static_cast< float >( it_aa->second.thickness->get_int( ) ),
						it_aa->second.get_glow_data( ) );
				}
				else
				{
					//2 = static_arc, 3 = gradient
					auto arc_angle = it_aa->second.arc_angle->get_int( ) / 2.f;
					auto end_angle = arc_angle;
					if ( arc_angle == 0 )
					{
						end_angle = range / std::max( range, ( myhero->get_distance( x ) + 300.f ) ) * 180.f;
						end_angle = std::clamp( end_angle, x->is_melee( ) ? 90.f : 30.f, 270.f );
					}

					auto end_color = mode == 3 ? it_aa->second.color_2->get_color( ) : it_aa->second.color_1->get_color( );
					A_::draw_gradient_arc( x->get_position( ), range, 0, end_angle, it_aa->second.color_1->get_color( ), end_color, static_cast< float >( it_aa->second.thickness->get_int( ) ) );
					A_::draw_gradient_arc( x->get_position( ), range, 0, -end_angle, it_aa->second.color_1->get_color( ), end_color, static_cast< float >( it_aa->second.thickness->get_int( ) ) );
				}
			};

			auto draw_spell_range = [ ]( game_object_script x, uint32_t hash )
			{
				auto it_cfg = cfg::spell::m_config.find( hash );
				if ( it_cfg == cfg::spell::m_config.end( ) )
					return;

				for ( auto i = static_cast< uint32_t >( spellslot::q ); i <= static_cast< uint32_t >( spellslot::r ); i++ )
				{
					auto slot = static_cast< spellslot >( i );
					auto it = it_cfg->second.find( i );
					if ( it == it_cfg->second.end( ) || !it->second.enabled->get_bool( ) )
						continue;

					auto spell = x->get_spell( slot );
					if ( !spell || !spell->is_learned( ) )
						continue;

					auto sdata = spell->get_spell_data( );
					if ( !sdata )
						continue;

					auto opt_cooldown = g_cooldown->spell_cd( x, slot );
					auto cooldown = opt_cooldown.has_value( ) ? opt_cooldown.value( ) : spell->cooldown( );

					if ( !cfg::spell::draw_on_cooldown->get_bool( ) && cooldown > 0.f )
						continue;

					auto color1 = it->second.color_1->get_color( ),
						color2 = it->second.color_2->get_color( );

					if ( cooldown > 0.f )
					{
						color1 = A_::set_alpha_percent( color1, static_cast<float>( cfg::spell::cooldown_alpha->get_int( ) ) / 255.f );
						color2 = A_::set_alpha_percent( color2, static_cast< float >( cfg::spell::cooldown_alpha->get_int( ) ) / 255.f );
					}

					auto range = *sdata->CastRangeDisplayOverride( );
					if ( sdata->CastRangeUseBoundingBoxes( ) )
						range += x->get_bounding_radius( );

					auto mode = it->second.glow_type->get_int( );
					if ( mode == 0 )
					{
						draw_manager->add_circle_with_glow_gradient( x->get_position( ),
							color1,
							color1,
							range,
							static_cast< float >( it->second.thickness->get_int( ) ),
							it->second.get_glow_data( ) );
					}
					else if ( mode == 1 )
					{
						draw_manager->add_circle_with_glow_gradient( x->get_position( ),
							color1,
							color2,
							range,
							static_cast< float >( it->second.thickness->get_int( ) ),
							it->second.get_glow_data( ) );
					}
					else
					{
						//2 = static_arc, 3 = gradient
						auto arc_angle = it->second.arc_angle->get_int( ) / 2.f;
						auto end_angle = arc_angle;
						if ( arc_angle == 0 )
						{
							end_angle = range / std::max( range, ( myhero->get_distance( x ) + 300.f ) ) * 180.f;
							end_angle = std::clamp( end_angle, x->is_melee( ) ? 90.f : 30.f, 270.f );
						}

						auto end_color = mode == 3 ? color2 : color1;
						A_::draw_gradient_arc( x->get_position( ), range, 0, end_angle, color1, end_color, static_cast<float>( it->second.thickness->get_int( ) ) );
						A_::draw_gradient_arc( x->get_position( ), range, 0, -end_angle, color1, end_color, static_cast< float >( it->second.thickness->get_int( ) ) );
					}
				}
			};

			for ( auto&& x : entitylist->get_all_heroes( ) )
			{
				if ( !x || !x->is_valid( ) || x->is_dead( ) || !x->is_visible( ) )
					continue;

				auto hash = buff_hash( "enemy" );
				if ( x->is_me( ) )
					hash = buff_hash( "me" );
				else if ( x->is_ally( ) )
					hash = buff_hash( "ally" );

				if ( cfg::aa::enabled->get_bool( ) )
					draw_aa_range( x, hash );

				if ( cfg::spell::enabled->get_bool( ) )
					draw_spell_range( x, hash );
			}

			//for ( auto&& x : entitylist->get_all_heroes( ) )
			//{
			//	if ( !x || !x->is_valid( ) || x->is_dead( ) || !x->is_visible( ) )
			//		continue;

			//	std::string entry_name = "enemies";
			//	if ( x->is_me( ) )
			//	{
			//		entry_name = "me";
			//		if ( !cfg::draw_ranges_me->get_bool( ) )
			//			continue;
			//	}
			//	else if ( !x->is_me( ) && x->is_ally( ) )
			//	{
			//		entry_name = "allies";
			//		if ( !cfg::draw_ranges_ally->get_bool( ) )
			//			continue;
			//	}
			//	else
			//		if ( !cfg::draw_ranges_enemy->get_bool( ) )
			//			continue;

			//	if ( cfg::ranges_whitelist.find( x->get_champion( ) ) == cfg::ranges_whitelist.end( ) )
			//		continue;

			//	for ( auto&& e : cfg::ranges_whitelist [ x->get_champion( ) ].priority->get_prority_sorted_list( ) )
			//	{
			//		if ( !e.is_active )
			//			continue;

			//		auto range = 0.f;
			//		if ( e.display_name == "AA" )
			//			range = x->get_attack_range( ) + x->get_bounding_radius( );
			//		else
			//		{
			//			auto spell = x->get_spell( cfg::map_key [ e.display_name [ 0 ] ] );
			//			if ( spell && spell->get_spell_data( ) && spell->is_learned( ) && spell->cooldown( ) <= 0.f )
			//				range = std::max( *spell->get_spell_data( )->CastRangeDisplayOverride( ), *spell->get_spell_data( )->CastRangeDisplayOverride( ) ) + ( spell->get_spell_data( )->CastRangeUseBoundingBoxes( ) ? x->get_bounding_radius( ) : 0.f );
			//		}

			//		if ( range <= 0 )
			//			continue;

			//		auto b_custom = cfg::ranges_whitelist [ x->get_champion( ) ].custom->get_bool( );

			//		auto glow_ind_p = b_custom ?
			//			cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].glow_ind_p->get_int( ) / 100.f
			//			: cfg::ranges [ entry_name ].glow_ind_p->get_int( ) / 100.f;

			//		auto glow_ind_s = b_custom ? cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].glow_ind_s->get_int( ) / 100.f
			//			: cfg::ranges [ entry_name ].glow_ind_s->get_int( ) / 100.f;

			//		auto glow_out_p = b_custom ? cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].glow_out_p->get_int( ) / 100.f
			//			: cfg::ranges [ entry_name ].glow_out_p->get_int( ) / 100.f;

			//		auto glow_out_s = b_custom ? cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].glow_out_s->get_int( ) / 100.f
			//			: cfg::ranges [ entry_name ].glow_out_s->get_int( ) / 100.f;

			//		auto glow_type = b_custom ? cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].glow_type->get_int( )
			//			: cfg::ranges [ entry_name ].glow_type->get_int( );

			//		auto color_1 = b_custom ? cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].color_1->get_color( )
			//			: cfg::ranges [ entry_name ].color_1->get_color( );

			//		auto color_2 = b_custom ? cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].color_2->get_color( )
			//			: cfg::ranges [ entry_name ].color_2->get_color( );

			//		auto t = b_custom ? static_cast< float >( cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].thickness->get_int( ) )
			//			: static_cast< float >( cfg::ranges [ entry_name ].thickness->get_int( ) );

			//		auto arc_angle = b_custom ? cfg::ranges_whitelist [ x->get_champion( ) ].overrides [ e.display_name ].arc_angle->get_int( ) / 2.f
			//			: cfg::ranges [ entry_name ].arc_angle->get_int( ) / 2.f;

			//		if ( glow_type <= 1 || !x->is_enemy( ) )
			//		{
			//			draw_manager->add_circle_with_glow_gradient( x->get_position( ),
			//				color_1,
			//				glow_type == 0 ? color_1 : color_2,
			//				range,
			//				t,
			//				glow_data( glow_ind_s, glow_ind_p, glow_out_s, glow_out_p ) );
			//		}
			//		else
			//		{
			//			//2 = static_arc, 3 = gradient
			//			auto end_angle = arc_angle;
			//			if ( arc_angle == 0 )
			//			{
			//				end_angle = range / std::max( range, ( myhero->get_distance( x ) + 300.f ) ) * 180.f;
			//				end_angle = std::clamp( end_angle, x->is_melee( ) ? 90.f : 30.f, 270.f );
			//			}

			//			auto end_color = glow_type == 3 ? color_2 : color_1;
			//			A_::draw_gradient_arc( x->get_position( ), range, 0, end_angle, color_1, end_color, t );
			//			A_::draw_gradient_arc( x->get_position( ), range, 0, -end_angle, color_1, end_color, t );
			//		}
			//	}
			//}
		}
	}
}