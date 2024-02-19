#include "misc.h"
#include "utility.h"
#include "fake_cursor.h"
#include "A_.h"
#include "orb.h"

namespace misc
{
	namespace cfg
	{
		TreeEntry* melee_magnet = nullptr;

		TreeEntry* red_enabled = nullptr;
		TreeEntry* green_enabled = nullptr;
		TreeEntry* red_extra_range = nullptr;
		TreeEntry* green_delay_min = nullptr;
		TreeEntry* green_delay_max = nullptr;

		TreeEntry* text_use = nullptr;
		TreeEntry* text_word = nullptr;
		TreeEntry* text_color = nullptr;
		TreeEntry* text_size = nullptr;
		TreeEntry* text_animate = nullptr;

		TreeEntry* ff_list = nullptr;

		TreeEntry* orb_plant = nullptr;

		TreeEntry* muteself = nullptr;

		TreeEntry* follow_mode = nullptr;
		std::map<std::uint32_t, TreeEntry*> follow_mode_on;
		TreeEntry* rundown = nullptr;
		TreeEntry* show_emote_type = nullptr;
		TreeEntry* show_master = nullptr;

		TreeEntry* map_mode;
		TreeEntry* map_color;
		TreeEntry* map_r;
		TreeEntry* map_g;
		TreeEntry* map_b;
		TreeEntry* map_a;

		TreeEntry* spam_mode;
		TreeEntry* spam_key;
		TreeEntry* spam_key_swap_mode;

		

		bool* follow_active;
		bool* show_emote_active;
		bool* chat_active;

		bool* map_active;

		bool* cursor_active;
	}

	bool mute_loop = true;
	game_object_script ally_base, enemy_base;
	bool Afk_state = true;
	float Afk_delay = 0;

	void create_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto misc_tab = main_tab->add_tab("misc", "Misc");
		if (misc_tab)
		{
			misc_tab->add_separator("Misc_1", " - - Misc - - ");

			auto map_tab = misc_tab->add_tab( "map_tab", "Custom map" );
#pragma region map_tab
			map_tab->add_separator( "custom_map_sep", " - - Colors - - " );
			cfg::map_mode = map_tab->add_combobox( "map_mode", "Map mode", { 
				{ "Disabled", nullptr }, 
				{ "Custom", nullptr }, 
				{ "Purple", nullptr } ,
				{ "Pink", nullptr },
				{ "Midnight", nullptr },
				{ "White", nullptr }
				}, 0 );
			cfg::map_active = new bool( false ); map_tab->set_assigned_active( cfg::map_active );

			float default_color[4] = { 1.f, 1.f, 1.f, 0.02f };
			cfg::map_color = map_tab->add_colorpick( "map_color", "Map color", default_color );
			cfg::map_r = map_tab->add_slider( "map_r", "Map red", 0, 0, 255 );
			cfg::map_g = map_tab->add_slider( "map_g", "Map green", 11, 0, 255 );
			cfg::map_b = map_tab->add_slider( "map_b", "Map blue", 30, 0, 255 );
			cfg::map_a = map_tab->add_slider( "map_a", "Map alpha", 50, 0, 255 );

#pragma region color_callbacks
			const auto on_map_mode_click = [ ]( TreeEntry* self ) {
				*cfg::map_active = self->get_int( ) > 0;

				cfg::map_color->is_hidden( ) = 
					cfg::map_r->is_hidden( ) = 
					cfg::map_g->is_hidden( ) = 
					cfg::map_b->is_hidden( ) = 
					cfg::map_a->is_hidden( ) = self->get_int( ) != 1;
			};

			cfg::map_color->add_property_change_callback( [ ]( TreeEntry* self ) {
				auto clr = self->get_color( );
				cfg::map_r->set_int( clr & 0xff );
				cfg::map_g->set_int( ( clr >> 8 ) & 0xff );
				cfg::map_b->set_int( ( clr >> 16 ) & 0xff );
				cfg::map_a->set_int( ( clr >> 24 ) & 0xff );
			} );

			const auto on_map_color_change = [ ]( TreeEntry* ) {
				float rgba [ 4 ] = {
					cfg::map_r->get_int( ) / 255.f,
					cfg::map_g->get_int( ) / 255.f,
					cfg::map_b->get_int( ) / 255.f,
					cfg::map_a->get_int( ) / 255.f,
				};

				cfg::map_color->set_color( rgba );
			};
			cfg::map_r->add_property_change_callback( on_map_color_change );
			cfg::map_g->add_property_change_callback( on_map_color_change );
			cfg::map_b->add_property_change_callback( on_map_color_change );
			cfg::map_a->add_property_change_callback( on_map_color_change );

			cfg::map_mode->add_property_change_callback( on_map_mode_click );
			on_map_mode_click( cfg::map_mode );
			on_map_color_change( nullptr );
#pragma endregion color_callbacks
#pragma endregion map_tab

			auto fake_cursor_tab = misc_tab->add_tab("fake_cursor_tab", "Fake cursor (Beta)");
			{
				fake_cursor_tab->add_separator("green_cursor_tab", " - - Move - - ");
				cfg::green_enabled = fake_cursor_tab->add_checkbox("green_enabled", "Enabled", false);
				cfg::green_delay_min = fake_cursor_tab->add_slider("green_delay_min", "Min delay ( ms )", 125, 50, 1000);
				cfg::green_delay_max = fake_cursor_tab->add_slider("green_delay_max", "Max delay ( ms )", 333, 100, 1000);

				fake_cursor_tab->add_separator("red_cursor_tab", " - - Attack - - ");
				cfg::red_enabled = fake_cursor_tab->add_checkbox("red_enabled", "Enabled", false);
				cfg::red_extra_range = fake_cursor_tab->add_slider("red_extra_range", "Extra randomize range", 15, 0, 125);

				cfg::cursor_active = new bool( false );
				fake_cursor_tab->set_assigned_active( cfg::cursor_active );
				const auto on_cursor_click = [ ]( TreeEntry* ) {
					*cfg::cursor_active = cfg::green_enabled->get_bool( ) || cfg::red_enabled->get_bool( );
				};
				on_cursor_click( nullptr );
				cfg::green_enabled->add_property_change_callback( on_cursor_click );
				cfg::red_enabled->add_property_change_callback( on_cursor_click );
			}

			auto orb_s = misc_tab->add_tab("orb_s", "Orbwalker");
			{
				orb_s->add_separator("orb_s1", " - - Orbwalker - - ");
				cfg::orb_plant = orb_s->add_checkbox("orb_plant", "Only fast laneclear hit plant", false);
				orb_s->add_separator("orb_s2", "  ^- Disable Orbwalker -> Misc -> Attack object -> Attack plants");

				auto magnet_sep = orb_s->add_separator( "chase_1", " - - Magnet - - " );
				cfg::melee_magnet = orb_s->add_hotkey( "magnet", "Key", TreeHotkeyMode::Hold, 0, false );
				auto chase_sep = orb_s->add_separator( "chase_2", "Credit : Chase (1v9)" );

				magnet_sep->is_hidden( ) = cfg::melee_magnet->is_hidden( ) = chase_sep->is_hidden( ) = !myhero->is_melee( );
			}

			auto ff_menu = misc_tab->add_tab("ff_menu", "Show FF");
			{
				ff_menu->add_separator("text_1", " - - Show FF - - ");
				cfg::ff_list = ff_menu->add_checkbox("show", "Show FF", true);
				ff_menu->set_assigned_active(cfg::ff_list);
			}

			auto chat = misc_tab->add_tab("chat", "Chat");
			{
				chat->add_separator("chat", " - - Chat - - ");
				cfg::muteself = chat->add_combobox("print_mute", "Chat setting", { { "None", nullptr }, { "Muteself", nullptr }, { "Deafen", nullptr }, { "Muteall", nullptr } }, 0, true);

				cfg::chat_active = new bool( false );
				const auto on_chat_click = [ ]( TreeEntry* self ) {
					*cfg::chat_active = cfg::muteself->get_int( ) > 0;
					mute_loop = true;
				};
				on_chat_click( cfg::muteself );
				cfg::muteself->add_property_change_callback( on_chat_click );
				chat->set_assigned_active( cfg::chat_active );
			}
			auto afk = misc_tab->add_tab("afk", "Follow ally mode");
			{
				afk->add_separator( "follow_ally_Sep", " - - Follow ally - - " );
				cfg::follow_mode = afk->add_checkbox("afk_mode", "Follow ally mode", false, true);
				for (auto& ally : entitylist->get_ally_heroes())
				{
					if (!ally->is_me())
					{
						cfg::follow_mode_on[ally->get_network_id()] = afk->add_checkbox(std::to_string((int)ally->get_champion()), ally->get_model(), true, false);
						cfg::follow_mode_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
					}
				}
				cfg::rundown = afk->add_checkbox("ban", "Auto rundown", false, false);

				cfg::follow_active = new bool( false );
				afk->set_assigned_active( cfg::follow_active );

				const auto on_run_follow_click = [ ]( TreeEntry* x ) {
					*cfg::follow_active = cfg::rundown->get_bool( ) || cfg::follow_mode->get_bool( );
				};	
				cfg::rundown->add_property_change_callback( on_run_follow_click );
				cfg::follow_mode->add_property_change_callback( on_run_follow_click );
				on_run_follow_click( nullptr );
			}
			auto show_emote = misc_tab->add_tab("show_emote", "Emote/mastery/dance/movement");
			{
				show_emote->add_separator("show_emote", " - - On kill - - ");
				cfg::show_emote_type = show_emote->add_combobox("show_emote_type", "Show Emote Type", {
					{ "None", nullptr }, { "Top", nullptr }, { "Top Right", nullptr }, { "Right", nullptr }, { "Right Down", nullptr }, { "Down", nullptr }, { "Left Down", nullptr }, { "Left", nullptr }, { "Top Left", nullptr }, { "Mid", nullptr } }, 0, true);

				cfg::show_master = show_emote->add_checkbox("show_master", "Show mastery", false, true);

				show_emote->add_separator( "emote_spam", " - - Spam - - " );
				cfg::spam_key = show_emote->add_hotkey( "emote_spam_key", "Emote spam key", TreeHotkeyMode::Hold, 0x00, false );
				cfg::spam_mode = show_emote->add_combobox( "spam_mode", "Mode", {
					{ "Dance", nullptr }, { "Taunt", nullptr }, { "Laugh", nullptr }, { "Joke", nullptr }, { "Toggle", nullptr },
					}, 1 );
				cfg::spam_key_swap_mode = show_emote->add_hotkey( "spam_key_swap_mode", "Next mode", TreeHotkeyMode::Toggle, 0x00, false );
				cfg::spam_key_swap_mode->add_property_change_callback( [ ]( TreeEntry* ) {
					cfg::spam_mode->set_int( cfg::spam_mode->get_int( ) >= 4 ? 0 : cfg::spam_mode->get_int( ) + 1 );
				} );

				cfg::show_emote_active = new bool( false );
				const auto on_show_emt_ms_click = [ ]( TreeEntry* ) {
					*cfg::show_emote_active = cfg::show_master->get_bool( ) || cfg::show_emote_type->get_int( ) > 0;
				};
				cfg::show_emote_type->add_property_change_callback( on_show_emt_ms_click );
				cfg::show_master->add_property_change_callback( on_show_emt_ms_click );
				on_show_emt_ms_click( nullptr );
				show_emote->set_assigned_active( cfg::show_emote_active );
			}
			auto text = misc_tab->add_tab("text", "Watermark");
			{
				text->add_separator("text_1", " - - Watermark - - ");
				cfg::text_use = text->add_checkbox("use", "Draw watermark", false);
				text->set_assigned_active(cfg::text_use);

				cfg::text_word = text->add_text_input("word", "Text", ": utils");
				cfg::text_size = text->add_slider("size", "Font size", 50, 10, 60);
				float color[] = { 1.0, 0, 0, 1.0 };
				cfg::text_color = text->add_colorpick("color", "Color", color);
				cfg::text_animate = text->add_checkbox("animat", "Animate alpha", true);

				const auto on_watermark_click = [ ]( TreeEntry* self )
				{
					cfg::text_word->is_hidden( ) =
						cfg::text_size->is_hidden( ) =
						cfg::text_color->is_hidden( ) =
						cfg::text_animate->is_hidden( ) = !self->get_bool( );
				};
				cfg::text_use->add_property_change_callback( on_watermark_click );
				on_watermark_click( cfg::text_use );
			}
			/*auto translate = misc_tab->add_tab( "translate", "Translate" );
			{
				translate->add_separator( "note1", "Now in Chinese only" );
				translate->add_separator( "note2", "You need change language to chinese" );
				translate->add_separator( "note3", "The following plugins are supported" );
				translate->add_separator( "note4", "Kiri" );
				translate->add_separator( "note5", "OpenUtilities" );
				translate->add_separator( "note6", "CSXAIO" );
			}*/
		}
	}

	//text
	void show_text()
	{
		if (!cfg::text_use->get_bool()) 
			return;

		std::string word = cfg::text_word->get_string();

		auto size = draw_manager->calc_text_size( cfg::text_size->get_int( ), word.c_str());
		auto pos = vector(( renderer->screen_width( ) - size.x) / 2.f, 20);

		draw_manager->add_text_on_screen( pos, cfg::text_animate->get_bool( ) ?
			A_::animate_color( cfg::text_color->get_color( ), 120, 255 ) : cfg::text_color->get_color( ),
			cfg::text_size->get_int( ), word.c_str( ) );
	}

	//ff
	float ff_time;
	struct ff
	{
		game_object_script target;
		bool vote;
	};
	std::vector<ff> ff_list;
	void ff_20()
	{
		if (gametime->get_time() > ff_time)
			ff_list.clear();
		if (gametime->get_time() > ff_time || !cfg::ff_list->get_bool()) return;
		float x = renderer->screen_width() - 230.f;
		float y = renderer->screen_height() / 2.f;

		if (ff_list.size() == 0) return;

		for (auto target : ff_list)
		{
			std::string text = target.vote ? "[Yes] " + target.target->get_model() : "[No] " + target.target->get_model();
			auto cor = target.vote ? MAKE_COLOR(0, 255, 50, 255) : MAKE_COLOR(255, 50, 50, 255);
			draw_manager->add_text_on_screen(vector(x, y), cor, 20, text.c_str());
			y += 25;
		}

	}

	//chat
	void mute_chat()
	{
		switch ( cfg::muteself->get_int( ) )
		{
			default: 
				break;

			case 1:
			{
				if ( mute_loop )
				{
					myhero->send_chat( "/muteself" );
					mute_loop = false;
				}
				break;
			}

			case 2:
			{
				if ( mute_loop )
				{
					myhero->send_chat( "/deafen" );
					mute_loop = false;
				}
				break;
			}

			case 3:
			{
				if ( mute_loop )
				{
					myhero->send_chat( "/muteall" );
					myhero->send_chat( "/mute all" );
					mute_loop = false;
				}
				break;
			}
			case 4:
				mute_loop = false;
				break;
		}
	}

	//rundown
	void auto_rundown()
	{
		if (!cfg::rundown->get_bool() )
			return;
		static auto t_last_order = -8.f;

		if (gametime->get_time() - t_last_order < 0.3f)
			return;

		auto spawnpoints = entitylist->get_all_spawnpoints();
		spawnpoints.erase(std::remove_if(spawnpoints.begin(), spawnpoints.end(), [](game_object_script a)
			{
				return !a || !a->is_valid( ) || !a->is_enemy( );
			}), spawnpoints.end());

		std::sort(spawnpoints.begin(), spawnpoints.end(), [](game_object_script a, game_object_script b)
			{
				return a->get_distance(myhero) < b->get_distance(myhero);
			});

		if (spawnpoints.empty())
			return;

		auto path_controller = myhero->get_path_controller();
		if (path_controller && path_controller->get_end_vec().is_valid() && path_controller->get_end_vec() == spawnpoints.front()->get_position())
			return;

		myhero->issue_order(spawnpoints.front()->get_position(), false, false);
		t_last_order = gametime->get_time();
	}
	void afk_logic()
	{
		static auto t_last_order = -8.f;

		if (gametime->get_time() - t_last_order < 0.05f)
			return;

		auto team = entitylist->get_ally_heroes();
		team.erase(std::remove_if(team.begin(), team.end(), [](game_object_script x)
			{
				return !x || !x->is_valid( ) || x->is_dead( ) || x->get_distance( ally_base ) < 700 || x->get_champion( ) == champion_id::Yuumi || x->is_me( );
			}), team.end());

		std::sort(team.begin(), team.end(), [](game_object_script a, game_object_script b)
			{
				return myhero->get_distance(a) < myhero->get_distance(b);
			});

		if (!team.empty())
		{
			for (auto&& team : team)
			{
				if (A_::checkbox(cfg::follow_mode_on, team))
				{
					if (team->get_distance(ally_base->get_position()) < 800)
						continue;

					if (myhero->get_health_percent() > 65)
					{
						if (myhero->count_enemies_in_range(myhero->get_attack_range() + myhero->get_bounding_radius()) == 1)
						{
							myhero->issue_order(ally_base->get_position());
							t_last_order = gametime->get_time();
						}
						if (myhero->count_enemies_in_range(1500) <= 1)
						{
							auto lane_minions = entitylist->get_enemy_minions();
							auto monsters = entitylist->get_jugnle_mobs_minions();

							lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
								{
									return !x->is_valid_target(myhero->get_attackRange() + myhero->get_bounding_radius());
								}), lane_minions.end());

							monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
								{
									return !x->is_valid_target(myhero->get_attackRange() + myhero->get_bounding_radius());
								}), monsters.end());

							std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
								{
									return a->get_position().distance(myhero->get_position()) < b->get_position().distance(myhero->get_position());
								});

							std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
								{
									return a->get_max_health() > b->get_max_health();
								});

							if (team->get_distance(myhero) < 400 && myhero->get_distance(ally_base) < team->get_distance(ally_base))
							{
								if (!lane_minions.empty())
								{
									myhero->issue_order(lane_minions.front());
									t_last_order = gametime->get_time();
								}
								else if (!monsters.empty())
								{
									myhero->issue_order(monsters.front());
									t_last_order = gametime->get_time();
								}
								else
								{
									auto pos = team->get_position().extend(ally_base->get_position(), 250);
									myhero->issue_order(pos);
									t_last_order = gametime->get_time();
								}
							}
							if (team->get_distance(myhero) >= 400)
							{
								auto pos = team->get_position().extend(ally_base->get_position(), 250);
								myhero->issue_order(pos);
								t_last_order = gametime->get_time();
							}
						}
						if (myhero->count_enemies_in_range(1500) > 1)
						{
							myhero->issue_order(ally_base->get_position());
							t_last_order = gametime->get_time();
						}
					}

					if (myhero->get_health_percent() <= 65)
					{
						myhero->issue_order(ally_base->get_position());
						t_last_order = gametime->get_time();
					}
				}
			}
		}
		else
		{
			myhero->issue_order(ally_base->get_position());
			t_last_order = gametime->get_time();
		}
	}
	void follow()
	{
		if (!ally_base || !enemy_base ) return;
		if (cfg::rundown->get_bool() || !cfg::follow_mode->get_bool())return;

		if (myhero->get_distance(ally_base) < 800 && myhero->get_gold() > 900)
		{
			myhero->buy_item(ItemId::Giants_Belt);
		}

		if (myhero->get_health_percent() < 99)
		{
			if (myhero->get_distance(ally_base) < 600)
			{
				return;
			}
			else
			{
				if (gametime->get_time() > Afk_delay)
				{
					Afk_delay = gametime->get_time() + 0.5f;
					afk_logic();
				}
			}
		}
		else
		{
			if (gametime->get_time() > Afk_delay)
			{
				Afk_delay = gametime->get_time() + 0.5f;
				afk_logic();
			}
		}
	}

	void on_vote(const on_vote_args& args)
	{
		if (args.vote_type == on_vote_type::surrender)
		{
			ff_time = gametime->get_time() + 70;
			auto target = args.sender;
			bool vote = args.success;
			ff_list.push_back({ target, vote });
		}
	}
	void on_process_spell_cast(game_object_script sender, spell_instance_script spell)
	{
		g_cursor->on_process_spell_cast(sender, spell);
	}

	auto on_notify(std::uint32_t hash_name, const char* event_name, global_event_params_script params) -> void
	{
		if (!params)
			return;

		auto event_id = params->get_argument(0),
			sender = params->get_argument(1);

		auto name = (std::string)event_name;
		std::transform(name.begin(), name.end(), name.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (name.find("onstartgamemessage1") != std::string::npos) //game start
		{
		}

		if (name.find("onminionsspawn") != std::string::npos) //minion spawn
		{
		}

		if (sender != myhero->get_network_id())
			return;

		if (name.find("ondeathassist") != std::string::npos) //assist, works on turrets, objectives
		{
		}

		//onchampionkillpre, onchampiondoublekill, onchampiontriplekill, onchampionquadrakill, onchampionpentakill
		else if (name.find("onchampion") != std::string::npos &&
			name.find("kill") != std::string::npos) //kill
		{
			if (cfg::show_master->get_bool())
				myhero->send_chat( "/masterybadge" );

			if (cfg::show_emote_type->get_int() != 0)
			{
				int i = cfg::show_emote_type->get_int() - 1;
				myhero->request_to_display_emote(static_cast<summoner_emote_slot>(i));
			}
		}
	}

	void all_logic()
	{
		mute_chat();

		if (myhero->is_dead()) 
			return;

		auto_rundown();
		follow();

		/*static auto load_time = gametime->get_time( );
		static auto orb_hash = orbwalker->get_active_orbwalker_name_hash( );

		if ( orb_hash != orbwalker->get_active_orbwalker_name_hash( ) )
		{
			load_time = gametime->get_time( );
			orb_hash = orbwalker->get_active_orbwalker_name_hash( );
		}*/

		//auto dynasty = orbwalker->get_active_orbwalker_name_hash() == 418294543;
		//if (not_dynasty)
		if ( orbwalker->get_active_orbwalker_name_hash( ) != 418294543 )
		{
			orb::spell_check();
			orb::magnet(cfg::melee_magnet->get_bool());
			orb::orb_plant(cfg::orb_plant->get_bool());
		}
	}

	void all_draw()
	{
		if (!myhero->is_dead())
			g_cursor->on_draw();

		show_text();

		if (keyboard_state->is_pressed(keyboard_game::tab))
			return;

		ff_20();

		if (myhero->is_dead()) return;

		if (cfg::rundown->get_bool())
			draw_manager->add_text(myhero->get_position(), MAKE_COLOR(255, 0, 0, 255), 50, "Rundown");
		else if (cfg::follow_mode->get_bool())
			draw_manager->add_text(myhero->get_position(), MAKE_COLOR(255, 0, 0, 255), 50, "Follow mode");
	}

	void all_new_draw()
	{
		if ( !myhero )
			return;

		if ( cfg::map_mode->get_int( ) > 0 )
		{
			auto color = 0;
			switch ( cfg::map_mode->get_int( ) )
			{
				default: break;
				case 1: color = cfg::map_color->get_color( ); break;
				case 2: color = MAKE_COLOR( 162, 155, 254, 10 ); break;
				case 3: color = MAKE_COLOR( 255, 204, 204, 15 ); break; //pink
				case 4: color = MAKE_COLOR( 30, 39, 46, 50 ); break; //midnight
				case 5: color = MAKE_COLOR( 200, 214, 229, 13 ); break; //white
			}

			if ( color > 0 )
				draw_manager->add_rectangle_3d_filled( { 0.f, 0.f, -1.f }, vector( 14800, 14800, -1.f ), color, 14800.f * 2.f, 0.f );
		}

		if (myhero->is_dead()) 
			return;

		orb::magnet_draw(cfg::melee_magnet->get_bool());
	}

	auto event_on_tick( ) -> void
	{
		if ( cfg::spam_key->get_bool( ) )
		{
			static auto t_last_order = -8.f;
			if ( gametime->get_time( ) - t_last_order >= 0.05f )
			{
				myhero->send_emote( ( emote_type )cfg::spam_mode->get_int( ) );
				t_last_order = gametime->get_time( );
			}
		}
	}

	void all_load()
	{
		event_handler<events::on_update>::add_callback( event_on_tick );

		event_handler<events::on_process_spell_cast>::add_callback(on_process_spell_cast);
		event_handler<events::on_vote>::add_callback(on_vote);
		event_handler<events::on_global_event>::add_callback(on_notify);
		g_cursor->load(cfg::red_enabled, cfg::red_extra_range, cfg::green_enabled, cfg::green_delay_max, cfg::green_delay_min);
		for (auto&& target : entitylist->get_all_spawnpoints())
		{
			if (target->is_ally())
				ally_base = target;
			else
				enemy_base = target;
		}
	}

	void all_unload()
	{
		delete cfg::follow_active; delete cfg::show_emote_active; delete cfg::chat_active;
		delete cfg::cursor_active; delete cfg::map_active;

		event_handler<events::on_update>::remove_handler( event_on_tick );

		event_handler<events::on_process_spell_cast>::remove_handler(on_process_spell_cast);
		event_handler<events::on_vote>::remove_handler(on_vote);
		event_handler<events::on_global_event>::remove_handler(on_notify);
	}
}