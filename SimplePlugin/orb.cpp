#include "../plugin_sdk/plugin_sdk.hpp"
#include "orb.h"

namespace orb
{
	void on_load()
	{

	}

	//
	
	//chase
	game_object_script orb_t, selected_t;
	vector magnet_pos;
	void spell_check()
	{
		orb_t = orbwalker->get_target();
		selected_t = target_selector->get_selected_target();
	}
	bool magnet_check;
	void magnet(bool hotkey)
	{
		if (!myhero->is_melee()) return;
		float aa_range = myhero->get_attack_range() + myhero->get_bounding_radius();
		auto target = orb_t;

		if (!target) return;

		if (selected_t && selected_t->is_valid_target() && selected_t->is_ai_hero()) //selected_t_range
			target = selected_t;

		if (!target) return;

		if ( target && target->is_valid_target() && target->is_ai_hero() && myhero->is_targetable() && !target->is_dead())
		{
			if (myhero->get_distance(target) > aa_range)
			{
				magnet_pos = target->get_position();
			}
			else
			{
				if (target->is_moving())
				{
					auto path = target->get_path_controller();
					if (path && !path->is_dashing())
					{
						auto path_array = path->get_path();
						magnet_pos = path_array[path->get_current_path_node()];
					}
				}
				else
				{
					if (myhero->get_distance(target->get_position()) < target->get_bounding_radius() + myhero->get_bounding_radius() + 10)
					{
						auto direction = (hud->get_hud_input_logic()->get_game_cursor_position() - myhero->get_position()).normalized();
						magnet_pos = target->get_position() + direction * (target->get_bounding_radius() + myhero->get_bounding_radius());
					}
					else
						magnet_pos = target->get_position();
				}
			}
		}
		else
		{
			magnet_pos = { 0,0,0 };
		}
		if (target && orbwalker->combo_mode() && hotkey && !target->is_dead() && target->is_valid_target( ) && target->is_ai_hero( ) && magnet_pos.is_valid( ) )
		{
			orbwalker->set_orbwalking_point(magnet_pos);
			magnet_check = true;
		}
		else if (magnet_check)
		{
			orbwalker->set_orbwalking_point({ 0,0,0 });
			magnet_check = false;
		}
	}
	
	void magnet_draw(bool hotkey)
	{
		if (magnet_pos.is_valid() && hotkey)
			draw_manager->add_line(myhero->get_position(), magnet_pos, MAKE_COLOR(255, 220, 220, 255), 1.333f);
	}

	//orb
	bool plant_check;
	void orb_plant(bool menu)
	{
		if (!menu) return;
		if (orbwalker->get_orb_state() == (orbwalker_state_flags::fast_lane_clear | orbwalker_state_flags::lane_clear))
		{
			auto object = entitylist->get_attackable_objects();

			object.erase(std::remove_if(object.begin(), object.end(), [](game_object_script x)
				{
					return !x || !x->is_valid( ) || !x->is_plant() || x->is_dead() || !myhero->is_in_auto_attack_range(x);
				}), object.end());

			std::sort(object.begin(), object.end(), [](game_object_script a, game_object_script b)
				{
					return myhero->get_distance(a) < myhero->get_distance(b);
				});

			if (!object.empty())
			{
				plant_check = true;
				if (orbwalker->lane_clear_mode())
				{
					orbwalker->set_orbwalking_target(object.front());
				}
			}
			else if (plant_check)
			{
				orbwalker->set_orbwalking_target(0);
				plant_check = false;

			}
		}
		
	}
}