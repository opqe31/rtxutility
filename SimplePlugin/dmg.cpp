#include "../plugin_sdk/plugin_sdk.hpp"
#include "dmg.h"

namespace dmg
{

	float calculate(float ad_dmg, float ap_dmg, float true_dmg, bool include_aa, game_object_script target)
	{

		damage_input input;
		input.raw_physical_damage = ad_dmg;
		input.raw_magical_damage = ap_dmg;
		input.raw_true_damage = true_dmg;

		float calculated = damagelib->calculate_damage_on_unit(myhero, target, &input);

		if (include_aa)
		{
			calculated += myhero->get_auto_attack_damage(target);
		}

		return calculated;
	}

	float get_damage(script_spell* spell, game_object_script target)
	{
		std::string spell_name = spell->name();

		if (spell_name == "EliseSpiderQCast")
		{
			float dmg[] = { 135.f, 190.0f, 245.0f, 300.0f, 355.0f };
			float base_magic_dmg = dmg[spell->level() - 1];
			return calculate(0.0f, base_magic_dmg, 0.0f, false, target);
		}

		return 0;


		// Default, not included in my dmglib
		//
		//return damagelib->get_spell_damage(myhero, target, spell->slot, false);
	}
}

//myhero->get_additional_attack_damage() bonus AD
//myhero->get_total_attack_damage() base AD
//myhero->get_base_attack_damage() »ùµA¹¥“ô
