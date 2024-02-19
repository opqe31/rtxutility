#include "fake_cursor.h"
#include "A_.h"

#include <iostream>
#include <random>
c_fake_cursor* g_cursor = new c_fake_cursor();

/* Lune_Utility
https ://raw.communitydragon.org/latest/game/assets/ux/cursors/

target_enemy_precise.png
target_ally_precise.png

hover_precise.png
hover_enemy_precise.png
hover_ally_precise.png
*/

auto c_fake_cursor::on_process_spell_cast(game_object_script sender, spell_instance_script spell) -> void
{
	if (!sender || !sender->is_valid( ) || !sender->is_me( ) || !initialized || !_red_enabled->get_bool( ) )
		return;

	if (spell->is_auto_attack() && !orbwalker->none_mode())
	{
		//console->print( "cursor_aa" );
		current_render.is_aa = true;
		current_render.is_enemy = true;

		auto target = entitylist->get_object(spell->get_last_target_id());
		auto random_max = ( target && target->is_valid( ) ) ? target->get_bounding_radius( ) * 0.9f : 15.f;
		random_max += _red_extra_range->get_int();

		current_render.start_time = gametime->get_time();
		current_render.end_time = gametime->get_time() + myhero->get_attack_cast_delay();
		current_render.pos = A_::randomize(spell->get_end_position(), -random_max, random_max);

		hud->get_hud_input_logic()->fake_click(current_render.pos, hud_cursor_type::move_to_red);
		//draw_aa_range = true;
	}
}

auto c_fake_cursor::on_draw() -> void
{
	if (!initialized || !_green_enabled->get_bool() || orbwalker->none_mode())
		return;

	/*if ( draw_aa_range )
	{
		auto w2s = vector::zero;

		renderer->world_to_screen( myhero->get_position( ), w2s );
		if ( w2s.is_valid( ) )
		{
			auto size = vector( myhero->get_attack_range( ), myhero->get_attack_range( ) );
			draw_manager->add_image( circularrangeindicator_dark->texture, w2s - size / 2, size );
		}
	}*/

	/*if ( current_render.end_time < gametime->get_time( ) && current_render.start_time > gametime->get_time( ) )
	{
		if ( current_render.pos.is_valid( ) )
		{
			auto w2s = vector::zero;

			renderer->world_to_screen( current_render.pos, w2s );

			if ( w2s.is_valid( ) )
			{
				auto size = vector( target_enemy_precise->width, target_enemy_precise->height );
				draw_manager->add_image( target_enemy_precise->texture , w2s - size / 2, size );
			}
		}
	}*/

	static auto last_path_end = vector::zero;
	static auto last_path = std::vector<vector>{ };
	static auto next_click_time = -8.f;
	auto path_controller = myhero->get_path_controller();
	if (path_controller && path_controller->get_end_vec().is_valid() && !path_controller->is_dashing())
	{
		if (path_controller->get_end_vec() != last_path_end && last_path_end.distance(path_controller->get_end_vec()) >= 33.f &&
			path_controller->get_end_vec().distance(myhero) >= 33.f //stop
			&& gametime->get_time() > next_click_time
			)
		{
			auto last_contains = false;
			if (last_path.size() > 1)
			{
				for (auto i = 0; i < last_path.size() - 1; i++)
				{
					auto v_start = last_path[i],
						v_end = last_path[i + 1];

					auto project = path_controller->get_end_vec().project_on(v_start, v_end);
					if (project.is_on_segment && project.line_point.distance(path_controller->get_end_vec()) <= 120.f)
						last_contains = true;
				}
			}

			if (!last_contains)
			{
				last_path_end = path_controller->get_end_vec();
				last_path = path_controller->get_path();

				if (!orbwalker->none_mode())
					hud->get_hud_input_logic()->fake_click(last_path_end, hud_cursor_type::move_to);

				next_click_time = gametime->get_time() + A_::random_float( _green_delay_max->get_int( ) / 1000.f, _green_delay_min->get_int( ) / 1000.f );
			}
		}
	}
}

auto c_fake_cursor::load(TreeEntry* red_enabled, TreeEntry* red_extra_range, TreeEntry* green_enabled, TreeEntry* green_delay_max, TreeEntry* green_delay_min) -> void
{
	_red_enabled = red_enabled; _red_extra_range = red_extra_range;
	_green_enabled = green_enabled; _green_delay_max = green_delay_max;  _green_delay_min = green_delay_min;
	/*target_enemy_precise = draw_manager->load_texture_from_file( L"Lune_Utility\\target_enemy_precise.png" );
	target_ally_precise = draw_manager->load_texture_from_file( L"Lune_Utility\\target_ally_precise.png" );
	circularrangeindicator_dark = draw_manager->load_texture_from_file( L"Lune_Utility\\circularrangeindicator_dark.png" );

	if ( !target_enemy_precise || !target_enemy_precise->texture ||
		!target_ally_precise || !target_ally_precise->texture ||
		!circularrangeindicator_dark  || !circularrangeindicator_dark->texture )
	{
		console->print( "cursor_load failed" );
		return;
	}

	console->print( "cursor_load" );*/
	initialized = true;

	console->print("fake_cursor loaded %s|%s", _red_enabled->get_bool() ? "+" : "-", _green_enabled->get_bool() ? "+" : "-");
}