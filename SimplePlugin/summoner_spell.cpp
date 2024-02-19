#include "../plugin_sdk/plugin_sdk.hpp"
#include "summoner_spell.h"
#include "dmg.h"
#include "champion.h"
#include "A_.h"
#include "exhaust_spelldb.h"

namespace summoner_spell
{
	script_spell* smite = nullptr;
	script_spell* exhaust = nullptr;
	script_spell* ghost = nullptr;
	script_spell* cleanse = nullptr;
	script_spell* barrier = nullptr;
	script_spell* heal = nullptr;
	script_spell* ignite = nullptr;
	script_spell* flash = nullptr;
	script_spell* snowball = nullptr;

	script_spell* q = nullptr;
	script_spell* w = nullptr;
	script_spell* e = nullptr;
	script_spell* r = nullptr;

	class Item {
	public:
		Item(ItemId id, float range) 
		{
			_range = range;
			_id = { id, static_cast<ItemId>( static_cast<int>( id ) + 220000 ) };
		}

		bool is_vaild() {
			auto slot = myhero->has_item( _id );
			if (slot != spellslot::invalid) {
				return true;
			}
			return false;
		}
		bool is_ready() 
		{
			for ( auto&& id : _id )
				if ( myhero->is_item_ready( id ) )
					return true;

			return false;
		}
		void cast(game_object_script target) {
			if (is_ready()) {
				auto slot = myhero->has_item(_id);
				if (slot != spellslot::invalid && target->get_distance(myhero) <= _range) {
					myhero->cast_spell(slot, target);
				}
			}
		}
		void cast() {
			if (is_ready()) {
				auto slot = myhero->has_item(_id);
				if (slot != spellslot::invalid) {
					myhero->cast_spell(slot);
				}
			}
		}
		void cast(vector vec) {
			if (is_ready()) {
				auto slot = myhero->has_item(_id);
				if (slot != spellslot::invalid) {
					myhero->cast_spell(slot, vec);
				}
			}
		}

	private:
		std::vector<ItemId> _id;
		float _range;
	};

	namespace all_spell
	{
		TreeEntry* only_show = nullptr;
		TreeEntry* use = nullptr;
	}
	namespace smite_spell
	{
		TreeEntry* mode = nullptr;
		TreeEntry* key = nullptr;
		TreeEntry* save = nullptr;
		TreeEntry* save_ks;
		TreeEntry* use_prediction;
		TreeEntry* draw_hpdamage_arrow;
		TreeEntry* draw_hpdamage_fill;
		//monster
		TreeEntry* smite_2 = nullptr;
		TreeTab* use_smite_black_list = nullptr;
		TreeEntry* smite_epic = nullptr;
		//TreeEntry* big_crab = nullptr;
		TreeEntry* small_crab = nullptr;
		TreeEntry* red_buff = nullptr;
		TreeEntry* cannon = nullptr;
		//TreeEntry* blue_buff = nullptr;
		TreeEntry* three_wolf = nullptr;
		TreeEntry* stone = nullptr;
		TreeEntry* six_bird = nullptr;
		TreeEntry* gromp = nullptr;

		//hotkey_0
		TreeEntry* smite_key = nullptr;
		//hotkey_1
		TreeEntry* smite_kill_key = nullptr;

		//monster
		TreeTab* hotkey_use_smite_key = nullptr;
		//TreeEntry* hotkey_use_baron = nullptr;
		TreeEntry* hotkey_use_epic = nullptr;
		//TreeEntry* hotkey_use_big_crab = nullptr;
		TreeEntry* hotkey_use_small_crab = nullptr;
		TreeEntry* hotkey_use_red_buff = nullptr;
		TreeEntry* hotkey_use_cannon = nullptr;
		//TreeEntry* hotkey_use_blue_buff = nullptr;
		TreeEntry* hotkey_use_three_wolf = nullptr;
		TreeEntry* hotkey_use_stone = nullptr;
		TreeEntry* hotkey_use_six_bird = nullptr;
		TreeEntry* hotkey_use_gromp = nullptr;
		TreeEntry* selector_position = nullptr;
		//hero
		TreeEntry* smite_hero_key = nullptr;
		TreeEntry* auto_smite_ks = nullptr;
		//seiga
		TreeEntry* smite_cart_tab;
		TreeEntry* smite_siege_minion = nullptr;
		//draw
		TreeEntry* state = nullptr;
		TreeEntry* damage;
		TreeEntry* range = nullptr;
		TreeEntry* range_style;
		TreeEntry* thickness = nullptr;
		TreeEntry* glow_ind_p = nullptr;
		TreeEntry* glow_ind_s = nullptr;
		TreeEntry* glow_out_p = nullptr;
		TreeEntry* glow_out_s = nullptr;
	}
	namespace exhaust_spell
	{
		TreeEntry* key = nullptr;
		TreeEntry* type = nullptr;

		std::map<champion_id, TreeEntry*> use_to;

		TreeEntry* exhaust = nullptr;
		TreeEntry* exhaust_for_ally;

		TreeEntry* exhaust_antigap;

		TreeEntry* xayah_e_count = nullptr;
		TreeEntry* twitch_e_count = nullptr;

		std::vector<TreeEntry*> config;
		std::vector<TreeEntry*> config_hp;

		std::map<champion_id, std::pair<TreeEntry*, TreeEntry*> > m_mode_hp;

		bool* b_enabled = new bool( false );
	}
	namespace ghost_spell
	{
		TreeEntry* use = nullptr;

		TreeEntry* chase = nullptr;
		TreeEntry* _logic_1 = nullptr;
		TreeEntry* hp_high = nullptr;
		TreeEntry* hp_low = nullptr;
		TreeEntry* mouse_dis = nullptr;
		TreeEntry* enemy_dis = nullptr;
		std::map<std::uint32_t, TreeEntry*> use_to;

		TreeEntry* flee = nullptr;
		TreeEntry* _logic_2 = nullptr;
		TreeEntry* hp_low_flee = nullptr;
		std::map<std::uint32_t, TreeEntry*> flee_to;
	}
	namespace cleanse_spell
	{
		TreeEntry* cleanse = nullptr;
		//1
		TreeEntry* cleanse_danger_level_1_alway_use = nullptr;
		TreeEntry* cleanse_danger_level_1_hp = nullptr;
		TreeEntry* cleanse_danger_level_1_enemy_range = nullptr;
		TreeEntry* cleanse_danger_level_1_enemy_count = nullptr;
		TreeEntry* cleanse_danger_level_1_delay = nullptr;
		TreeEntry* cleanse_danger_level_1_duration = nullptr;
		TreeEntry* cleanse_danger_level_1_combo = nullptr;
		TreeEntry* cleanse_danger_level_1_harass = nullptr;
		TreeEntry* cleanse_danger_level_1_other = nullptr;
		//2
		TreeEntry* cleanse_danger_level_2_alway_use = nullptr;
		TreeEntry* cleanse_danger_level_2_hp = nullptr;
		TreeEntry* cleanse_danger_level_2_enemy_range = nullptr;
		TreeEntry* cleanse_danger_level_2_enemy_count = nullptr;
		TreeEntry* cleanse_danger_level_2_delay = nullptr;
		TreeEntry* cleanse_danger_level_2_duration = nullptr;
		TreeEntry* cleanse_danger_level_2_combo = nullptr;
		TreeEntry* cleanse_danger_level_2_harass = nullptr;
		TreeEntry* cleanse_danger_level_2_other = nullptr;
		//3
		TreeEntry* cleanse_danger_level_3_alway_use = nullptr;
		TreeEntry* cleanse_danger_level_3_hp = nullptr;
		TreeEntry* cleanse_danger_level_3_enemy_range = nullptr;
		TreeEntry* cleanse_danger_level_3_enemy_count = nullptr;
		TreeEntry* cleanse_danger_level_3_delay = nullptr;
		TreeEntry* cleanse_danger_level_3_duration = nullptr;
		TreeEntry* cleanse_danger_level_3_combo = nullptr;
		TreeEntry* cleanse_danger_level_3_harass = nullptr;
		TreeEntry* cleanse_danger_level_3_other = nullptr;
		//4
		TreeEntry* cleanse_danger_level_4_alway_use = nullptr;
		TreeEntry* cleanse_danger_level_4_hp = nullptr;
		TreeEntry* cleanse_danger_level_4_enemy_range = nullptr;
		TreeEntry* cleanse_danger_level_4_enemy_count = nullptr;
		TreeEntry* cleanse_danger_level_4_delay = nullptr;
		TreeEntry* cleanse_danger_level_4_duration = nullptr;
		TreeEntry* cleanse_danger_level_4_combo = nullptr;
		TreeEntry* cleanse_danger_level_4_harass = nullptr;
		TreeEntry* cleanse_danger_level_4_other = nullptr;
		//CC
		TreeEntry* Exhaust_cleanse = nullptr;				//Ì“Èõ
		TreeEntry* Nasusw_cleanse = nullptr;				//NasosW
		TreeEntry* Ignite_cleanse = nullptr;				//üc»ð
		TreeEntry* Slow_cleanse = nullptr;					//œpËÙ
		TreeEntry* Snare_cleanse = nullptr;					//½ûåd
		TreeEntry* Stun_cleanse = nullptr;					//•žÑ£
		TreeEntry* Taunt_cleanse = nullptr;					//³°ÖS
		TreeEntry* Berserk_cleanse = nullptr;				//¿ñ±©
		TreeEntry* Polymorph_cleanse = nullptr;				//×ƒÐÎ
		TreeEntry* Fear_cleanse = nullptr;					//¿Ö‘Ö
		TreeEntry* Charm_cleanse = nullptr;					//÷È»ó
		TreeEntry* NearSight_cleanse = nullptr;				//½üÊÓ
		TreeEntry* Blind_cleanse = nullptr;					//Ã¤ÈË
		TreeEntry* Disarm_cleanse = nullptr;				//½â³ýÎä×°
		TreeEntry* Drowsy_cleanse = nullptr;				//»è»èÓûË¯
		TreeEntry* Asleep_cleanse = nullptr;				//Ë¯ÓX
		TreeEntry* Silence_cleanse = nullptr;				//³ÁÄ¬
		TreeEntry* qss = nullptr;
		//CC
		TreeEntry* Slow_qss = nullptr;					//œpËÙ
		TreeEntry* Snare_qss = nullptr;					//½ûåd
		TreeEntry* Stun_qss = nullptr;					//•žÑ£
		TreeEntry* Taunt_qss = nullptr;					//³°ÖS
		TreeEntry* Nasusw_qss = nullptr;				//NasosW
		TreeEntry* Berserk_qss = nullptr;				//¿ñ±©
		TreeEntry* Polymorph_qss = nullptr;				//×ƒÐÎ
		TreeEntry* Fear_qss = nullptr;					//¿Ö‘Ö
		TreeEntry* Charm_qss = nullptr;					//÷È»ó
		TreeEntry* Suppression_qss = nullptr;			//‰ºÖÆ
		TreeEntry* NearSight_qss = nullptr;				//½üÊÓ
		TreeEntry* Blind_qss = nullptr;					//Ã¤ÈË
		TreeEntry* Disarm_qss = nullptr;				//½â³ýÎä×°
		TreeEntry* Drowsy_qss = nullptr;				//»è»èÓûË¯
		TreeEntry* Asleep_qss = nullptr;				//Ë¯ÓX
		TreeEntry* Silence_qss = nullptr;				//³ÁÄ¬

		TreeEntry* mikael = nullptr;

		TreeEntry* mikael_heal;
		TreeEntry* mikael_heal_hp;
		std::map<champion_id, TreeEntry*> mikael_heal_on;

		std::map<std::uint32_t, TreeEntry*> mikael_use_on;
		//CC
		TreeEntry* Slow_mikael = nullptr;					//œpËÙ
		TreeEntry* Snare_mikael = nullptr;					//½ûåd
		TreeEntry* Stun_mikael = nullptr;					//•žÑ£
		TreeEntry* Taunt_mikael = nullptr;					//³°ÖS
		TreeEntry* Berserk_mikael = nullptr;				//¿ñ±©
		TreeEntry* Polymorph_mikael = nullptr;				//×ƒÐÎ
		TreeEntry* Fear_mikael = nullptr;					//¿Ö‘Ö
		TreeEntry* Charm_mikael = nullptr;					//÷È»ó
		TreeEntry* Drowsy_mikael = nullptr;					//»è»èÓûË¯
		TreeEntry* Asleep_mikael = nullptr;					//Ë¯ÓX
		TreeEntry* Silence_mikael = nullptr;				//³ÁÄ¬

	}
	namespace use_heal
	{
		TreeEntry* heal = nullptr;
		TreeEntry* level = nullptr;
		TreeEntry* hp = nullptr;
		TreeEntry* calc_hp = nullptr;

		std::map<std::uint32_t, TreeEntry*> ally;
		TreeEntry* ally_hp = nullptr;
		TreeEntry* ally_calc_hp = nullptr;

		TreeEntry* dont_use_dead = nullptr;
		TreeEntry* c = nullptr;
		TreeEntry* harass = nullptr;
		TreeEntry* other = nullptr;
		TreeEntry* rang = nullptr;
		TreeEntry* enemy = nullptr;
	}
	namespace use_barrier
	{
		TreeEntry* barrier = nullptr;
		TreeEntry* level = nullptr;
		TreeEntry* hp = nullptr;
		TreeEntry* calc_hp = nullptr;

		TreeEntry* dont_use_dead = nullptr;
		TreeEntry* c = nullptr;
		TreeEntry* harass = nullptr;
		TreeEntry* other = nullptr;
		TreeEntry* rang = nullptr;
		TreeEntry* enemy = nullptr;
	}
	namespace use_ignite
	{
		TreeEntry* ignite = nullptr;
		TreeEntry* key = nullptr;
		TreeEntry* type = nullptr;

		TreeEntry* dont_over_kill = nullptr;
		TreeEntry* c = nullptr;
		TreeEntry* harass = nullptr;
		TreeEntry* other = nullptr;

		TreeEntry* rang = nullptr;
		TreeEntry* enemy = nullptr;

		std::map<std::uint32_t, TreeEntry*> use_to;
	}
	namespace use_flash
	{
		TreeEntry* use_flash = nullptr;

		TreeEntry* anti_flash = nullptr;
		TreeEntry* anti_gapclose = nullptr;

		std::map<std::uint32_t, TreeEntry*> use_to;
		std::map<std::uint32_t, TreeEntry*> anti_range;
		std::map<std::uint32_t, TreeEntry*> anti_hp;
	}
	namespace use_snowball
	{
		TreeEntry* semi_key = nullptr;
		TreeEntry* use = nullptr;
		TreeEntry* range = nullptr;
		TreeEntry* hp = nullptr;
		std::map<std::uint32_t, TreeEntry*> use_to;
	}

	void summoner_spell_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		all_spell::use = summoner->add_hotkey("use", "Use summoner spell", TreeHotkeyMode::Toggle, 0, false);
		all_spell::only_show = summoner->add_checkbox("show", "Show all spell", false);
	}

	void spell_check()
	{
		//ÖØ“ô
		//SummonerSmite, S5_SummonerSmitePlayerGanker, SummonerSmiteAvatarOffensive, SummonerSmiteAvatarUtility, SummonerSmiteAvatarDefensive 
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name().find("SummonerSmite") != std::string::npos)
			smite = plugin_sdk->register_spell(spellslot::summoner1, 500.f);
		if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name().find("SummonerSmite") != std::string::npos)
			smite = plugin_sdk->register_spell(spellslot::summoner2, 500.f);

		//Ì“Èõ
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerExhaust"))
			exhaust = plugin_sdk->register_spell(spellslot::summoner1, 650.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerExhaust"))
			exhaust = plugin_sdk->register_spell(spellslot::summoner2, 650.f);

		//¹í²½
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerHaste"))
			ghost = plugin_sdk->register_spell(spellslot::summoner1, 650.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerHaste"))
			ghost = plugin_sdk->register_spell(spellslot::summoner2, 650.f);

		//œQ»¯
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerBoost"))
			cleanse = plugin_sdk->register_spell(spellslot::summoner1, 0.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerBoost"))
			cleanse = plugin_sdk->register_spell(spellslot::summoner2, 0.f);

		//¹â¶Ü
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerBarrier"))
			barrier = plugin_sdk->register_spell(spellslot::summoner1, 0.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerBarrier"))
			barrier = plugin_sdk->register_spell(spellslot::summoner2, 0.f);

		//ÖÎÁÆ
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerHeal"))
			heal = plugin_sdk->register_spell(spellslot::summoner1, 0.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerHeal"))
			heal = plugin_sdk->register_spell(spellslot::summoner2, 0.f);

		//dot
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerDot"))
			ignite = plugin_sdk->register_spell(spellslot::summoner1, 0.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerDot"))
			ignite = plugin_sdk->register_spell(spellslot::summoner2, 0.f);

		//flash
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
			flash = plugin_sdk->register_spell(spellslot::summoner1, 0.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
			flash = plugin_sdk->register_spell(spellslot::summoner2, 0.f);

		//smowball
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerSnowball"))
			snowball = plugin_sdk->register_spell(spellslot::summoner1, 0.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerSnowball"))
			snowball = plugin_sdk->register_spell(spellslot::summoner2, 0.f);

		Item qss_1(ItemId::Quicksilver_Sash, 0);
		Item qss_2(ItemId::Mercurial_Scimitar, 0);
		Item qss_3(ItemId::Silvermere_Dawn, 0);
		Item mikael(ItemId::Mikaels_Blessing, 650);
	}

#pragma region smite
	
	bool smite_valid_monster(game_object_script x)
	{
		auto name = x->get_name();
		std::transform(name.begin(), name.end(), name.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if ( !A_::is_smite_monster( name ) )
			return false;

		if (smite_spell::mode->get_int() == 0)
		{
			if (smite_spell::smite_epic->get_bool() && A_::is_epic_monster(name))
			{
				return true;
			}
			else if (smite_spell::small_crab->get_bool() && (name.find("sru_crab") != std::string::npos))
			{
				return true;
			}
			else if (smite_spell::red_buff->get_bool() && A_::is_jungle_buff(name))
			{
				return true;
			}
			else if (smite_spell::three_wolf->get_bool() && (name.find("sru_murkwolf") != std::string::npos && name.find("mini") == std::string::npos))
			{
				return true;
			}
			else if (smite_spell::stone->get_bool() && (name.find("sru_krug") != std::string::npos && name.find("mini") == std::string::npos))
			{
				return true;
			}
			else if (smite_spell::six_bird->get_bool() && (name.find("sru_razorbeak") != std::string::npos && name.find("mini") == std::string::npos)) //
			{
				return true;
			}
			else if (smite_spell::gromp->get_bool() && (name.find("sru_gromp") != std::string::npos))
			{
				return true;
			}
		}
		else if (smite_spell::mode->get_int() == 1 && (myhero->count_enemies_in_range(1250) > 0 || smite_spell::smite_kill_key->get_bool())) //&& x->is_epic_monster( )
			return true;

		return false;
	};

	vector get_object_position(game_object_script object)
	{
		auto object_position = object->get_position();
		if (smite_spell::selector_position->get_int() == 0 || object->is_lane_minion())
			return object_position;

		auto name = object->get_name();
		std::transform(name.begin(), name.end(), name.begin(),
			[](unsigned char c) { return std::tolower(c); });

		auto v_camp_positions = std::vector<vector>{ };
		if (name.find("sru_riftherald") != std::string::npos || name.find("sru_baron") != std::string::npos)
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Herlad));
		else if (name.find("sru_dragon") != std::string::npos)
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Dragon));
		else if (name.find("sru_red") != std::string::npos)
		{
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Red_Chaos));
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Red_Order));
		}
		else if (name.find("sru_blue") != std::string::npos)
		{
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Blue_Chaos));
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Blue_Order));
		}
		else if (name.find("sru_crab") != std::string::npos)
		{
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Crab_Bottom));
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Crab_Top));
		}
		else if (name.find("sru_murkwolf") != std::string::npos && name.find("mini") == std::string::npos)
		{
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Wolves_Chaos));
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Wolves_Order));
		}
		else if (name.find("sru_razor") != std::string::npos && name.find("mini") == std::string::npos)
		{
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Raptors_Chaos));
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Raptors_Order));
		}
		else if (name.find("sru_krug") != std::string::npos && name.find("mini") == std::string::npos)
		{
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Krugs_Chaos));
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Krugs_Order));
		}
		else if (name.find("sru_gromp") != std::string::npos)
		{
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Gromp_Chaos));
			v_camp_positions.push_back(camp_manager->get_camp_position(neutral_camp_id::Gromp_Order));
		}
		if (!v_camp_positions.empty())
		{
			std::sort(v_camp_positions.begin(), v_camp_positions.end(), [object_position](vector a, vector b)
				{
					return a.distance(object_position) < b.distance(object_position);
				});
			return v_camp_positions.front();
		}

		return object_position;
	}

	auto smite_semi_key_delay = -8.f;
	void smite_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_smite = summoner->add_tab("smite_menu", "Smite");
		if (summoner_smite)
		{
			summoner_smite->add_separator("smite_1", " - - Smite - - ");
			smite_spell::key = summoner_smite->add_hotkey("use_key", "Use smite key", TreeHotkeyMode::Toggle, 'N', true);
			summoner_smite->set_assigned_active(smite_spell::key);
			smite_spell::mode = summoner_smite->add_combobox("mode", "Smite mode", { {"Custom",nullptr},{"Near enemy",nullptr } }, 0, true);
			smite_spell::save = summoner_smite->add_checkbox("save", "Save smite (?)", true, true);
			
			smite_spell::use_prediction = summoner_smite->add_checkbox( "use_prediction", "Use prediction", true, true );

			smite_spell::save->set_tooltip("Save smite if any epic is alive or about to spawn ( don't smite monster )");
			

			auto db_summoner_smite = database->get_spell_by_hash( spell_hash( "SummonerSmite" ) );
			if ( db_summoner_smite )
			{
				auto smite_texture = db_summoner_smite->get_icon_texture_by_index( 0 );
				if ( smite_texture )
					summoner_smite->set_texture( smite_texture );
			}

			smite_spell::smite_2 = summoner_smite->add_separator("smite_2", " - - Monster - - ");
			smite_spell::use_smite_black_list = summoner_smite->add_tab("use_smite_black_list", "Monster whitelist");
			{
				smite_spell::smite_epic = smite_spell::use_smite_black_list->add_checkbox("smite_epic", "Objective", true, true);
				smite_spell::small_crab = smite_spell::use_smite_black_list->add_checkbox("smite_small_crab", "Scuttle Crab", true, true);
				smite_spell::red_buff = smite_spell::use_smite_black_list->add_checkbox("smite_red_buff", "Blue/Red Buff", true, true);
				smite_spell::three_wolf = smite_spell::use_smite_black_list->add_checkbox("smite_three_wolf", "Murkwolf", true, true);
				smite_spell::stone = smite_spell::use_smite_black_list->add_checkbox("smite_stone", "Krug", true, true);
				smite_spell::six_bird = smite_spell::use_smite_black_list->add_checkbox("smite_six_bird", "Raptor", true, true);
				smite_spell::gromp = smite_spell::use_smite_black_list->add_checkbox("smite_gromp", "Gromp", true, true);

				smite_spell::cannon = smite_spell::use_smite_black_list->add_checkbox("smite_cannon", "Siege minion", false, true);
			}

			smite_spell::smite_kill_key = summoner_smite->add_hotkey("smite_kill_key", "Semi smite monster(can kill)", TreeHotkeyMode::Hold, 'F', false);
			smite_spell::smite_key = summoner_smite->add_hotkey("chose_key", "Monster selector key", TreeHotkeyMode::Hold, 'F', false);
			smite_spell::hotkey_use_smite_key = summoner_smite->add_tab("dont_use_smite_key", "  ^- Use selector hotkey monster");
			{
				smite_spell::hotkey_use_epic = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_dragon", "Objective", false, true);
				smite_spell::hotkey_use_small_crab = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_small_crab", "Scuttle Crab", true, true);
				smite_spell::hotkey_use_red_buff = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_red_buff", "Blue/Red Buff", true, true);
				smite_spell::hotkey_use_three_wolf = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_three_wolf", "Murkwolf", true, true);
				smite_spell::hotkey_use_stone = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_stone", "Krug", true, true);
				smite_spell::hotkey_use_six_bird = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_six_bird", "Raptor", true, true);
				smite_spell::hotkey_use_gromp = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_gromp", "Gromp", true, true);

				smite_spell::hotkey_use_cannon = smite_spell::hotkey_use_smite_key->add_checkbox("hotkey_smite_cannon", "Siege minion", false, true);
			}
			smite_spell::selector_position = summoner_smite->add_combobox("selector_position", "Selector position", { {"Monster", nullptr}, {"Camp", nullptr} }, 0); //+3

			summoner_smite->add_separator("smite_3", " - - Smite Hero - - ");
			smite_spell::smite_hero_key = summoner_smite->add_hotkey("smite_hero_key", "Semi smite enemy", TreeHotkeyMode::Hold, 'F', false);
			smite_spell::auto_smite_ks = summoner_smite->add_checkbox("auto_smite_ks", "Auto smite ks", true, true);
			smite_spell::save_ks = summoner_smite->add_checkbox( "save_ks", "Save smite ks (?)", true, true );
			smite_spell::save_ks->set_tooltip( "Save smite if any epic is alive or about to spawn ( don't killsteal on enemy hero )" );

			//summoner_smite->add_separator("smite_4", " - - Warning - - ");
			smite_spell::smite_cart_tab = summoner_smite->add_separator("smite_4", " - - Smite Siege Minion (?) - - ");
			smite_spell::smite_cart_tab->set_tooltip("  Using this feature may make your teammates AFK  ");
			smite_spell::smite_siege_minion = summoner_smite->add_checkbox("smite_siege_minion", "Smite siege minion", false, true);

			summoner_smite->add_separator("smite_6", " - - Draw - - ");
			smite_spell::draw_hpdamage_arrow = summoner_smite->add_checkbox( "draw_hpdamage_arrow", "Draw healthbar damage arrow", true, true );
			smite_spell::draw_hpdamage_fill = summoner_smite->add_checkbox( "draw_hpdamage_fill", "Draw healthbar damage", true, true );
			smite_spell::state = summoner_smite->add_checkbox("state", "Draw smite state", true);
			smite_spell::range_style = summoner_smite->add_combobox( "range_style", "Range style", { { "Circle", nullptr }, { "Corner box", nullptr } }, 0 );
			smite_spell::range = summoner_smite->add_slider("range", "Draw smite range (100 = Disable)", 600, 100, 600);
			smite_spell::damage = summoner_smite->add_checkbox("damage", "Draw smite damage", true, true);
			smite_spell::thickness = summoner_smite->add_slider("thickness", "Thickness", 2, 1, 5);
			smite_spell::glow_ind_s = summoner_smite->add_slider("glow_ind_s", "Glow inside size", 40, 1, 100);
			smite_spell::glow_ind_p = summoner_smite->add_slider("glow_ind_p", "Glow inside power", 50, 1, 100);
			smite_spell::glow_out_s = summoner_smite->add_slider("glow_out_s", "Glow outside size", 100, 1, 100);
			smite_spell::glow_out_p = summoner_smite->add_slider("glow_out_p", "Glow outside power", 50, 1, 100);

		}
	}
	
	void cast_smite_monster(game_object_script target)
	{
		float dragon = camp_manager->get_camp_respawn_time(neutral_camp_id::Dragon) - gametime->get_time();
		float baron = camp_manager->get_camp_respawn_time(neutral_camp_id::Baron) - gametime->get_time();
		float herlad = camp_manager->get_camp_respawn_time(neutral_camp_id::Herlad) - gametime->get_time();
#ifdef __TEST
		//console->print( "dragon: %.0f | baron: %.0f | herald: %.0f | charges: %d", dragon, baron, herlad, smite->ammo( ) );
#endif
		auto name = target->get_name();
		std::transform(name.begin(), name.end(), name.begin(),
			[](unsigned char c) { return std::tolower(c); });

		float cd = smite->handle()->cooldownex_end() - gametime->get_time();
		if ((herlad < 75 || baron < 75 || dragon < 75) && smite->ammo() == 1 && cd > 15 && 
			( ( smite_spell::save->get_bool( ) && !target->is_ai_hero( ) ) || ( smite_spell::save_ks->get_bool( ) && target->is_ai_hero( ) ) ) &&
			!A_::is_epic_monster(name))
			return;

		if (myhero->get_distance(target) > smite->range() + myhero->get_bounding_radius( ) + target->get_bounding_radius( ))
			return;

		std::vector< buff_instance_script> buffs = { myhero->get_buff( buff_hash( "SmiteDamageTracker" ) ), myhero->get_buff( buff_hash( "SmiteDamageTrackerStalker" ) ), myhero->get_buff( buff_hash( "SmiteDamageTrackerAvatar" ) ) };
		std::sort( std::begin( buffs ), std::end( buffs ), [ ]( buff_instance_script b1, buff_instance_script b2 ) {
			return b1 && b1->is_valid( ) && ( !b2 || !b2->is_valid( ) );
		} );
		auto&& buff = buffs.front( );

		if (buff && buff->is_valid() && buff->is_alive())
		{
			float smite_damage = (buff->get_hash_name( ) == buff_hash( "SmiteDamageTrackerStalker" ) && target->get_bounding_radius( ) == 65.f ) ? 500.f + target->get_max_health( ) * 0.1f : static_cast<float>( buff->get_count( ) );

			auto monster_health = smite_spell::use_prediction->get_bool( ) ? health_prediction->get_health_prediction( target, ping->get_ping( ) / 2000.f, 0.f ) : target->get_health( );
			if ( monster_health <= smite_damage)
			{
				myhero->cast_spell( smite->get_slot(), target );
				return;
			}

			//if (!q)
			//	q = plugin_sdk->register_spell(spellslot::q, 0);

			//if (!w)
			//	w = plugin_sdk->register_spell(spellslot::w, 0);

			//if (!e)
			//	e = plugin_sdk->register_spell(spellslot::e, 0);

			//if (!r)
			//	r = plugin_sdk->register_spell(spellslot::r, 0);

			//q->set_spell_lock(false);
			//w->set_spell_lock(false);
			//e->set_spell_lock(false);
			//r->set_spell_lock(false);

			//auto d = myhero->get_distance(target);

			//if (myhero->get_champion() == champion_id::Diana && myhero->get_spell_state(spellslot::q) == spell_state::Ready)
			//{
			//	auto t = 0.25f + d / 1900.0f;
			//	auto dmg = smite_damage + q->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, q->get_damage(target));

			//		q->cast(target->get_position());
			//		cast_loop = gametime->get_time() + 0.2f;
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Elise && myhero->get_spell_state(spellslot::q) == spell_state::Ready && myhero->has_buff(buff_hash("EliseR")))
			//{
			//	auto t = 0.22f;
			//	auto dmg = smite_damage + dmg::get_damage(q, target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, dmg::get_damage(q, target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		q->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Evelynn && myhero->get_spell_state(spellslot::e) == spell_state::Ready)
			//{
			//	auto t = 0.22f;
			//	auto dmg = smite_damage + e->get_damage(target);

			//	if (monster_health <= dmg && myhero->get_distance(target) < 210)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, e->get_damage(target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		e->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Hecarim && myhero->get_spell_state(spellslot::q) == spell_state::Ready)
			//{
			//	auto t = 0.0f;
			//	auto dmg = smite_damage + q->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, q->get_damage(target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		q->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::JarvanIV && myhero->get_spell_state(spellslot::q) == spell_state::Ready)
			//{
			//	auto t = 0.4f;
			//	auto dmg = smite_damage + q->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, q->get_damage(target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		q->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Khazix && myhero->get_spell_state(spellslot::q) == spell_state::Ready)
			//{
			//	auto t = 0.22f;
			//	auto dmg = smite_damage + q->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, q->get_damage(target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		q->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Lillia && myhero->get_spell_state(spellslot::q) == spell_state::Ready)
			//{
			//	auto t = 0.15;
			//	auto dmg = smite_damage + q->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, q->get_damage(target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		q->cast();
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Nunu && myhero->get_spell_state(spellslot::q) == spell_state::Ready)
			//{
			//	auto t = 0.22;
			//	auto dmg = smite_damage + q->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, q->get_damage(target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		q->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Olaf && myhero->get_spell_state(spellslot::e) == spell_state::Ready)
			//{
			//	auto t = 0.23;
			//	auto dmg = smite_damage + e->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, e->get_damage(target));

			//		cast_loop = gametime->get_time() + 0.2f;
			//		e->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::Sylas && myhero->get_spell_state(spellslot::w) == spell_state::Ready)
			//{
			//	auto t = 0.22;
			//	auto dmg = smite_damage + w->get_damage(target);

			//	if (monster_health <= dmg)
			//	{
			//		cast_loop = gametime->get_time() + 0.2f;
			//		//myhero->print_chat(1, "total: %f, spell dmg: %f", dmg, w->get_damage(target));

			//		w->cast(target);
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
			//if (myhero->get_champion() == champion_id::LeeSin && myhero->get_spell_state(spellslot::e) == spell_state::Ready)
			//{
			//	auto t = 0.22;
			//	auto dmg = smite_damage + e->get_damage(target);

			//	if (monster_health <= dmg && e->name_hash() == spell_hash("BlindMonkEOne"))
			//	{
			//		cast_loop = gametime->get_time() + 0.2f;
			//		e->cast();
			//		scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
			//			{
			//				smite->cast(target);
			//			});
			//	}
			//}
		}


		if (buff && buff->is_valid() && buff->is_alive() )
		{
			float smite_damage = (buff == myhero->get_buff(buff_hash("SmiteDamageTrackerStalker")) && target->get_bounding_radius() == 65.f) ? 500.f + target->get_max_health() * 0.1f : buff->get_count();
			auto monster_health = smite_spell::use_prediction->get_bool( ) ? health_prediction->get_health_prediction( target, ping->get_ping( ) / 2000.f, 0.f ) : target->get_health( );
			if ( monster_health <= smite_damage)
			{
				myhero->cast_spell( smite->slot, target );
				//cast_loop = gametime->get_time() + 0.2f;
			}

			/*if ( !q )
			{
				q = plugin_sdk->register_spell( spellslot::q, 0 );
				q->set_spell_lock( false );
			}

			auto d = myhero->get_distance(target);
			if (myhero->get_champion() == champion_id::LeeSin && myhero->get_spell_state(spellslot::q) == spell_state::Ready)
			{
				auto t = d / (1400.f + myhero->get_move_speed());
				auto dmg = smite_damage + q->get_damage(target);

				if ( monster_health <= dmg && q->name_hash() == spell_hash("BlindMonkQTwo"))
				{
					cast_loop = gametime->get_time() + 0.2f;
					q->cast(target);
					scheduler->delay_action(t - ping->get_ping() / 1000.0f, [target]
						{
							smite->cast(target);
						});
				}
			}*/
		}
	}

	void smite_monster()
	{
		if (!smite || !smite->is_ready() || !smite_spell::key->get_bool())
			return;

		for (auto&& object : entitylist->get_jugnle_mobs_minions())
		{
			if (object->is_valid() && !object->is_dead() && object->is_visible() && !object->has_buff(buff_hash("KindredRNoDeathBuff")) && smite_valid_monster(object))
				cast_smite_monster(object);
		}

		if ((smite_spell::mode->get_int() == 1 && smite_spell::smite_siege_minion->get_bool()) ||
			(smite_spell::mode->get_int() == 0 && smite_spell::cannon->get_bool()))
		{
			for (auto&& minions : entitylist->get_enemy_minions())
				if (minions->is_valid() && minions->get_bounding_radius() == 65.f)
					cast_smite_monster(minions);
		}

		//

		//Î²µ¶
		auto enemies = entitylist->get_enemy_heroes();
		enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
			{
				return x->is_dead() || !x->is_valid_target(smite->range() + myhero->get_bounding_radius( ) + x->get_bounding_radius( ));
			}), enemies.end());

		if (enemies.empty() || !smite_spell::auto_smite_ks->get_bool())
			return;

		if (!myhero->has_buff(buff_hash("SmiteDamageTrackerStalker")) && !myhero->has_buff(buff_hash("SmiteDamageTrackerAvatar")))
			return;

		for (auto&& enemy : enemies)
		{
			auto smite_damage_hero = 80 + 80 / 17 * (myhero->get_level() - 1);
			auto monster_health = smite_spell::use_prediction->get_bool( ) ? health_prediction->get_health_prediction( enemy, ping->get_ping( ) / 2000.f, 0.f ) : enemy->get_health( );
			if ( monster_health > smite_damage_hero)
				continue;

			myhero->cast_spell( smite->get_slot( ), enemy );
			return;
		}
	}

	void semi_smite()
	{
		if (!smite)
			return;

		//ÈÈ¼ü
		//smite whitelist modify
		if (smite_semi_key_delay < gametime->get_time() && smite_spell::smite_key->get_bool() && smite_spell::mode->get_int() == 0)
		{
			for (auto&& object : entitylist->get_jugnle_mobs_minions())
			{
				if (!object->is_valid())
					continue;

				if (get_object_position(object).distance(hud->get_hud_input_logic()->get_game_cursor_position()) > smite->range())
					continue;

				auto name = object->get_name();
				std::transform(name.begin(), name.end(), name.begin(),
					[](unsigned char c) { return std::tolower(c); });

				//Ð¡ýˆ
				if (smite_spell::hotkey_use_epic->get_bool() && A_::is_epic_monster(name))
				{
					smite_spell::smite_epic->set_bool(!smite_spell::smite_epic->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
				//ºÓÐ·
				else if (smite_spell::hotkey_use_small_crab->get_bool() && name.find("sru_crab") != std::string::npos)
				{
					smite_spell::small_crab->set_bool(!smite_spell::small_crab->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
				//¼t
				else if (smite_spell::hotkey_use_red_buff->get_bool() && A_::is_jungle_buff(name))
				{
					smite_spell::red_buff->set_bool(!smite_spell::red_buff->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
				//ÈýÀÇ
				else if (smite_spell::hotkey_use_three_wolf->get_bool() && name.find("sru_murkwolf") != std::string::npos && name.find("mini") == std::string::npos)
				{
					smite_spell::three_wolf->set_bool(!smite_spell::three_wolf->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
				//Ê¯î^
				else if (smite_spell::hotkey_use_stone->get_bool() && name.find("sru_krug") != std::string::npos && name.find("mini") == std::string::npos)
				{
					smite_spell::stone->set_bool(!smite_spell::stone->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
				//ÁùøB
				else if (smite_spell::hotkey_use_six_bird->get_bool() && name.find("sru_razor") != std::string::npos && name.find("mini") == std::string::npos)
				{
					smite_spell::six_bird->set_bool(!smite_spell::six_bird->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
				//´óó¸òÜ
				else if (smite_spell::hotkey_use_gromp->get_bool() && name.find("sru_gromp") != std::string::npos)
				{
					smite_spell::gromp->set_bool(!smite_spell::gromp->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
			}

			for (auto&& minions : entitylist->get_enemy_minions())
			{
				if (!minions->is_valid() || minions->get_bounding_radius() != 65.f || !minions->is_lane_minion())
					continue;

				if (minions->get_distance(hud->get_hud_input_logic()->get_game_cursor_position()) > smite->range())
					continue;

				if (smite_spell::hotkey_use_cannon->get_bool())
				{
					smite_spell::cannon->set_bool(!smite_spell::cannon->get_bool());
					smite_semi_key_delay = gametime->get_time() + 0.3f;
				}
			}
		}

		if (!smite->is_ready())
			return;

		//°ëÊÖ¶¯³Í½äµÐÈË
		//smite hero assist

		auto enemies = entitylist->get_enemy_heroes( );
		enemies.erase( std::remove_if( enemies.begin( ), enemies.end( ), [ ]( game_object_script x ) {
			return !x || !x->is_valid( ) || !x->is_valid_target( smite->range( ) + myhero->get_bounding_radius( ) + x->get_bounding_radius( ) ) || x->has_buff_type( buff_type::SpellShield ) ||
				x->is_invulnerable( ) || x->has_buff( { buff_hash( "KindredRNoDeathBuff" ), buff_hash( "UndyingRage" ), buff_hash( "ChronoRevive" ), buff_hash( "ChronoShift" ) } );
		} ), enemies.end( ) );

		if ( enemies.empty( ) )
			return;

		auto target = target_selector->get_target( enemies, damage_type::true_dmg );
		if (target != nullptr && smite_spell::smite_hero_key->get_bool() &&
			(myhero->get_buff(buff_hash("SmiteDamageTrackerStalker")) || myhero->get_buff(buff_hash("SmiteDamageTrackerAvatar"))))
		{
			float dragon = camp_manager->get_camp_respawn_time(neutral_camp_id::Dragon) - gametime->get_time();
			float baron = camp_manager->get_camp_respawn_time(neutral_camp_id::Baron) - gametime->get_time();
			float herlad = camp_manager->get_camp_respawn_time(neutral_camp_id::Herlad) - gametime->get_time();

			float cd = smite->handle()->cooldownex_end() - gametime->get_time();
			if ((herlad < 75 || baron < 75 || dragon < 75) && smite->ammo() == 1 && cd > 15 )
				return;

			myhero->cast_spell( smite->slot, target );
			return;
		}

	}

	//draw
	void smite_circle(game_object_script object, bool cancel)
	{
		auto position = get_object_position(object);

		auto damage = 600.f;
		std::vector< buff_instance_script> buffs = { myhero->get_buff( buff_hash( "SmiteDamageTracker" ) ), myhero->get_buff( buff_hash( "SmiteDamageTrackerStalker" ) ), myhero->get_buff( buff_hash( "SmiteDamageTrackerAvatar" ) ) };
		std::sort( std::begin( buffs ), std::end( buffs ), [ ]( buff_instance_script b1, buff_instance_script b2 ) {
			return b1 && b1->is_valid( ) && ( !b2 || !b2->is_valid( ) );
		} );
		auto&& buff = buffs.front( );
		if (buff && buff->is_valid() && buff->is_alive())
			damage = (buff == myhero->get_buff(buff_hash("SmiteDamageTrackerStalker")) && object->get_bounding_radius() == 65.f ) ? 500.f + object->get_max_health() * 0.1f : buff->get_count();

		auto range = smite_spell::range->get_int() == 600 ? smite->range( ) + myhero->get_bounding_radius( ) + object->get_bounding_radius( ) : smite_spell::range->get_int( );
		auto thickness = static_cast< float >(  smite_spell::thickness->get_int( ) );

		auto glow_ind_p = smite_spell::glow_ind_p->get_int() / 100.f,
			glow_ind_s = smite_spell::glow_ind_s->get_int() / 100.f,
			glow_out_p = smite_spell::glow_out_p->get_int() / 100.f,
			glow_out_s = smite_spell::glow_out_s->get_int() / 100.f;
		if (cancel)
		{
			if ( !smite->is_ready( ) ) //not ready
			{
				if ( smite_spell::range_style->get_int( ) == 0 )
					draw_manager->add_circle_with_glow( position, MAKE_COLOR( 223, 230, 233, 50 ), range, thickness, glow_data( glow_ind_s, glow_ind_p, glow_out_s, glow_out_p ) );
				else if ( smite_spell::range_style->get_int( ) == 1 )
				{
					auto min = object->get_bbox_min( ),
						max = object->get_bbox_max( );
					renderer->world_to_screen( min, min );
					renderer->world_to_screen( max, max );
					A_::draw_corner_box( min, max, MAKE_COLOR( 223, 230, 233, 50 ), false );
				}
			}
			else
			{
				auto min = object->get_bbox_min( ),
					max = object->get_bbox_max( );
				renderer->world_to_screen( min, min );
				renderer->world_to_screen( max, max );

				if (smite_spell::damage->get_bool())
				{
					if ( smite_spell::range_style->get_int( ) == 0 )
					{
						static auto size = vector( myhero->get_bounding_radius( ) * 0.75f, myhero->get_bounding_radius( ) * 0.75f );
						//static auto bar_height = size.y / 5.f;

						auto w2s = vector::zero;
						renderer->world_to_screen( position - vector( object->get_bounding_radius( ) * 2.f, object->get_bounding_radius( ) * 2.f ), w2s );

						w2s = w2s - ( size / 2 );
						draw_manager->add_image( smite->handle( )->get_icon_texture( ), w2s, size );

						draw_manager->add_filled_rect( w2s + vector( 0, size.y ), w2s + vector( size.x, size.x + size.x / 5 ), MAKE_COLOR( 220, 216, 192, 255 ) );

						auto damage_pcent = std::min( 3.0f, damage / object->get_health( ) );
						auto color = A_::get_color( damage_pcent * 100.f );

						draw_manager->add_filled_rect( w2s + vector( 0, size.y ), w2s + vector( size.x * std::min( 1.f, std::max( 0.1f, damage_pcent ) ), size.y + ( size.y / 5 ) ),
							damage_pcent >= 0.75 ? A_::animate_color( color, 120, 255 ) : color );

						auto text_size = draw_manager->calc_text_size( static_cast<int>( size.x * 0.35f ), "%.0f%%", damage_pcent * 100.f );
						draw_manager->add_text_on_screen( w2s + vector( size.x / 2 - text_size.x / 2, size.y + size.y / 5 + 5 ), MAKE_COLOR( 245, 246, 250, 255 ), static_cast< int >( size.x * 0.35f ), "%.0f%%", damage_pcent * 100.f );
					}
					else if ( smite_spell::range_style->get_int( ) == 1 )
					{
						static auto size = vector( myhero->get_bounding_radius( ) * 0.75f, myhero->get_bounding_radius( ) * 0.75f );

						auto w2s = vector( ( min.x + max.x ) / 2 - size.x / 2, std::max( min.y, max.y ) + size.y / 2 );
						draw_manager->add_image( smite->handle( )->get_icon_texture( ), w2s, size );
						
						draw_manager->add_filled_rect( w2s + vector( 0, size.y ), w2s + vector( size.x, size.x + size.x / 5 ), MAKE_COLOR( 220, 216, 192, 255 ) );

						auto damage_pcent = std::min( 3.0f, damage / object->get_health( ) );
						auto color = A_::get_color( damage_pcent * 100.f );

						draw_manager->add_filled_rect( w2s + vector( 0, size.y ), w2s + vector( size.x * std::min( 1.f, std::max( 0.1f, damage_pcent ) ), size.y + ( size.y / 5 ) ),
							damage_pcent >= 0.75 ? A_::animate_color( color, 120, 255 ) : color );

						auto text_size = draw_manager->calc_text_size( static_cast< int >( size.x * 0.35f ), "%.0f%%", damage_pcent * 100.f );
						draw_manager->add_text_on_screen( w2s + vector( size.x / 2 - text_size.x / 2, size.y + size.y / 5 + 5 ), MAKE_COLOR( 245, 246, 250, 255 ), static_cast< int >( size.x * 0.35f ), "%.0f%%", damage_pcent * 100.f );
					}
				}

				if ( smite_spell::range_style->get_int( ) == 0 )
				{
					draw_manager->add_circle_with_glow( position,
						myhero->get_distance( object ) < smite->range( ) + myhero->get_bounding_radius( ) + object->get_bounding_radius( ) ?
						MAKE_COLOR( 29, 209, 161, 255 ) //in_range: green | old: 255, 100, 0 ( orange )
						:
						MAKE_COLOR( 254, 202, 87, 160 ) //out of range: yellow, old: 255, 255, 255, 255 ( white )
						, range, thickness, glow_data( glow_ind_s, glow_ind_p, glow_out_s, glow_out_p ) );
				}
				else if ( smite_spell::range_style->get_int( ) == 1 )
				{
					auto in_range = myhero->get_distance( object ) < smite->range( ) + myhero->get_bounding_radius( ) + object->get_bounding_radius( );
					A_::draw_corner_box( min, max,
						in_range ?
						MAKE_COLOR( 29, 209, 161, 255 ) //in_range: green | old: 255, 100, 0 ( orange )
						:
						MAKE_COLOR( 254, 202, 87, 255 ) //out of range: yellow, old: 255, 255, 255, 255 ( white )
						, in_range, -1.f, 3.f );
				}
			}
		}
	}

	void draw_smite()
	{
		if (!smite_spell::key->get_bool() || !smite)
			return;

		if (smite_spell::range->get_int() > 100 && smite_spell::mode->get_int() == 0)
		{
			for (auto&& object : entitylist->get_jugnle_mobs_minions())
			{
				if (!object->is_valid())
					continue;

				auto name = object->get_name();
				std::transform(name.begin(), name.end(), name.begin(),
					[](unsigned char c) { return std::tolower(c); });

				if (A_::is_epic_monster(name))
					smite_circle(object, smite_spell::smite_epic->get_bool());
				else if (name.find("sru_crab") != std::string::npos)
					smite_circle(object, smite_spell::small_crab->get_bool());
				else if ( A_::is_jungle_buff(name))
					smite_circle(object, smite_spell::red_buff->get_bool());
				else if (name.find("sru_murkwolf") != std::string::npos && name.find("mini") == std::string::npos)
					smite_circle(object, smite_spell::three_wolf->get_bool());
				else if (name.find("sru_krug") != std::string::npos && name.find("mini") == std::string::npos)
					smite_circle(object, smite_spell::stone->get_bool());
				else if (name.find("sru_razor") != std::string::npos && name.find("mini") == std::string::npos)
					smite_circle(object, smite_spell::six_bird->get_bool());
				else if (name.find("sru_gromp") != std::string::npos)
					smite_circle(object, smite_spell::gromp->get_bool());
			}

			if ((smite_spell::mode->get_int() == 1 && smite_spell::smite_siege_minion->get_bool()) ||
				(smite_spell::mode->get_int() == 0 && smite_spell::cannon->get_bool()))
			{
				for (auto&& minions : entitylist->get_enemy_minions())
				{
					if (!minions->is_valid() || minions->get_bounding_radius() != 65.f || !minions->is_lane_minion())
						continue;

					smite_circle(minions, true);
				}
			}
		}
	}

	void draw_smite_state()
	{
		if ( smite /*&& smite->is_ready( )*/ && ( smite_spell::draw_hpdamage_arrow->get_bool( ) || smite_spell::draw_hpdamage_fill->get_bool( ) ) && smite_spell::key->get_bool( ) )
		{
			for ( auto&& x : entitylist->get_jugnle_mobs_minions( ) )
			{
				if ( !x->is_valid( ) || !x->is_valid_target( smite->range( ) + myhero->get_bounding_radius( ) + x->get_bounding_radius( ) + 300.f ) )
					continue;

				auto name = x->get_name( );
				if ( name.empty( ) )
					continue;

				std::transform( name.begin( ), name.end( ), name.begin( ),
					[ ]( unsigned char c ) { return std::tolower( c ); } );

				if ( !A_::is_smite_monster( name ) )
					continue;

				vector4 v4_bar_pos;
				vector4 v4_hp_bar_pos;
				if ( !x->get_health_bar_position( v4_bar_pos, v4_hp_bar_pos ) )
					continue;

				auto bar_width = v4_hp_bar_pos.z,
					bar_height = v4_hp_bar_pos.w;

				auto bar_pos = vector( v4_hp_bar_pos.x,
					v4_hp_bar_pos.y );

				auto damage = 600.f;
				std::vector< buff_instance_script> buffs = { myhero->get_buff( buff_hash( "SmiteDamageTracker" ) ), myhero->get_buff( buff_hash( "SmiteDamageTrackerStalker" ) ), myhero->get_buff( buff_hash( "SmiteDamageTrackerAvatar" ) ) };
				std::sort( std::begin( buffs ), std::end( buffs ), [ ]( buff_instance_script b1, buff_instance_script b2 ) {
					return b1 && b1->is_valid( ) && ( !b2 || !b2->is_valid( ) );
				} );
				auto&& buff = buffs.front( );
				if ( buff && buff->is_valid( ) && buff->is_alive( ) )
					damage = ( buff == myhero->get_buff( buff_hash( "SmiteDamageTrackerStalker" ) ) && x->get_bounding_radius( ) == 65.f ) ? 500.f + x->get_max_health( ) * 0.1f : buff->get_count( );

				auto bar_pos_damage = bar_pos;
				bar_pos_damage.x += bar_width * ( damage / x->get_max_health( ) );

				if ( smite_spell::draw_hpdamage_arrow->get_bool( ) )
				{
					auto line_end = vector( bar_pos_damage.x, bar_pos_damage.y + bar_height );
					draw_manager->add_line_on_screen( bar_pos_damage, line_end, MAKE_COLOR( 255, 255, 255, 255 ), 2.f );

					auto l_arrow = bar_pos_damage.extend( line_end, -bar_height / 1.5f );
					auto r_arrow = l_arrow - ( bar_pos_damage - l_arrow ).rotated( -45.f );
					l_arrow = l_arrow - ( bar_pos_damage - l_arrow ).rotated( 45.f );

					auto color = smite->is_ready( ) ? MAKE_COLOR( 0, 184, 148, 255 ) : MAKE_COLOR( 250, 177, 160, 255 );
					draw_manager->add_path_filled_on_screen( { l_arrow, r_arrow, bar_pos_damage }, color );
					draw_manager->add_line_on_screen( bar_pos_damage, l_arrow, MAKE_COLOR( 0, 0, 0, 255 ), 2 );
					draw_manager->add_line_on_screen( bar_pos_damage, r_arrow, MAKE_COLOR( 0, 0, 0, 255 ), 2 );
					draw_manager->add_line_on_screen( l_arrow, r_arrow, MAKE_COLOR( 0, 0, 0, 255 ), 2 );
				}

				if ( smite_spell::draw_hpdamage_fill->get_bool( ) )
				{
					bar_pos_damage.y += bar_height;
					draw_manager->add_filled_rect( bar_pos, bar_pos_damage, MAKE_COLOR( 162, 155, 254, 80 ) );
				}
				
				//draw_manager->add_filled_circle_on_screen( bar_pos_damage.extend( line_end, -bar_height / 8.f ), bar_height / 4.f, MAKE_COLOR( 255, 255, 255, 255 ) );
				//draw_manager->add_filled_circle_on_screen( line_end.extend( bar_pos_damage, -bar_height / 8.f ), bar_height / 4.f, MAKE_COLOR( 255, 255, 255, 255 ) );
			}
		}

		if (smite && smite_spell::state->get_bool())
		{
			std::string word = smite_spell::key->get_bool() ? "Smite: ON" : "Smite: OFF";
			auto color = smite_spell::key->get_bool() ? MAKE_COLOR(16, 172, 132, 255) : MAKE_COLOR(255, 107, 107, 255);
			auto pos = myhero->get_position() + vector(100, -100);
			if (smite_spell::key->get_bool() && smite_spell::smite_kill_key->get_bool() && smite_spell::mode->get_int() == 1)
			{
				word = "Smite: On [Can kill]";
			}
			renderer->world_to_screen(pos, pos);
			draw_manager->add_text_on_screen(pos, color, 20, word.c_str());
		}
		if (all_spell::use->get_bool())
		{
			auto pos = myhero->get_position() + vector(100, -140);
			renderer->world_to_screen(pos, pos);
			draw_manager->add_text_on_screen(pos, MAKE_COLOR(255, 0, 0, 255), 20, "Disable summoner spell");
		}
	}
#pragma endregion

#pragma region exhaust
	float twitch_use_e, xayah_use_e;
	int count_twitch_e;
	void exhaust_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_exhaust = summoner->add_tab("exhaust_menu", "Exhaust");
		if (summoner_exhaust)
		{
			/*if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerExhaust"))
				summoner_exhaust->set_texture(myhero->get_spell(spellslot::summoner1)->get_icon_texture());
			else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerExhaust"))
				summoner_exhaust->set_texture(myhero->get_spell(spellslot::summoner2)->get_icon_texture());*/

			auto db_summoner_exhaust = database->get_spell_by_hash( spell_hash( "SummonerExhaust" ) );
			if ( db_summoner_exhaust )
			{
				auto exhaust_texture = db_summoner_exhaust->get_icon_texture_by_index( 0 );
				if ( exhaust_texture )
					summoner_exhaust->set_texture( exhaust_texture );
			}

			summoner_exhaust->add_separator("exhaust1", " - -  Exhaust  - - ");

			exhaust_spell::key = summoner_exhaust->add_hotkey("key", "Semi exhaust", TreeHotkeyMode::Hold, 'F', false);
			exhaust_spell::type = summoner_exhaust->add_combobox("type", "Type", { {"Low hp",nullptr},{"Target selector",nullptr },{"Close to me",nullptr },{"Close to mouse",nullptr } }, 2, true);

			summoner_exhaust->add_separator("exhaust3", " - -  Use on  - - ");

			for (auto& ally : entitylist->get_enemy_heroes())
			{
				exhaust_spell::use_to[ally->get_champion()] = summoner_exhaust->add_checkbox(std::to_string(static_cast<int>( ally->get_champion())) + "exhaust", ally->get_model(), true, true);
				exhaust_spell::use_to[ally->get_champion()]->set_texture(ally->get_square_icon_portrait());
			}

			summoner_exhaust->add_separator("exhaust2", " - -  Auto  - - ");
			{
				exhaust_spell::exhaust = summoner_exhaust->add_checkbox( "exhaust", "Enabled", true, true );
				exhaust_spell::exhaust_for_ally = summoner_exhaust->add_checkbox( "exhaust_for_ally", "Use for ally", false, true );

				auto&& spells = exhaust_spelldb::load( );
				for ( auto&& enemy : entitylist->get_enemy_heroes( ) )
				{
					if ( enemy->get_champion( ) == champion_id::Twitch )
					{
						auto twitch = summoner_exhaust->add_tab( "twitch", "Twitch" );
						twitch->set_texture( enemy->get_square_icon_portrait( ) );
						{
							twitch->add_separator( enemy->get_model( ), enemy->get_model( ) );
							exhaust_spell::twitch_e_count = twitch->add_slider( "exhaust_twitch", "Twitch's E >= x", 5, 1, 6 );
						}
					}
					if ( enemy->get_champion( ) == champion_id::Xayah )
					{
						auto xayah = summoner_exhaust->add_tab( "xayah", "Xayah" );
						xayah->set_texture( enemy->get_square_icon_portrait( ) );
						{
							xayah->add_separator( enemy->get_model( ), enemy->get_model( ) );
							exhaust_spell::xayah_e_count = xayah->add_slider( "exhaust_xayah", "Xayah's E >= x", 5, 1, 20 );
						}
					}
					auto& enemy_spells = spells [ enemy->get_champion( ) ];

					if ( !enemy_spells.empty( ) )
					{
						auto enemy_config = summoner_exhaust->add_tab( enemy->get_model( ), enemy->get_model( ) );
						enemy_config->set_texture( enemy->get_square_icon_portrait( ) );

						for ( auto&& spell_data : enemy_spells )
						{
							enemy_config->add_separator( enemy->get_model( ), enemy->get_model( ) );
							auto&& spell_name = spell_data.display_name;
							auto spell_config = enemy_config->add_checkbox( spell_name, enemy->get_model( ) + "_" + spell_name, spell_data.menu );
							auto spell_hp = enemy_config->add_slider( std::string( spell_name ) + "hp", "  ^- Hp <= x%", spell_data.hp, 0, 100 );

							exhaust_spell::config.push_back( spell_config );
							exhaust_spell::config_hp.push_back( spell_hp );
						}
					}
				}
			}
			summoner_exhaust->add_separator( "exhaust3", " - -  Anti-gapcloser  - - " );
			{
				exhaust_spell::exhaust_antigap = summoner_exhaust->add_checkbox( "exhaust_antigap", "Enabled", true, true );
				for ( auto&& x : entitylist->get_enemy_heroes( ) )
				{
					if ( !x || !x->is_valid( ) )
						continue;

					auto tab = summoner_exhaust->add_tab( x->get_model( ) + "_anti_gap_tab", x->get_model( ) );
					{
						auto pair = std::make_pair(
							tab->add_combobox( "mode", "Mode", { { "Disabled", nullptr }, { "Always", nullptr }, { "If health <= {x} %", nullptr } }, 0 ),
							tab->add_slider( "hp", " ^ - Health (%)", 40, 5, 100 )
						);

						exhaust_spell::m_mode_hp [ x->get_champion( ) ] = pair;
					}
				}
			}

			auto on_change = [ ]( TreeEntry* ) {
				*exhaust_spell::b_enabled = exhaust_spell::key->get_bool( ) || exhaust_spell::exhaust->get_bool( ) || exhaust_spell::exhaust_antigap->get_bool( );
			};
			exhaust_spell::key->add_property_change_callback( on_change );
			exhaust_spell::exhaust->add_property_change_callback( on_change );
			exhaust_spell::exhaust_antigap->add_property_change_callback( on_change );
			on_change( nullptr );

			summoner_exhaust->set_assigned_active( exhaust_spell::b_enabled );
		}
	}
	//exhaust
	void exhaust_logic( )
	{
		static auto main_tab = menu->get_tab( "utility" );
		if ( !main_tab )
			return;

		static auto summoner = main_tab->get_tab( "summoner" );
		if ( !summoner )
			return;

		static auto summoner_exhaust = summoner->get_tab( "exhaust_menu" );
		if ( !summoner_exhaust )
			return;

		if ( !exhaust || !exhaust->is_ready( ) || !exhaust_spell::exhaust || !exhaust_spell::exhaust->get_bool( ) ) return;

		for ( auto&& enemy : entitylist->get_enemy_heroes( ) )
		{
			if ( enemy != nullptr && enemy->is_valid_target( 650 ) )
			{
				switch ( enemy->get_champion( ) )
				{
					default:
						break;

					case champion_id::Xayah:
					{
						if ( xayah_use_e > gametime->get_time( ) )
						{
							auto e_count = champion::count_xayah_e( );
							if ( e_count >= exhaust_spell::xayah_e_count->get_int( ) )
							{
								exhaust->cast( enemy );
								return;
							}
						}

						break;
					}

					case champion_id::Twitch:
					{
						if ( twitch_use_e > gametime->get_time( ) )
						{
							auto e_count = count_twitch_e;
							if ( e_count >= exhaust_spell::twitch_e_count->get_int( ) )
							{
								exhaust->cast( enemy );
								return;
							}
						}
						break;
					}
				}

				static auto spells = exhaust_spelldb::load( );
				auto it_spells = spells.find( enemy->get_champion( ) );
				if ( it_spells == spells.end( ) || it_spells->second.empty( ) )
					continue;

				auto& enemy_spells = it_spells->second;
				for ( auto&& spell_data : enemy_spells )
				{
					auto champion = summoner_exhaust->get_tab( enemy->get_model( ) );

					if ( !champion ) 
						continue;

					auto config = champion->get_entry( spell_data.display_name );
					if ( !config )
						continue;

					auto config_hp = champion->get_entry( std::string( spell_data.display_name ) + "hp" );
					if ( !config_hp ) 
						continue;

					if ( config->get_bool( ) && spell_data.is_active( enemy, config_hp->get_int( ), exhaust_spell::exhaust_for_ally ) )
					{
						exhaust->cast( enemy );
						return;
					}
				}
			}
		}
	}
	float exhaust_spell_time;
	void key_exhaust_logic()
	{
		if (!exhaust || !exhaust->is_ready()) return;

		if (!exhaust_spell::key->get_bool()) return;

		auto enemy = entitylist->get_enemy_heroes();

		enemy.erase(std::remove_if(enemy.begin(), enemy.end(), [](game_object_script x)
			{
				return !x->is_valid() || x->is_me( ) || x->get_distance(myhero) > 650 || x->is_dead() || !A_::checkbox(exhaust_spell::use_to, x);
			}), enemy.end());

		if (gametime->get_time() > exhaust_spell_time)
		{
			if (exhaust_spell::type->get_int() == 0)
			{
				std::sort(enemy.begin(), enemy.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_health() < b->get_health();
					});

				if (!enemy.empty())
				{
					exhaust->cast(enemy.front());
					exhaust_spell_time = gametime->get_time() + 0.5f;
					return;
				}
			}
			else if (exhaust_spell::type->get_int() == 1)
			{
				auto target = target_selector->get_target(650, damage_type::true_dmg);
				if (target != nullptr && A_::checkbox(exhaust_spell::use_to, target))
				{
					if (exhaust->cast(target))
					{
						exhaust_spell_time = gametime->get_time() + 0.5f;
						return;
					}
				}
			}
			else if (exhaust_spell::type->get_int() == 2)
			{
				std::sort(enemy.begin(), enemy.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_distance(myhero) < b->get_distance(myhero);
					});

				if (!enemy.empty())
				{
					exhaust->cast(enemy.front());
					exhaust_spell_time = gametime->get_time() + 0.5f;
					return;
				}
			}
			else if (exhaust_spell::type->get_int() == 3)
			{
				std::sort(enemy.begin(), enemy.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_distance(hud->get_hud_input_logic()->get_game_cursor_position()) < b->get_distance(hud->get_hud_input_logic()->get_game_cursor_position());
					});

				if (!enemy.empty())
				{
					exhaust->cast(enemy.front());
					exhaust_spell_time = gametime->get_time() + 0.5f;
					return;
				}
			}
		}
	}

	void draw_exhaust()
	{
		if (exhaust && exhaust_spell::exhaust->get_bool())
		{
			/*if (language_info* lang = translation->add_language("cn", "CN"))
			{
				auto pos = myhero->get_position() + vector(40, -40);
				renderer->world_to_screen(pos, pos);
				draw_manager->add_text_on_screen((pos), MAKE_COLOR(255, 125, 0, 255), 17, "Ì“Èõ");
			}*/
			/*auto pos = myhero->get_position() + vector(40, -40);
			renderer->world_to_screen(pos, pos);
			draw_manager->add_text_on_screen((pos), MAKE_COLOR(255, 125, 0, 255), 17, "Exhaust");*/
		}
	}
#pragma endregion

#pragma region ghost
	void ghost_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_ghost = summoner->add_tab("ghost_menu", "Ghost");
		if (summoner_ghost)
		{
			/*if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerHaste"))
				summoner_ghost->set_texture(myhero->get_spell(spellslot::summoner1)->get_icon_texture());
			else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerHaste"))
				summoner_ghost->set_texture(myhero->get_spell(spellslot::summoner2)->get_icon_texture());*/

			auto db_summoner_haste = database->get_spell_by_hash( spell_hash( "SummonerHaste" ) );
			if ( db_summoner_haste )
			{
				auto haste_texture = db_summoner_haste->get_icon_texture_by_index( 0 );
				if ( haste_texture )
					summoner_ghost->set_texture( haste_texture );
			}

			summoner_ghost->add_separator("ghost1", " - - Ghost - - ");

			ghost_spell::use = summoner_ghost->add_checkbox("use", "Ghost", true, true);
			summoner_ghost->set_assigned_active(ghost_spell::use);

			summoner_ghost->add_separator("ghost2", " - - Chase logic - - ");
			ghost_spell::chase = summoner_ghost->add_checkbox("chase", "Chase", true, true);
			ghost_spell::_logic_1 = summoner_ghost->add_checkbox("_logic1", "  ^-Logic (?)", true, true);
			ghost_spell::_logic_1->set_tooltip("Avoiding wasting ghost");
			ghost_spell::hp_high = summoner_ghost->add_slider("hp_high", "  ^-Enemy HP >= x%", 10, 10, 30);
			ghost_spell::hp_low = summoner_ghost->add_slider("hp_low", "  ^-Enemy HP <= x%", 40, 30, 60);
			ghost_spell::mouse_dis = summoner_ghost->add_slider("mouse_dis", "  ^-Mouse distance <= x%", 300, 100, 500);
			ghost_spell::enemy_dis = summoner_ghost->add_slider("enemy_dis", "  ^-Enemy distance <= AA range + x", 500, 300, 600);

			auto chase_to = summoner_ghost->add_tab("chase_to", "Chase whitelist");
			{
				chase_to->add_separator("ghost0", " - - Chase whitelist - - ");
				for (auto& ally : entitylist->get_enemy_heroes())
				{
					ghost_spell::use_to[ally->get_network_id()] = chase_to->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, true);
					ghost_spell::use_to[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
				}
			}

			/*summoner_ghost->add_separator("ghost3", " - - Flee logic - - ");
			ghost_spell::flee = summoner_ghost->add_checkbox("flee", "Flee", true, true);
			ghost_spell::_logic_2 = summoner_ghost->add_checkbox("_logic2", "  ^- logic (?)", true, true);
			ghost_spell::_logic_2->set_tooltip("Avoiding waste ghost");
			ghost_spell::hp_low_flee = summoner_ghost->add_slider("hp_low_flee", "  ^-HP <= x%", 33, 10, 40);*/

			/*auto flee_to = summoner_ghost->add_tab("flee_to", "Flee to");
			{
				flee_to->add_separator("ghost0", " - - Flee to - - ");
				for (auto& ally : entitylist->get_ally_heroes())
				{
					if ( ally->is_me( ) )
						continue;

					ghost_spell::flee_to[ally->get_network_id()] = flee_to->add_checkbox("flee" + std::to_string(ally->get_network_id()), ally->get_model(), true, true);
					ghost_spell::flee_to[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
				}
			}*/
		}
	}
	void ghost_logic()
	{
		if (!ghost || !ghost->is_ready() || !ghost_spell::use->get_bool()) return;

		if (ghost_spell::chase->get_bool() && orbwalker->combo_mode())
		{
			auto dis = ghost_spell::enemy_dis->get_int() + myhero->get_attack_range() + myhero->get_bounding_radius();
			auto hud_dis = ghost_spell::mouse_dis->get_int();
			auto hud_pos = hud->get_hud_input_logic()->get_game_cursor_position();

			auto target = target_selector->get_target(dis, damage_type::physical);
			if (target && target->get_distance(hud_pos) < hud_dis && A_::checkbox(ghost_spell::use_to, target))
			{
				auto is_dashing = false;
				{
					auto ai_path = target->get_path_controller( );
					auto thru_wall = false;
					if ( ai_path && ai_path->is_dashing( ) )
					{
						auto d = ai_path->get_start_vec( ).distance( ai_path->get_end_vec( ) );
						if ( d > 20 )
						{
							for ( int i = 10; i < d; i += 10 )
							{
								if ( ai_path->get_start_vec( ).extend( ai_path->get_end_vec( ), static_cast< float >( i ) ).is_wall( ) )
								{
									thru_wall = true;
									break;
								}
							}
						}
						if ( !thru_wall )
							is_dashing = true;
					}
				}
				bool  = !myhero->is_in_auto_attack_range( target ) || target->get_move_speed( ) - myhero->get_move_speed( ) > 33.f || is_dashing;
				bool hp = target->get_health_percent() >= ghost_spell::hp_high->get_int() && target->get_health_percent() <= ghost_spell::hp_low->get_int();
				bool face = myhero->is_facing(target);
				if (hp && face && )
				{
					ghost->cast();
					return;
				}
			}
		}
		/*if (ghost_spell::flee->get_bool() && orbwalker->flee_mode())
		{
			int count = myhero->count_enemies_in_range(500);
			auto target = target_selector->get_target(500, damage_type::physical);
			bool hp = myhero->get_health_percent() < ghost_spell::hp_low_flee->get_int();

			if (target && count > 0 && A_::checkbox(ghost_spell::flee_to, target) && hp)
			{
				bool  = target->get_move_speed() + 100 > myhero->get_move_speed();
				if ()
					ghost->cast();
			}
		}*/
	}
#pragma endregion

#pragma region cleanse

	//œQ»¯
	float cc_time_loop = 0.0f;
	float cc_clean_loop = 0.0f;
	float Exhaust_time;
	float Ignite_time;
	float CombatDehancer_time;		//Ì“Èõ
	float Slow_time;				//œpËÙ
	float AttackSpeedSlow_time;		//¹¥ËÙ½µµÍ
	float Snare_time;				//½ûåd
	float Stun_time;				//•žÑ£
	float Taunt_time;				//³°ÖS
	float Berserk_time;				//¿ñ±©
	float Nasusw_time;
	float Polymorph_time;			//×ƒÐÎ
	float Fear_time;				//¿Ö‘Ö
	float Flee_time;				//ÌÓÅÜ
	float Charm_time;				//÷È»ó
	float Poison_time;				//¶¾Ò©
	float Suppression_time;			//‰ºÖÆ
	float NearSight_time;			//½üÊÓ
	float Blind_time;				//Ã¤ÈË
	float Disarm_time;				//½â³ýÎä×°
	float Knockup_time;				//“ôïw
	float Knockback_time;			//ÍÆé_
	float Drowsy_time;				//»è»èÓûË¯
	float Asleep_time;				//Ë¯ÓX
	float Silence_time;				//³ÁÄ¬

	void cleanse_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto qss_cleanse = main_tab->add_tab("cleanse_menu", "Cleanse/Qss/Mikael");
		if (qss_cleanse)
		{
			auto qss = database->get_item_by_id(ItemId::Quicksilver_Sash)->get_texture();
			auto qss_icon = create_texture_descriptor(qss.first, { qss.second.x, qss.second.y, qss.second.z, qss.second.w });
			qss_cleanse->set_assigned_texture(qss_icon);
			qss_cleanse->add_separator("qss_cleanse_setting", " - - Setting - - ");
			//qss_cleanse->set_revert_enabled(true);
			auto item_mikael = qss_cleanse->add_tab("mikael", "Mikael");
			{
				auto mikael = database->get_item_by_id(ItemId::Mikaels_Blessing)->get_texture();
				auto mikael_icon = create_texture_descriptor(mikael.first, { mikael.second.x, mikael.second.y, mikael.second.z, mikael.second.w });
				item_mikael->set_assigned_texture(mikael_icon);
				item_mikael->add_separator("mikael", " - - Mikael - - ");
				cleanse_spell::mikael = item_mikael->add_checkbox("use_mikael", "Mikael", true);
				item_mikael->set_assigned_active(cleanse_spell::mikael);

				item_mikael->add_separator("cleanse_setting1", "Mikael use on");
				for (auto& ally : entitylist->get_ally_heroes())
				{
					if (!ally->is_me())
					{
						cleanse_spell::mikael_use_on[ally->get_network_id()] = item_mikael->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, false);
						cleanse_spell::mikael_use_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
					}
				}

				item_mikael->add_separator("mikael_setting2", " - - Mikael setting - - ");

				cleanse_spell::Berserk_mikael = item_mikael->add_slider("Berserk_mikael", "Danger level : Berserk", 4, 0, 4);
				cleanse_spell::Stun_mikael = item_mikael->add_slider("Stun_mikael", "Danger level : Stun", 3, 0, 4);
				cleanse_spell::Taunt_mikael = item_mikael->add_slider("Taunt_mikael", "Danger level : Taunt", 3, 0, 4);
				cleanse_spell::Polymorph_mikael = item_mikael->add_slider("Polymorph_mikael", "Danger level : Polymorph", 3, 0, 4);
				cleanse_spell::Fear_mikael = item_mikael->add_slider("Fear_mikael", "Danger level : Fear", 3, 0, 4);
				cleanse_spell::Charm_mikael = item_mikael->add_slider("Charm_mikael", "Danger level : Charm", 3, 0, 4);
				cleanse_spell::Drowsy_mikael = item_mikael->add_slider("Drowsy_mikael", "Danger level : Drowsy", 3, 0, 4);
				cleanse_spell::Asleep_mikael = item_mikael->add_slider("Asleep_mikael", "Danger level : Asleep", 3, 0, 4);
				cleanse_spell::Snare_mikael = item_mikael->add_slider("Snare_mikael", "Danger level : Snare", 2, 0, 4);
				cleanse_spell::Slow_mikael = item_mikael->add_slider("Slow_mikael", "Danger level : Slow", 1, 0, 4);
				cleanse_spell::Silence_mikael = item_mikael->add_slider("Silence_mikael", "Danger level : Silence", 0, 0, 4);

				item_mikael->add_separator( "mikael_setting3", " - - Mikael heal - - " );
				cleanse_spell::mikael_heal = item_mikael->add_checkbox( "mikael_heal", "Use to heal", false );
				cleanse_spell::mikael_heal_hp = item_mikael->add_slider( "mikael_heal_hp", " ^ - Health (%)", 15, 0, 60 );

				item_mikael->add_separator( "mikael_setting4", " - - Heal whitelist - - " );
				for ( auto&& ally : entitylist->get_ally_heroes( ) )
				{
					if ( ally && ally->is_valid( ) )
					{
						cleanse_spell::mikael_heal_on [ ally->get_champion( ) ] = item_mikael->add_checkbox( "heal_" + std::to_string( static_cast<int>( ally->get_champion( ) ) ), ally->get_model( ), true );
						cleanse_spell::mikael_heal_on [ ally->get_champion( ) ]->set_texture( ally->get_square_icon_portrait( ) );
					}
				}
			}

			auto summoner_cleanse = qss_cleanse->add_tab("cleanse", "Cleanse");
			{
				auto db_summoner_cleanse = database->get_spell_by_hash( spell_hash( "SummonerBoost" ) );
				if ( db_summoner_cleanse )
				{
					auto cleanse_texture = db_summoner_cleanse->get_icon_texture_by_index( 0 );
					if ( cleanse_texture )
						summoner_cleanse->set_texture( cleanse_texture );
				}

				summoner_cleanse->add_separator("cleanse", " - - Cleanse - - ");
				cleanse_spell::cleanse = summoner_cleanse->add_checkbox("use_cleanse", "Cleanse", true);
				summoner_cleanse->set_assigned_active(cleanse_spell::cleanse);

				summoner_cleanse->add_separator("cleanse_setting", " - - Cleanse setting - - ");
				cleanse_spell::Exhaust_cleanse = summoner_cleanse->add_slider("Exhaust_cleanse", "Danger level : Exhaust", 2, 0, 4);
				cleanse_spell::Nasusw_cleanse = summoner_cleanse->add_slider( "Nasusw_cleanse", "Danger level : Nasus W", 3, 0, 4 );
				cleanse_spell::Ignite_cleanse = summoner_cleanse->add_slider("Ignite_cleanse", "Danger level : Ignite", 2, 0, 4);
				cleanse_spell::Berserk_cleanse = summoner_cleanse->add_slider("Berserk_cleanse", "Danger level : Berserk", 4, 0, 4);
				cleanse_spell::Stun_cleanse = summoner_cleanse->add_slider("Stun_cleanse", "Danger level : Stun", 3, 0, 4);
				cleanse_spell::Taunt_cleanse = summoner_cleanse->add_slider("Taunt_cleanse", "Danger level : Taunt", 3, 0, 4);
				cleanse_spell::Polymorph_cleanse = summoner_cleanse->add_slider("Polymorph_cleanse", "Danger level : Polymorph", 3, 0, 4);
				cleanse_spell::Fear_cleanse = summoner_cleanse->add_slider("Fear_cleanse", "Danger level : Fear", 3, 0, 4);
				cleanse_spell::Charm_cleanse = summoner_cleanse->add_slider("Charm_cleanse", "Danger level : Charm", 3, 0, 4);
				cleanse_spell::Drowsy_cleanse = summoner_cleanse->add_slider("Drowsy_cleanse", "Danger level : Drowsy", 3, 0, 4);
				cleanse_spell::Asleep_cleanse = summoner_cleanse->add_slider("Asleep_cleanse", "Danger level : Asleep", 3, 0, 4);
				cleanse_spell::Snare_cleanse = summoner_cleanse->add_slider("Snare_cleanse", "Danger level : Snare", 2, 0, 4);
				cleanse_spell::Slow_cleanse = summoner_cleanse->add_slider("Slow_cleanse", "Danger level : Slow", 1, 0, 4);
				cleanse_spell::Blind_cleanse = summoner_cleanse->add_slider("Blind_cleanse", "Danger level : Blind", 1, 0, 4);
				cleanse_spell::Disarm_cleanse = summoner_cleanse->add_slider("Disarm_cleanse", "Danger level : Disarm", 1, 0, 4);
				cleanse_spell::Silence_cleanse = summoner_cleanse->add_slider("Silence_cleanse", "Danger level : Silence", 0, 0, 4);
				cleanse_spell::NearSight_cleanse = summoner_cleanse->add_slider("NearSight_cleanse", "Danger level : NearSight", 0, 0, 4);
				if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerBoost"))
				{
					summoner_cleanse->set_texture(myhero->get_spell(spellslot::summoner1)->get_icon_texture());
				}
				else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerBoost"))
				{
					summoner_cleanse->set_texture(myhero->get_spell(spellslot::summoner2)->get_icon_texture());
				}
			}
			auto item_qss = qss_cleanse->add_tab("qss", "Qss");
			{
				auto qss = database->get_item_by_id(ItemId::Quicksilver_Sash)->get_texture();
				auto qss_icon = create_texture_descriptor(qss.first, { qss.second.x, qss.second.y, qss.second.z, qss.second.w });
				item_qss->set_assigned_texture(qss_icon);
				item_qss->add_separator("qss", " - - Qss - - ");
				cleanse_spell::qss = item_qss->add_checkbox("use_qss", "Qss", true);
				item_qss->set_assigned_active(cleanse_spell::qss);

				item_qss->add_separator("cleanse_setting", " - - Qss setting - - ");

				cleanse_spell::Nasusw_qss = item_qss->add_slider( "Nasusw_qss", "Danger level : Nasus W", 4, 0, 4 );
				cleanse_spell::Berserk_qss = item_qss->add_slider("Berserk_qss", "Danger level : Berserk", 4, 0, 4);
				cleanse_spell::Suppression_qss = item_qss->add_slider("Suppression_qss", "Danger level : Suppression", 4, 0, 4);
				cleanse_spell::Stun_qss = item_qss->add_slider("Stun_qss", "Danger level : Stun", 3, 0, 4);
				cleanse_spell::Taunt_qss = item_qss->add_slider("Taunt_qss", "Danger level : Taunt", 3, 0, 4);
				cleanse_spell::Polymorph_qss = item_qss->add_slider("Polymorph_qss", "Danger level : Polymorph", 3, 0, 4);
				cleanse_spell::Fear_qss = item_qss->add_slider("Fear_qss", "Danger level : Fear", 3, 0, 4);
				cleanse_spell::Charm_qss = item_qss->add_slider("Charm_qss", "Danger level : Charm", 3, 0, 4);
				cleanse_spell::Drowsy_qss = item_qss->add_slider("Drowsy_qss", "Danger level : Drowsy", 3, 0, 4);
				cleanse_spell::Asleep_qss = item_qss->add_slider("Asleep_qss", "Danger level : Asleep", 3, 0, 4);
				cleanse_spell::Snare_qss = item_qss->add_slider("Snare_qss", "Danger level : Snare", 2, 0, 4);
				cleanse_spell::Slow_qss = item_qss->add_slider("Slow_qss", "Danger level : Slow", 1, 0, 4);
				cleanse_spell::Blind_qss = item_qss->add_slider("Blind_qss", "Danger level : Blind", 1, 0, 4);
				cleanse_spell::Disarm_qss = item_qss->add_slider("Disarm_qss", "Danger level : Disarm", 1, 0, 4);
				cleanse_spell::Silence_qss = item_qss->add_slider("Silence_qss", "Danger level : Silence", 0, 0, 4);
				cleanse_spell::NearSight_qss = item_qss->add_slider("NearSight_qss", "Danger level : NearSight", 0, 0, 4);
			}
			auto cleanse_danger = qss_cleanse->add_tab("cleanse_danger", "Danger level setting");
			{
				cleanse_danger->add_separator("cleanse_danger", " - - Danger Level - - ");
				auto cleanse_danger_0 = cleanse_danger->add_tab("cleanse_danger_0", "Danger level 0");
				{
					cleanse_danger_0->add_separator("cleanse_danger_0_1", "Danger level 0 = Disable");
				}
				auto cleanse_danger_1 = cleanse_danger->add_tab("cleanse_danger_1", "Danger level 1");
				{
					cleanse_spell::cleanse_danger_level_1_alway_use = cleanse_danger_1->add_checkbox("alway_use", "Alway use", false);
					cleanse_spell::cleanse_danger_level_1_delay = cleanse_danger_1->add_slider("cleanse_danger_level_1_delay", "Delay x ms", 200, 0, 1000);
					cleanse_spell::cleanse_danger_level_1_hp = cleanse_danger_1->add_slider("cleanse_danger_level_1_hp", "Hp <= x %", 40, 0, 100);
					cleanse_spell::cleanse_danger_level_1_enemy_range = cleanse_danger_1->add_slider("cleanse_danger_level_1_enemy_range", "When x range", 500, 0, 1000);
					cleanse_spell::cleanse_danger_level_1_enemy_count = cleanse_danger_1->add_slider("cleanse_danger_level_1_enemy_count", "  ^- >= x enemy", 2, 1, 5);
					cleanse_spell::cleanse_danger_level_1_duration = cleanse_danger_1->add_slider("cleanse_danger_level_1_duration", "CC duration >= x ms", 1001, 0, 2000);
					cleanse_spell::cleanse_danger_level_1_combo = cleanse_danger_1->add_checkbox("cleanse_danger_level_1_combo", "Use on combo mode", true);
					cleanse_spell::cleanse_danger_level_1_harass = cleanse_danger_1->add_checkbox("cleanse_danger_level_1_harass", "Use on harass mode", false);
					cleanse_spell::cleanse_danger_level_1_other = cleanse_danger_1->add_checkbox("cleanse_danger_level_1_other", "Use on other mode", false);
				}
				auto cleanse_danger_2 = cleanse_danger->add_tab("cleanse_danger_2", "Danger level 2");
				{
					cleanse_spell::cleanse_danger_level_2_alway_use = cleanse_danger_2->add_checkbox("alway_use", "Alway use", false);
					cleanse_spell::cleanse_danger_level_2_delay = cleanse_danger_2->add_slider("cleanse_danger_level_2_delay", "Delay x ms", 100, 0, 1000);
					cleanse_spell::cleanse_danger_level_2_hp = cleanse_danger_2->add_slider("cleanse_danger_level_2_hp", "Hp <= x %", 70, 0, 100);
					cleanse_spell::cleanse_danger_level_2_enemy_range = cleanse_danger_2->add_slider("cleanse_danger_level_2_enemy_range", "When x range", 700, 0, 1000);
					cleanse_spell::cleanse_danger_level_2_enemy_count = cleanse_danger_2->add_slider("cleanse_danger_level_2_enemy_count", "  ^- >= x enemy", 2, 1, 5);
					cleanse_spell::cleanse_danger_level_2_duration = cleanse_danger_2->add_slider("cleanse_danger_level_2_duration", "CC duration >= x ms", 700, 0, 2000);
					cleanse_spell::cleanse_danger_level_2_combo = cleanse_danger_2->add_checkbox("cleanse_danger_level_2_combo", "Use on combo mode", true);
					cleanse_spell::cleanse_danger_level_2_harass = cleanse_danger_2->add_checkbox("cleanse_danger_level_2_harass", "Use on harass mode", true);
					cleanse_spell::cleanse_danger_level_2_other = cleanse_danger_2->add_checkbox("cleanse_danger_level_2_other", "Use on other mode", false);
				}
				auto cleanse_danger_3 = cleanse_danger->add_tab("cleanse_danger_3", "Danger level 3");
				{
					cleanse_spell::cleanse_danger_level_3_alway_use = cleanse_danger_3->add_checkbox("alway_use", "Alway use", false);
					cleanse_spell::cleanse_danger_level_3_delay = cleanse_danger_3->add_slider("cleanse_danger_level_3_delay", "Delay x ms", 50, 0, 1000);
					cleanse_spell::cleanse_danger_level_3_hp = cleanse_danger_3->add_slider("cleanse_danger_level_3_hp", "Hp <= x %", 90, 0, 100);
					cleanse_spell::cleanse_danger_level_3_enemy_range = cleanse_danger_3->add_slider("cleanse_danger_level_3_enemy_range", "When x range", 1000, 0, 1000);
					cleanse_spell::cleanse_danger_level_3_enemy_count = cleanse_danger_3->add_slider("cleanse_danger_level_3_enemy_count", "  ^- >= x enemy", 1, 1, 5);
					cleanse_spell::cleanse_danger_level_3_duration = cleanse_danger_3->add_slider("cleanse_danger_level_3_duration", "CC duration >= x ms", 400, 0, 2000);
					cleanse_spell::cleanse_danger_level_3_combo = cleanse_danger_3->add_checkbox("cleanse_danger_level_3_combo", "Use on combo mode", true);
					cleanse_spell::cleanse_danger_level_3_harass = cleanse_danger_3->add_checkbox("cleanse_danger_level_3_harass", "Use on harass mode", false);
					cleanse_spell::cleanse_danger_level_3_other = cleanse_danger_3->add_checkbox("cleanse_danger_level_3_other", "Use on other mode", false);
				}
				auto cleanse_danger_4 = cleanse_danger->add_tab("cleanse_danger_4", "Danger level 4");
				{
					cleanse_spell::cleanse_danger_level_4_alway_use = cleanse_danger_4->add_checkbox("alway_use", "Alway use", true);
					cleanse_spell::cleanse_danger_level_4_delay = cleanse_danger_4->add_slider("cleanse_danger_level_4_delay", "Delay x ms", 0, 0, 1000);
					cleanse_spell::cleanse_danger_level_4_hp = cleanse_danger_4->add_slider("cleanse_danger_level_4_hp", "Hp <= x %", 100, 0, 100);
					cleanse_spell::cleanse_danger_level_4_enemy_range = cleanse_danger_4->add_slider("cleanse_danger_level_4_enemy_range", "When x range", 500, 0, 1000);
					cleanse_spell::cleanse_danger_level_4_enemy_count = cleanse_danger_4->add_slider("cleanse_danger_level_4_enemy_count", "  ^- >= x enemy", 1, 1, 5);
					cleanse_spell::cleanse_danger_level_4_duration = cleanse_danger_4->add_slider("cleanse_danger_level_4_duration", "CC duration x ms", 0, 0, 2000);
					cleanse_spell::cleanse_danger_level_4_combo = cleanse_danger_4->add_checkbox("cleanse_danger_level_4_combo", "Use on combo mode", true);
					cleanse_spell::cleanse_danger_level_4_harass = cleanse_danger_4->add_checkbox("cleanse_danger_level_4_harass", "Use on harass mode", true);
					cleanse_spell::cleanse_danger_level_4_other = cleanse_danger_4->add_checkbox("cleanse_danger_level_4_other", "Use on other mode", true);
				}
			}
		}
	}
	float cast_qss_t;
	void cast_qss()
	{
		Item qss_1(ItemId::Quicksilver_Sash, 0);
		Item qss_2(ItemId::Mercurial_Scimitar, 0);
		Item qss_3(ItemId::Silvermere_Dawn, 0);
		if (qss_1.is_ready())
		{
			qss_1.cast();
		}
		else if (qss_2.is_ready())
		{
			qss_2.cast();
		}
		else if (qss_3.is_ready())
		{
			qss_3.cast();
		}
	}
	void clean_cc_logic(float time, float duration, bool use_cleanse, int cleanse_danger, bool use_qss, int qss_danger, bool use_mikael, game_object_script target, int mikael_danger)
	{
		Item qss_1(ItemId::Quicksilver_Sash, 0);
		Item qss_2(ItemId::Mercurial_Scimitar, 0);
		Item qss_3(ItemId::Silvermere_Dawn, 0);
		Item mikael(ItemId::Mikaels_Blessing, 650);
		if (mikael.is_ready() && use_mikael == true)
		{
			if (mikael_danger == 0)
			{
				return;
			}
			else if (mikael_danger == 1)
			{
				if (cleanse_spell::cleanse_danger_level_1_alway_use->get_bool())
				{
					mikael.cast(target);
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_1_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_1_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_1_other->get_bool()))
					{
						if (target->get_health_percent() <= static_cast<float>( cleanse_spell::cleanse_danger_level_1_hp->get_int() ) ||
							target->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_1_enemy_range->get_int() )) >= cleanse_spell::cleanse_danger_level_1_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_1_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_1_delay->get_int() / 1000.0f ? 0 : time + cleanse_spell::cleanse_danger_level_1_delay->get_int() / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, [target]
									{
										Item mikael(ItemId::Mikaels_Blessing, 650);
										if (mikael.is_ready())
										{
											mikael.cast(target);
										}
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", target->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (mikael_danger == 2)
			{
				if (cleanse_spell::cleanse_danger_level_2_alway_use->get_bool())
				{
					mikael.cast(target);
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_2_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_2_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_2_other->get_bool()))
					{
						if (target->get_health_percent() <= static_cast< float >( cleanse_spell::cleanse_danger_level_2_hp->get_int() ) || 
							target->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_2_enemy_range->get_int() )) >= cleanse_spell::cleanse_danger_level_2_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_2_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_2_delay->get_int() / 1000.0f ? 0 : time + cleanse_spell::cleanse_danger_level_2_delay->get_int() / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, [target]
									{
										Item mikael(ItemId::Mikaels_Blessing, 650);
										if (mikael.is_ready())
										{
											mikael.cast(target);
										}
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", target->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (mikael_danger == 3)
			{
				if (cleanse_spell::cleanse_danger_level_3_alway_use->get_bool())
				{
					mikael.cast(target);
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_3_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_3_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_3_other->get_bool()))
					{
						if (target->get_health_percent() <= cleanse_spell::cleanse_danger_level_3_hp->get_int() || target->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_3_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_3_enemy_count->get_int())
						{
							if (duration >= static_cast< float >( cleanse_spell::cleanse_danger_level_3_duration->get_int() ) / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > static_cast< float >( cleanse_spell::cleanse_danger_level_3_delay->get_int() ) / 1000.0f ? 0.f : time + static_cast< float >( cleanse_spell::cleanse_danger_level_3_delay->get_int() ) / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, [target]
									{
										Item mikael(ItemId::Mikaels_Blessing, 650);
										if (mikael.is_ready())
										{
											mikael.cast(target);
										}
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", target->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (mikael_danger == 4)
			{
				if (cleanse_spell::cleanse_danger_level_4_alway_use->get_bool())
				{
					mikael.cast(target);
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_4_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_4_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_4_other->get_bool()))
					{
						if (target->get_health_percent() <= cleanse_spell::cleanse_danger_level_4_hp->get_int() || target->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_4_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_4_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_4_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > static_cast< float >( cleanse_spell::cleanse_danger_level_4_delay->get_int() ) / 1000.0f ? 0 : time + cleanse_spell::cleanse_danger_level_4_delay->get_int() / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, [target]
									{
										Item mikael(ItemId::Mikaels_Blessing, 650);
										if (mikael.is_ready())
										{
											mikael.cast(target);
										}
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", target->get_name_cstr());
									});
							}
						}
					}

				}
			}
		}
		else if ((qss_1.is_ready() || qss_2.is_ready() || qss_3.is_ready()) && use_qss == true && target == myhero)
		{
			if (qss_danger == 0)
			{
				return;
			}
			else if (qss_danger == 1)
			{
				if (cleanse_spell::cleanse_danger_level_1_alway_use->get_bool())
				{
					cast_qss();
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_1_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_1_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_1_other->get_bool()))
					{
						if (myhero->get_health_percent() <= cleanse_spell::cleanse_danger_level_1_hp->get_int() || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_1_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_1_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_1_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_1_delay->get_int() / 1000.0f ? 0 : time + cleanse_spell::cleanse_danger_level_1_delay->get_int() / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cast_qss();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (qss_danger == 2)
			{
				if (cleanse_spell::cleanse_danger_level_2_alway_use->get_bool())
				{
					cast_qss();
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_2_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_2_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_2_other->get_bool()))
					{
						if (myhero->get_health_percent() <= cleanse_spell::cleanse_danger_level_2_hp->get_int() || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_2_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_2_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_2_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_2_delay->get_int() / 1000.0f ? 0 : time + static_cast< float >( cleanse_spell::cleanse_danger_level_2_delay->get_int() ) / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cast_qss();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (qss_danger == 3)
			{
				if (cleanse_spell::cleanse_danger_level_3_alway_use->get_bool())
				{
					cast_qss();
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_3_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_3_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_3_other->get_bool()))
					{
						if (myhero->get_health_percent() <= cleanse_spell::cleanse_danger_level_3_hp->get_int() || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_3_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_3_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_3_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_3_delay->get_int() / 1000.0f ? 0 : time + cleanse_spell::cleanse_danger_level_3_delay->get_int() / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cast_qss();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (qss_danger == 4)
			{
				if (cleanse_spell::cleanse_danger_level_4_alway_use->get_bool())
				{
					cast_qss();
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_4_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_4_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_4_other->get_bool()))
					{
						if (myhero->get_health_percent() <= static_cast< float >( cleanse_spell::cleanse_danger_level_4_hp->get_int() ) || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_4_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_4_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_4_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_4_delay->get_int() / 1000.0f ? 0 : time + static_cast< float >( cleanse_spell::cleanse_danger_level_4_delay->get_int() ) / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cast_qss();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
		}
		else if (gametime->get_time() > cast_qss_t + 0.5f && cleanse && cleanse->is_ready() && use_cleanse == true && target == myhero &&
			((myhero->has_item(ItemId::Quicksilver_Sash) != spellslot::invalid && !qss_1.is_ready()) ||
				(myhero->has_item(ItemId::Mercurial_Scimitar) != spellslot::invalid && !qss_2.is_ready()) ||
				(myhero->has_item(ItemId::Silvermere_Dawn) != spellslot::invalid && !qss_3.is_ready()) ||
				(myhero->has_item(ItemId::Quicksilver_Sash) == spellslot::invalid &&
					myhero->has_item(ItemId::Mercurial_Scimitar) == spellslot::invalid &&
					myhero->has_item(ItemId::Silvermere_Dawn) == spellslot::invalid)))
		{
			if (cleanse_danger == 0)
			{
				return;
			}
			else if (cleanse_danger == 1)
			{
				if (cleanse_spell::cleanse_danger_level_1_alway_use->get_bool())
				{
					cleanse->cast();
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_1_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_1_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_1_other->get_bool()))
					{
						if (myhero->get_health_percent() <= static_cast< float >( cleanse_spell::cleanse_danger_level_1_hp->get_int() ) || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_1_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_1_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_1_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_1_delay->get_int() / 1000.0f ? 0 : time + cleanse_spell::cleanse_danger_level_1_delay->get_int() / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cleanse->cast();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (cleanse_danger == 2)
			{
				if (cleanse_spell::cleanse_danger_level_2_alway_use->get_bool())
				{
					cleanse->cast(myhero);
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_2_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_2_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_2_other->get_bool()))
					{
						if (myhero->get_health_percent() <= static_cast< float >( cleanse_spell::cleanse_danger_level_2_hp->get_int() ) || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_2_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_2_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_2_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_2_delay->get_int() / 1000.0f ? 0 : time + static_cast< float >( cleanse_spell::cleanse_danger_level_2_delay->get_int()) / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cleanse->cast();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (cleanse_danger == 3)
			{
				if (cleanse_spell::cleanse_danger_level_3_alway_use->get_bool())
				{
					cleanse->cast(myhero);
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_3_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_3_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_3_other->get_bool()))
					{
						if (myhero->get_health_percent() <= static_cast< float >( cleanse_spell::cleanse_danger_level_3_hp->get_int() ) || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_3_enemy_range->get_int())) >= cleanse_spell::cleanse_danger_level_3_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_3_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_3_delay->get_int() / 1000.0f ? 0 : time + cleanse_spell::cleanse_danger_level_3_delay->get_int() / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cleanse->cast();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
			else if (cleanse_danger == 4)
			{
				if (cleanse_spell::cleanse_danger_level_4_alway_use->get_bool())
				{
					cleanse->cast();
				}
				else
				{
					if ((orbwalker->combo_mode() && cleanse_spell::cleanse_danger_level_4_combo->get_bool()) || (orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_4_harass->get_bool()) || (!orbwalker->combo_mode() && !orbwalker->mixed_mode() && cleanse_spell::cleanse_danger_level_4_other->get_bool()))
					{
						if (myhero->get_health_percent() <= static_cast<float>( cleanse_spell::cleanse_danger_level_4_hp->get_int() ) || myhero->get_position().count_enemies_in_range( static_cast< float >( cleanse_spell::cleanse_danger_level_4_enemy_range->get_int() )) >= cleanse_spell::cleanse_danger_level_4_enemy_count->get_int())
						{
							if (duration >= cleanse_spell::cleanse_danger_level_4_duration->get_int() / 1000.0f)
							{
								float clean_time = gametime->get_time() - time > cleanse_spell::cleanse_danger_level_4_delay->get_int() / 1000.0f ? 0 : time + static_cast< float >( cleanse_spell::cleanse_danger_level_4_delay->get_int() ) / 1000.0f - gametime->get_time();
								//myhero->print_chat(1, "clean time : %f", clean_time);
								scheduler->delay_action(clean_time, []
									{
										cleanse->cast();
										//myhero->print_chat(1, "clean time %f", gametime->get_time() - time);
										//myhero->print_chat(1, "%s !!", myhero->get_name_cstr());
									});
							}
						}
					}

				}
			}
		}
	}
	void cc_logic()
	{
		if ( cleanse_spell::mikael_heal->get_bool( ) )
		{
			Item mikael( ItemId::Mikaels_Blessing, 650 );
			if ( mikael.is_vaild( ) && mikael.is_ready( ) )
			{
				auto ally = entitylist->get_ally_heroes( );
				ally.erase( std::remove_if( ally.begin( ), ally.end( ), [ ]( game_object_script x )
				{
					if ( !x || !x->is_valid( ) || x->get_distance( myhero ) > 650 || x->is_dead( ) )
						return true;

					auto it = cleanse_spell::mikael_heal_on.find( x->get_champion( ) );
					if ( it == cleanse_spell::mikael_heal_on.end( ) || !it->second->get_bool( ) )
						return true;

					auto damage = health_prediction->get_incoming_damage( x, 0.5f, false );
					if ( damage <= 0.f || ( x->get_health( ) - damage ) / x->get_max_health( ) > static_cast< float >( cleanse_spell::mikael_heal_hp->get_int( ) ) / 100.f )
						return true;

					return false;
				} ), ally.end( ) );

				if ( !ally.empty( ) )
				{
					std::sort( ally.begin( ), ally.end( ), [ ]( game_object_script x1, game_object_script x2 ) {
						return x1->get_health( ) < x2->get_health( );
					} );

					mikael.cast( ally.front( ) );
					return;
				}
			}
		}

		if (cleanse_spell::mikael->get_bool())
		{
			auto ally = entitylist->get_ally_heroes();

			ally.erase(std::remove_if(ally.begin(), ally.end(), [](game_object_script x)
				{
					return !x || !x->is_valid( ) || x->get_distance(myhero) > 650 || x->is_dead() || !A_::checkbox(cleanse_spell::mikael_use_on, x) ||
						x->has_buff_type(buff_type::Knockup) || x->has_buff_type(buff_type::Knockback) ||
						x->has_buff(buff_hash("rellw_knockup")) || x->has_buff(buff_hash("gragasestun")) || x->has_buff(buff_hash("headbutttarget" ) );
				}), ally.end());

			for (auto&& ally : ally)
			{
				//cleanse_spell::Nasusw_qss
				if ( ally->has_buff( buff_hash( "NasusW" ) ) )
				{
					auto buff = ally->get_buff( buff_hash( "NasusW" ) );
					if ( buff != nullptr && buff->is_valid( ) && buff->is_alive( ) )
					{
						if ( cc_time_loop < gametime->get_time( ) )
						{
							Nasusw_time = gametime->get_time( );
							cc_time_loop = gametime->get_time( ) + 0.2f;
						}
						if ( cc_clean_loop < gametime->get_time( ) )
						{
							clean_cc_logic( Nasusw_time, buff->get_remaining_time( ), cleanse_spell::cleanse->get_bool( ), cleanse_spell::Nasusw_cleanse->get_int( ), cleanse_spell::qss->get_bool( ), cleanse_spell::Nasusw_cleanse->get_int( ), cleanse_spell::mikael->get_bool( ), ally, cleanse_spell::Berserk_mikael->get_int( ) );
							cc_clean_loop = gametime->get_time( ) + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Berserk))
				{
					auto buff = ally->get_buff_by_type(buff_type::Berserk);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Berserk_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Berserk_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Berserk_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Berserk_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Berserk_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Stun))
				{
					auto buff = ally->get_buff_by_type(buff_type::Stun);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Stun_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
							//myhero->print_chat(1, "%f   %f", Stun_time, buff->get_remaining_time());
							//myhero->print_chat(1, "%f %f", cc_time, gametime->get_time());
							//myhero->print_chat(1, "Stun : %f %s !!", buff->get_remaining_time(), ally->get_name_cstr());
							//clean_cc_logic(Stun_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Stun_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Stun_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Stun_mikael->get_int());
						}
						//myhero->print_chat(1, "%f   %f", Stun_time, buff->get_remaining_time());
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Stun_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Stun_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Stun_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Stun_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Taunt))
				{
					auto buff = ally->get_buff_by_type(buff_type::Taunt);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Taunt_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Taunt_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Taunt_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Taunt_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Taunt_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Polymorph))
				{
					auto buff = ally->get_buff_by_type(buff_type::Polymorph);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Polymorph_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Polymorph_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Polymorph_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Polymorph_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Polymorph_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Fear) || ally->has_buff_type(buff_type::Flee))
				{
					auto buff_fear = ally->get_buff_by_type(buff_type::Fear);
					bool fear = buff_fear && buff_fear->is_valid() && buff_fear->is_alive();
					auto buff_flee = ally->get_buff_by_type(buff_type::Flee);
					bool flee = buff_flee && buff_flee->is_valid() && buff_flee->is_alive();
					float buff_t = 0.f;
					if (flee)
					{
						buff_t = buff_flee->get_remaining_time();
					}
					else if (fear)
					{
						buff_t = buff_fear->get_remaining_time();
					}
					if (flee || fear)
					{
						if (cc_time_loop < gametime->get_time())
						{
							Fear_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Fear_time, buff_t, cleanse_spell::cleanse->get_bool(), cleanse_spell::Fear_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Fear_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Fear_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Charm))
				{
					auto buff = ally->get_buff_by_type(buff_type::Charm);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Charm_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Charm_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Charm_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Charm_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Charm_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Drowsy))
				{
					auto buff = ally->get_buff_by_type(buff_type::Drowsy);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Drowsy_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Drowsy_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Drowsy_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Drowsy_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Drowsy_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Asleep))
				{
					auto buff = ally->get_buff_by_type(buff_type::Asleep);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Asleep_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Asleep_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Asleep_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Asleep_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Asleep_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Snare))
				{
					auto buff = ally->get_buff_by_type(buff_type::Snare);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Snare_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Snare_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Snare_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Snare_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Snare_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Slow) && ally->get_move_speed() < 250)
				{
					auto buff = ally->get_buff_by_type(buff_type::Slow);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Slow_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Slow_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Slow_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Slow_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Slow_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (ally->has_buff_type(buff_type::Silence))
				{
					auto buff = ally->get_buff_by_type(buff_type::Silence);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Silence_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Silence_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Silence_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Silence_qss->get_int(), cleanse_spell::mikael->get_bool(), ally, cleanse_spell::Silence_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
			}

		}
		if (cleanse_spell::qss->get_bool() || cleanse_spell::cleanse->get_bool())
		{
			buff_instance_script buff = nullptr;
			for (auto&& have_buff : myhero->get_bufflist())
			{
				if (have_buff->is_valid() && have_buff->is_alive() && have_buff->get_name() == "Stun")
				{
					buff = have_buff;
				}
			}
			bool gragas_buff = myhero->has_buff_type(buff_type::Stun) && buff && buff->get_caster()->get_champion() == champion_id::Gragas;
			bool cow_buff = myhero->has_buff( buff_hash( "headbutttarget" ) );
			bool leona_buff = myhero->has_buff(buff_hash("LeonaSunlight" ) ) || myhero->has_buff(buff_hash("leonazenithbladeroo" ) );
			bool blitzcrank_buff = myhero->has_buff( buff_hash( "rocketgrab2" ) );
			if (!myhero->has_buff_type(buff_type::Knockup) && !myhero->has_buff_type(buff_type::Knockback) &&
				!myhero->has_buff(buff_hash("rellw_knockup")) && !gragas_buff && !cow_buff && !leona_buff && !blitzcrank_buff )
			{
				if ( myhero->has_buff( buff_hash( "NasusW" ) ) )
				{
					auto buff = myhero->get_buff( buff_hash( "NasusW" ) );
					if ( buff != nullptr && buff->is_valid( ) && buff->is_alive( ) )
					{
						if ( cc_time_loop < gametime->get_time( ) )
						{
							Nasusw_time = gametime->get_time( );
							cc_time_loop = gametime->get_time( ) + 0.2f;
						}
						if ( cc_clean_loop < gametime->get_time( ) )
						{
							clean_cc_logic( Nasusw_time, buff->get_remaining_time( ), cleanse_spell::cleanse->get_bool( ), cleanse_spell::Nasusw_cleanse->get_int( ), cleanse_spell::qss->get_bool( ), 0, cleanse_spell::mikael->get_bool( ), myhero, 0 );
							cc_clean_loop = gametime->get_time( ) + 0.1f;
						}
					}
				}
				if (myhero->has_buff(buff_hash("SummonerExhaust")))
				{
					auto buff = myhero->get_buff(buff_hash("SummonerExhaust"));
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Exhaust_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Exhaust_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Exhaust_cleanse->get_int(), cleanse_spell::qss->get_bool(), 0, cleanse_spell::mikael->get_bool(), myhero, 0);
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff(buff_hash("SummonerDot")))
				{
					auto buff = myhero->get_buff(buff_hash("SummonerDot"));
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Ignite_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Ignite_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Ignite_cleanse->get_int(), cleanse_spell::qss->get_bool(), 0, cleanse_spell::mikael->get_bool(), myhero, 0);
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Stun))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Stun);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Stun_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
							//myhero->print_chat(1, "%f   %f", Stun_time, buff->get_remaining_time());
							//myhero->print_chat(1, "%f %f", cc_time, gametime->get_time());
							//myhero->print_chat(1, "Stun : %f %s !!", buff->get_remaining_time(), myhero->get_name_cstr());
							//clean_cc_logic(Stun_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Stun_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Stun_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Stun_mikael->get_int());
						}
						//myhero->print_chat(1, "%f   %f", Stun_time, buff->get_remaining_time());
						if (cc_clean_loop < gametime->get_time())
						{
							//myhero->print_chat(1, "%f", buff->get_remaining_time());
							clean_cc_logic(Stun_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Stun_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Stun_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Stun_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Berserk))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Berserk);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Berserk_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Berserk_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Berserk_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Berserk_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Berserk_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Taunt))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Taunt);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Taunt_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Taunt_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Taunt_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Taunt_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Taunt_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Polymorph))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Polymorph);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Polymorph_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Polymorph_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Polymorph_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Polymorph_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Polymorph_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Fear) || myhero->has_buff_type(buff_type::Flee))
				{
					auto buff_fear = myhero->get_buff_by_type(buff_type::Fear);
					bool fear = buff_fear && buff_fear->is_valid() && buff_fear->is_alive();
					auto buff_flee = myhero->get_buff_by_type(buff_type::Flee);
					bool flee = buff_flee && buff_flee->is_valid() && buff_flee->is_alive();
					float buff_t = 0.f;
					if (flee)
					{
						buff_t = buff_flee->get_remaining_time();
					}
					else if (fear)
					{
						buff_t = buff_fear->get_remaining_time();
					}
					if (flee || fear)
					{
						if (cc_time_loop < gametime->get_time())
						{
							Fear_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Fear_time, buff_t, cleanse_spell::cleanse->get_bool(), cleanse_spell::Fear_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Fear_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Fear_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Charm))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Charm);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Charm_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Charm_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Charm_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Charm_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Charm_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Drowsy))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Drowsy);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Drowsy_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Drowsy_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Drowsy_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Drowsy_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Drowsy_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Asleep))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Asleep);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Asleep_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Asleep_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Asleep_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Asleep_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Asleep_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Snare))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Snare);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Snare_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Snare_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Snare_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Snare_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Snare_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Slow) && myhero->get_move_speed() < 250)
				{
					auto buff = myhero->get_buff_by_type(buff_type::Slow);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Slow_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Slow_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Slow_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Slow_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Slow_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Blind))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Blind);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Blind_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Blind_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Blind_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Blind_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, 0);
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Disarm))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Disarm);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Disarm_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Disarm_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Disarm_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Disarm_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, 0);
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Silence))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Silence);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Silence_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Silence_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::Silence_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::Silence_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, cleanse_spell::Silence_mikael->get_int());
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::NearSight))
				{
					auto buff = myhero->get_buff_by_type(buff_type::NearSight);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							NearSight_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(NearSight_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), cleanse_spell::NearSight_cleanse->get_int(), cleanse_spell::qss->get_bool(), cleanse_spell::NearSight_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, 0);
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
				if (myhero->has_buff_type(buff_type::Suppression))
				{
					auto buff = myhero->get_buff_by_type(buff_type::Suppression);
					if (buff != nullptr && buff->is_valid() && buff->is_alive())
					{
						if (cc_time_loop < gametime->get_time())
						{
							Suppression_time = gametime->get_time();
							cc_time_loop = gametime->get_time() + 0.2f;
						}
						if (cc_clean_loop < gametime->get_time())
						{
							clean_cc_logic(Suppression_time, buff->get_remaining_time(), cleanse_spell::cleanse->get_bool(), 0, cleanse_spell::qss->get_bool(), cleanse_spell::Suppression_qss->get_int(), cleanse_spell::mikael->get_bool(), myhero, 0);
							cc_clean_loop = gametime->get_time() + 0.1f;
						}
					}
				}
			}
		}
	}
#pragma endregion

	float spell_time;
#pragma region ignite
	void ignite_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_ignite = summoner->add_tab("ignite_menu", "Ignite");
		if (summoner_ignite)
		{
			/*if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerDot"))
				summoner_ignite->set_texture(myhero->get_spell(spellslot::summoner1)->get_icon_texture());
			else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerDot"))
				summoner_ignite->set_texture(myhero->get_spell(spellslot::summoner2)->get_icon_texture());*/

			auto db_summoner_dot = database->get_spell_by_hash( spell_hash( "SummonerDot" ) );
			if ( db_summoner_dot )
			{
				auto dot_texture = db_summoner_dot->get_icon_texture_by_index( 0 );
				if ( dot_texture )
					summoner_ignite->set_texture( dot_texture );
			}

			summoner_ignite->add_separator("ignite1", " - - Ignite - - ");

			use_ignite::key = summoner_ignite->add_hotkey("key", "Use semi ignite", TreeHotkeyMode::Hold, 'F', false);
			use_ignite::type = summoner_ignite->add_combobox("type", "Type", { {"Low hp",nullptr},{"Target selector",nullptr },{"Close to me",nullptr },{"Close to mouse",nullptr } }, 0, true);

			summoner_ignite->add_separator("ignite2", " - - Auto - - ");

			use_ignite::ignite = summoner_ignite->add_checkbox("ignite", "Ignite", true, true);
			summoner_ignite->set_assigned_active(use_ignite::ignite);
			use_ignite::dont_over_kill = summoner_ignite->add_checkbox("dont_over_kill", "Dont over kill", true, true);
			use_ignite::c = summoner_ignite->add_checkbox("c ", "Use in combo", true, true);
			use_ignite::harass = summoner_ignite->add_checkbox("harass ", "Use in harass", true, true);
			use_ignite::other = summoner_ignite->add_checkbox("other ", "Use in other mode", true, true);
			use_ignite::rang = summoner_ignite->add_slider("rang", "Use if enemy x range", 500, 1, 1000);
			use_ignite::enemy = summoner_ignite->add_slider("enemy", "  ^- ally <= x", 2, 0, 6);

			summoner_ignite->add_separator("ignite3", " - - Use to - - ");

			for (auto& ally : entitylist->get_enemy_heroes())
			{
				use_ignite::use_to[ally->get_network_id()] = summoner_ignite->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, true);
				use_ignite::use_to[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
			}
		}
	}
	void ignite_logic()
	{
		if (!ignite || !ignite->is_ready() || !use_ignite::ignite->get_bool()) return;

		bool ignite_c = use_ignite::c->get_bool() && orbwalker->combo_mode();
		bool ignite_h = use_ignite::harass->get_bool() && orbwalker->mixed_mode();
		bool ignite_other = use_ignite::other->get_bool() && !orbwalker->mixed_mode() && !orbwalker->combo_mode();
		if (!ignite_c && !ignite_h && !ignite_other) return;

		auto enemy = entitylist->get_enemy_heroes();

		enemy.erase(std::remove_if(enemy.begin(), enemy.end(), [](game_object_script x)
			{
				float ignite_dmg = 50.f + 20.f * static_cast< float >( myhero->get_level() );
				return !x->is_valid() || x->is_me( ) || x->get_distance(myhero) > 600 || x->is_dead() || !A_::checkbox(use_ignite::use_to, x) ||
					x->get_health() > ignite_dmg * 0.9 ||
					(use_ignite::dont_over_kill->get_bool() && x->get_health() < myhero->get_auto_attack_damage(x) * 2.f) ||
					x->count_enemies_in_range(static_cast<float>( use_ignite::rang->get_int() )) > use_ignite::enemy->get_int();
			}), enemy.end());

		std::sort(enemy.begin(), enemy.end(), [](game_object_script a, game_object_script b)
			{
				return a->get_health() < b->get_health();
			});

		if (!enemy.empty() && gametime->get_time() > spell_time)
		{
			spell_time = gametime->get_time() + 0.5f;
			ignite->cast(enemy.front());
			return;
		}
	}
	void semi_ignite()
	{
		if (!ignite || !ignite->is_ready() || !use_ignite::key->get_bool()) return;

		auto enemy = entitylist->get_enemy_heroes();

		enemy.erase(std::remove_if(enemy.begin(), enemy.end(), [](game_object_script x)
			{
				return !x->is_valid() || x->is_me( ) || x->get_distance( myhero ) > 600 || x->is_dead( ) || !A_::checkbox( use_ignite::use_to, x );
			}), enemy.end());

		if (gametime->get_time() > spell_time)
		{
			if (use_ignite::type->get_int() == 0)
			{
				std::sort(enemy.begin(), enemy.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_health() < b->get_health();
					});

				if (!enemy.empty())
				{
					ignite->cast(enemy.front());
					spell_time = gametime->get_time() + 0.5f;
					return;
				}
			}
			else if (use_ignite::type->get_int() == 1)
			{
				auto target = target_selector->get_target(600, damage_type::true_dmg);
				if (target != nullptr)
				{
					if (ignite->cast(target))
					{
						spell_time = gametime->get_time() + 0.5f;
						return;
					}
				}
			}
			else if (use_ignite::type->get_int() == 2)
			{
				std::sort(enemy.begin(), enemy.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_distance(myhero) < b->get_distance(myhero);
					});

				if (!enemy.empty())
				{
					ignite->cast(enemy.front());
					spell_time = gametime->get_time() + 0.5f;
					return;
				}
			}
			else if (use_ignite::type->get_int() == 3)
			{
				std::sort(enemy.begin(), enemy.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_distance(hud->get_hud_input_logic()->get_game_cursor_position()) < b->get_distance(hud->get_hud_input_logic()->get_game_cursor_position());
					});

				if (!enemy.empty())
				{
					ignite->cast(enemy.front());
					spell_time = gametime->get_time() + 0.5f;
					return;
				}
			}
		}
	}
#pragma endregion

#pragma region heal
	void heal_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_heal = summoner->add_tab("heal_menu", "Heal");
		if (summoner_heal)
		{
			/*if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerHeal"))
				summoner_heal->set_texture(myhero->get_spell(spellslot::summoner1)->get_icon_texture());
			else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerHeal"))
				summoner_heal->set_texture(myhero->get_spell(spellslot::summoner2)->get_icon_texture());*/

			auto db_summoner_heal = database->get_spell_by_hash( spell_hash( "SummonerHeal" ) );
			if ( db_summoner_heal )
			{
				auto heal_texture = db_summoner_heal->get_icon_texture_by_index( 0 );
				if ( heal_texture )
					summoner_heal->set_texture( heal_texture );
			}

			summoner_heal->add_separator("heal1", " - - Heal - - ");

			use_heal::heal = summoner_heal->add_checkbox("heal ", "Heal", true, true);
			summoner_heal->set_assigned_active(use_heal::heal);

			summoner_heal->add_separator("heal2", " - - Myhero - - ");
			use_heal::hp = summoner_heal->add_slider("hp", "HP <= x%", 10, 0, 100);
			use_heal::calc_hp = summoner_heal->add_slider("calc_hp", "Incoming damage HP <= x%", 5, 0, 100);

			summoner_heal->add_separator("heal3", " - - Ally - - ");
			for (auto& ally : entitylist->get_ally_heroes())
			{
				if (ally->is_valid()  &&!ally->is_me())
				{
					use_heal::ally[ally->get_network_id()] = summoner_heal->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, true);
					use_heal::ally[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
				}
			}
			use_heal::ally_hp = summoner_heal->add_slider("ally_hp", "HP <= x%", 10, 0, 100);
			use_heal::ally_calc_hp = summoner_heal->add_slider("ally_calc_hp", "Incoming damage HP <= x%", 5, 0, 100);
			summoner_heal->add_separator("heal4", " - - Other - - ");
			use_heal::level = summoner_heal->add_slider("level", "Level > x", 0, 0, 6);
			use_heal::dont_use_dead = summoner_heal->add_checkbox("dont_use_dead ", "Dont use heal if dmg > hp + heal", false, true);
			use_heal::c = summoner_heal->add_checkbox("c ", "Use in combo", true, true);
			use_heal::harass = summoner_heal->add_checkbox("harass ", "Use in harass", true, true);
			use_heal::other = summoner_heal->add_checkbox("other ", "Use in other mode", true, true);
			use_heal::rang = summoner_heal->add_slider("rang", "Use if x range", 1000, 1, 1000);
			use_heal::enemy = summoner_heal->add_slider("enemy", "  ^- enemy >= x", 1, 0, 6);
		}
	}
	float healt_time;
	void heal_me()
	{
		if (!heal || !heal->is_ready()) return;

		auto heal_hp = use_heal::hp->get_int();
		auto calc_hp = myhero->get_max_health() * use_heal::calc_hp->get_int() / 100.0f;
		auto income_hp = myhero->get_health() - health_prediction->get_incoming_damage(myhero, 1.0f, true);
		if (myhero->get_health_percent() > heal_hp && income_hp > calc_hp) return;

		auto dead_hp = myhero->get_health() - health_prediction->get_incoming_damage(myhero, 1.0f, false);
		auto heal_count = static_cast<float>( 66 + 14 * myhero->get_level() );
		if (dead_hp + heal_count < 0 && use_heal::dont_use_dead->get_bool()) return;

		bool heal_c = use_heal::c->get_bool() && orbwalker->combo_mode();
		bool heal_h = use_heal::harass->get_bool() && orbwalker->mixed_mode();
		bool heal_other = use_heal::other->get_bool() && !orbwalker->mixed_mode() && !orbwalker->combo_mode();
		if (!heal_c && !heal_h && !heal_other) return;

		if (myhero->get_level() <= use_heal::level->get_int()) return;

		if (myhero->count_enemies_in_range(static_cast<float>( use_heal::rang->get_int( ) ) ) >= use_heal::enemy->get_int( ) && gametime->get_time() > healt_time)
		{
			if (heal->cast())
			{
				healt_time = gametime->get_time() + 0.5f;
				return;
			}
		}
	}
	void heal_ally()
	{
		if (!heal || !heal->is_ready()) return;

		auto team = entitylist->get_ally_heroes();

		team.erase(std::remove_if(team.begin(), team.end(), [](game_object_script x)
			{
				return !x->is_valid() || x->is_me( ) || x->get_distance( myhero ) > 850 || x->is_dead( ) || !A_::checkbox( use_heal::ally, x );
			}), team.end());

		if (!team.empty())
		{
			for (auto team : team)
			{
				auto heal_hp = static_cast<float>( use_heal::ally_hp->get_int() );
				auto calc_hp = team->get_max_health() * use_heal::ally_calc_hp->get_int() / 100.0f;
				auto income_hp = team->get_health() - health_prediction->get_incoming_damage(team, 1.0f, true);
				if (team->get_health_percent() > heal_hp && income_hp > calc_hp) return;

				auto dead_hp = team->get_health() - health_prediction->get_incoming_damage(team, 1.0f, false);
				auto heal_count = 66 + 14 * myhero->get_level();
				if (dead_hp + static_cast<float>( heal_count ) < 0 && use_heal::dont_use_dead->get_bool()) return;

				bool heal_c = use_heal::c->get_bool() && orbwalker->combo_mode();
				bool heal_h = use_heal::harass->get_bool() && orbwalker->mixed_mode();
				bool heal_other = use_heal::other->get_bool() && !orbwalker->mixed_mode() && !orbwalker->combo_mode();
				if (!heal_c && !heal_h && !heal_other) return;

				if (myhero->get_level() <= use_heal::level->get_int()) return;

				if (team->count_enemies_in_range(static_cast<float>( use_heal::rang->get_int( )  )) >= use_heal::enemy->get_int( ) && gametime->get_time() > healt_time)
				{
					if (heal->cast())
					{
						healt_time = gametime->get_time() + 0.5f;
						return;
					}
				}
			}
		}
	}
	void heal_logic()
	{
		if (!use_heal::heal->get_bool()) return;
		heal_me();
		heal_ally();
	}
#pragma endregion

#pragma region barrier
	void barrier_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_barrier = summoner->add_tab("barrier_menu", "Barrier");
		if (summoner_barrier)
		{
			auto db_summoner_barrier = database->get_spell_by_hash( spell_hash( "SummonerBarrier" ) );
			if ( db_summoner_barrier )
			{
				auto barrier_texture = db_summoner_barrier->get_icon_texture_by_index( 0 );
				if ( barrier_texture )
					summoner_barrier->set_texture( barrier_texture );
			}

			summoner_barrier->add_separator("barrier1", " - - Barrier - - ");

			use_barrier::barrier = summoner_barrier->add_checkbox("barrier", "Barrier", true, true);
			summoner_barrier->set_assigned_active(use_barrier::barrier);

			use_barrier::hp = summoner_barrier->add_slider("hp", "HP <= x%", 10, 0, 100);
			use_barrier::calc_hp = summoner_barrier->add_slider("calc_hp", "Incoming damage HP <= x%", 5, 0, 100);

			summoner_barrier->add_separator("barrier2", " - - Other - - ");
			use_barrier::level = summoner_barrier->add_slider("level", "Level > x", 0, 0, 6);
			use_barrier::dont_use_dead = summoner_barrier->add_checkbox("dont_use_dead ", "Dont use barrier if dmg > hp + heal", false, true);
			use_barrier::c = summoner_barrier->add_checkbox("c ", "Use in combo", true, true);
			use_barrier::harass = summoner_barrier->add_checkbox("harass ", "Use in harass", true, true);
			use_barrier::other = summoner_barrier->add_checkbox("other ", "Use in other mode", true, true);
			use_barrier::rang = summoner_barrier->add_slider("rang", "Use if x range", 1000, 1, 1000);
			use_barrier::enemy = summoner_barrier->add_slider("enemy", "  ^- enemy >= x", 1, 0, 6);
		}
	}
	void barrier_logic()
	{
		if (!barrier || !barrier->is_ready() || !use_barrier::barrier->get_bool()) return;

		auto barrier_hp = static_cast<float>( use_barrier::hp->get_int() );
		auto calc_hp = myhero->get_max_health() * use_barrier::calc_hp->get_int() / 100.0f;
		auto income_hp = myhero->get_health() - health_prediction->get_incoming_damage(myhero, 1.0f, true);
		if (myhero->get_health_percent() > barrier_hp && income_hp > calc_hp) return;

		auto dead_hp = myhero->get_health() - health_prediction->get_incoming_damage(myhero, 1.0f, false);
		auto barrier_count = static_cast<float>( 87 + 18 * myhero->get_level() );
		if (dead_hp + barrier_count < 0 && use_barrier::dont_use_dead->get_bool()) return;

		bool barrier_c = use_barrier::c->get_bool() && orbwalker->combo_mode();
		bool barrier_h = use_barrier::harass->get_bool() && orbwalker->mixed_mode();
		bool barrier_other = use_barrier::other->get_bool() && !orbwalker->mixed_mode() && !orbwalker->combo_mode();
		if (!barrier_c && !barrier_h && !barrier_other) return;

		if (myhero->get_level() <= use_barrier::level->get_int()) return;

		if (myhero->count_enemies_in_range( static_cast< float >( use_barrier::rang->get_int( ) )) >= use_barrier::enemy->get_int( ) && gametime->get_time() > spell_time)
		{
			if (barrier->cast())
			{
				spell_time = gametime->get_time() + 0.5f;
				return;
			}
		}
	}
#pragma endregion

#pragma region flash
	void flash_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_flash = summoner->add_tab("flash_menu", "Flash");
		if (summoner_flash)
		{
			/*if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
				summoner_flash->set_texture(myhero->get_spell(spellslot::summoner1)->get_icon_texture());
			else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
				summoner_flash->set_texture(myhero->get_spell(spellslot::summoner2)->get_icon_texture());*/

			auto db_summoner_flash = database->get_spell_by_hash( spell_hash( "SummonerFlash" ) );
			if ( db_summoner_flash )
			{
				auto f_texture = db_summoner_flash->get_icon_texture_by_index( 0 );
				if ( f_texture )
					summoner_flash->set_texture( f_texture );
			}

			summoner_flash->add_separator("flash1", " - - Flash - - ");

			use_flash::use_flash = summoner_flash->add_checkbox("use_flash", "Use flash", false, true);

			summoner_flash->add_separator("flash2", " - - Setting - - ");

			//use_flash::anti_gapclose = summoner_flash->add_checkbox("anti_gapclose", "Flash anti gapcloser", false, true);
			use_flash::anti_flash = summoner_flash->add_checkbox("anti_flash", "Anti Flash", false, true);
			summoner_flash->set_assigned_active(use_flash::use_flash);
			summoner_flash->add_separator("flash3", " - - Anti Flash to - - ");

			for (auto& ally : entitylist->get_enemy_heroes())
			{
				use_flash::use_to[ally->get_network_id()] = summoner_flash->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, true);
				use_flash::use_to[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
				use_flash::anti_range[ally->get_network_id()] = summoner_flash->add_slider( "anti_range" + ally->get_model( ), "Enemy close me <= x range", 400, 200, 800 );
				use_flash::anti_hp[ally->get_network_id()] = summoner_flash->add_slider( "anti_hp" + ally->get_model( ), "Hp <= x%", 87, 0, 100 );
			}
		}
	}
#pragma endregion

#pragma region snowball
	void snowball_menu()
	{
		auto main_tab = menu->get_tab("utility");
		auto summoner = main_tab->get_tab("summoner");
		auto summoner_snowball = summoner->add_tab("snowball_menu", "Snow ball");
		if (summoner_snowball)
		{
			/*if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerSnowball"))
				summoner_snowball->set_texture(myhero->get_spell(spellslot::summoner1)->get_icon_texture());
			else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerSnowball"))
				summoner_snowball->set_texture(myhero->get_spell(spellslot::summoner2)->get_icon_texture());*/

			auto db_summoner_snow = database->get_spell_by_hash( spell_hash( "SummonerSnowball" ) );
			if ( db_summoner_snow )
			{
				auto snow_texture = db_summoner_snow->get_icon_texture_by_index( 0 );
				if ( snow_texture )
					summoner_snowball->set_texture( snow_texture );
			}

			summoner_snowball->add_separator("snowball0", " - - Snowball - - ");
			use_snowball::semi_key = summoner_snowball->add_hotkey("snowball_semi_key", "Semi snowball", TreeHotkeyMode::Hold, 0, false);

			summoner_snowball->add_separator("snowball1", " - - Auto - - ");
			use_snowball::use = summoner_snowball->add_checkbox("use_snowball", "Snowball", true, true);
			summoner_snowball->set_assigned_active(use_snowball::use);
			use_snowball::range = summoner_snowball->add_slider("range", "Use range", 700, 300, 1600);
			use_snowball::hp = summoner_snowball->add_slider("hp", "Enemy hp <= x", 15, 0, 100);

			summoner_snowball->add_separator("snowball2", " - - Use to - - ");

			for (auto& ally : entitylist->get_enemy_heroes())
			{
				use_snowball::use_to[ally->get_network_id()] = summoner_snowball->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, true);
				use_snowball::use_to[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
			}
		}
	}
	void cast_snow_ball(script_spell* spell, game_object_script target)
	{
		prediction_input x;

		x._from = myhero->get_position();
		x._range_check_from = myhero->get_position();
		x.unit = target;
		x.delay = 0.0f;
		x.radius = 50;
		x.speed = 1250;
		x.collision_objects = { collisionable_objects::minions, collisionable_objects::heroes };//collisionable_objects::minions, collisionable_objects::heroes
		x.range = 1600;
		x.type = skillshot_type::skillshot_line;
		x.aoe = true;
		x.spell_slot = spell->slot;
		x.use_bounding_radius = false;

		auto pred = prediction->get_prediction(&x);

		if (pred.hitchance >= hit_chance::high)
		{
			if (spell->cast(pred.get_cast_position()))
			{
				return;
			}
		}
	}
	float snow_loop;
	void snowball_logic()
	{
		if (!snowball || !snowball->is_ready() || !use_snowball::use->get_bool() || snow_loop > gametime->get_time() || myhero->has_buff(buff_hash("snowballfollowupself"))) return;
		auto target = target_selector->get_target( static_cast< float >( use_snowball::range->get_int() ), damage_type::true_dmg);
		if (target && A_::checkbox(use_snowball::use_to, target) && target->get_health_percent() < use_snowball::hp->get_int())
		{
			snow_loop = gametime->get_time() + 0.3f;
			cast_snow_ball(snowball, target);
		}
	}
	void semi_snowball()
	{
		if (!snowball || !snowball->is_ready() || !use_snowball::semi_key->get_bool() || snow_loop > gametime->get_time() || myhero->has_buff(buff_hash("snowballfollowupself"))) return;

		auto target = target_selector->get_target(1600, damage_type::true_dmg);
		if (target)
		{
			snow_loop = gametime->get_time() + 0.3f;
			cast_snow_ball(snowball, target);
		}
	}
#pragma endregion

	void flash_gapcloser(script_spell* spell, game_object_script sender)
	{
		if (all_spell::use->get_bool( ) ) return;
		if (!spell->is_ready() || !spell) return;
		auto pos = myhero->get_position().extend(sender->get_position(), -1000);
		spell->cast(pos);
		return;
	}
	void OnProcessSpellCast(game_object_script sender, spell_instance_script spell)
	{
		if ( !sender || !sender->is_valid( ) || !spell || !spell->get_spell_data( ) )
			return;

		auto spell_hash = spell->get_spell_data()->get_name_hash();
		if (sender->is_enemy())
		{
			if (spell_hash == spell_hash("TwitchExpunge"))
			{
				twitch_use_e = gametime->get_time() + 2.45f - ping->get_ping() / 1000.0f;
				auto buff = myhero->get_buff(buff_hash("TwitchDeadlyVenom"));
				if (buff != nullptr && buff->is_valid() && buff->is_alive())
				{
					count_twitch_e = buff->get_count();
				}
			}
			if (spell_hash == spell_hash("XayahE"))
			{
				xayah_use_e = gametime->get_time() + 0.5f - ping->get_ping() / 1000.0f;
			}
			if (spell_hash == spell_hash("TwitchExpunge"))
			{
				twitch_use_e = gametime->get_time() + 2.45f - ping->get_ping() / 1000.0f;
				auto buff = myhero->get_buff(buff_hash("TwitchDeadlyVenom"));
				if (buff != nullptr && buff->is_valid() && buff->is_alive())
				{
					count_twitch_e = buff->get_count();
				}
			}
			if (spell_hash == spell_hash("SummonerFlash") && !all_spell::use->get_bool( ) )
			{
				auto dis = myhero->get_distance( sender );
				auto pos = spell->get_start_position( ).distance( spell->get_end_position( ) ) > dis ? spell->get_start_position( ).extend( spell->get_end_position( ), dis - 100 ) : spell->get_end_position( );
				
				if (use_flash::use_flash->get_bool() && use_flash::anti_flash->get_bool() && flash && flash->is_ready() &&
					A_::checkbox(use_flash::use_to, sender) )
					//&& myhero->get_distance( pos ) < use_flash::anti_range->get_int( ) && myhero->get_health_percent( ) - 3 <= use_flash::anti_hp->get_int( )
				{
					auto hp_check = A_::get_slider( use_flash::anti_hp, sender ) >= myhero->get_health_percent( ) - 1;
					auto dis_check = A_::get_slider( use_flash::anti_range, sender ) >= myhero->get_distance( pos );

					//myhero->print_chat( 1, "%d, %f", A_::get_slider( use_flash::anti_hp, sender ), myhero->get_health_percent( ) );
					//myhero->print_chat( 1, "%d, %f", A_::get_slider( use_flash::anti_range, sender ), myhero->get_distance( pos ) );
					if ( hp_check && dis_check )
					{
						auto flash_pos = myhero->get_position( ).extend( pos, -1000 );
						flash->cast( flash_pos );
						return;
					}
				}
			}
		}
		if (sender->is_me())
		{
			//console->print("%s", spell->get_spell_data()->get_name_cstr());
			if (spell_hash == spell_hash("QuicksilverSash") || spell_hash == spell_hash("6035_Spell") || spell_hash == spell_hash("ItemMercurial"))
			{
				cast_qss_t = gametime->get_time();
			}
			if (spell_hash == spell_hash("Summonersnowball"))
			{
				snow_loop = gametime->get_time() + 4.0f;
			}
		}
	}
	void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
	{
		if ( !sender || !sender->is_valid( ) || !sender->is_ai_hero( ) || !sender->is_enemy( ) )
			return;

		/*if (use_flash::use_flash->get_bool() && use_flash::anti_gapclose->get_bool() && flash && flash->is_ready() &&
			A_::checkbox(use_flash::use_to, sender) && sender->is_valid_target( static_cast< float >( use_flash::anti_range->get_int() )) &&
			myhero->get_health_percent() <= use_flash::anti_hp->get_int())
		{
			summoner_spell::flash_gapcloser(flash, sender);
		}*/

		auto health_pcent = health_prediction->get_health_prediction( myhero, 0.65f, 0.f ) / myhero->get_max_health( ) * 100.f;
		if ( exhaust && exhaust->is_ready( ) && exhaust_spell::exhaust_antigap->get_bool( ) && args->end_position.distance( myhero ) < args->start_position.distance( myhero ) 
			&& sender->is_valid_target( 650.f ) )
		{
			auto it = exhaust_spell::m_mode_hp.find( sender->get_champion( ) );
			if ( it != exhaust_spell::m_mode_hp.end( ) && 
				( it->second.first->get_int( ) == 1 || ( it->second.first->get_int( ) == 2 && static_cast<float>( it->second.second->get_int( ) ) <= health_pcent ) ) )
			{
				exhaust->cast( sender );
			}
		}
	}

	void all_menu()
	{
		summoner_spell_menu();
		smite_menu();
		exhaust_menu();
		ghost_menu();
		ignite_menu();
		heal_menu();
		barrier_menu();
		flash_menu();
		snowball_menu();

		spell_check();

#pragma region summoner_menu_callbacks

#pragma region summoner_showall_callback		
		const auto on_show_all_click = [ ]( TreeEntry* self ) {
			auto main_tab = menu->get_tab( "utility" ),
				summoner = main_tab->get_tab( "summoner" ),
				show_smite = summoner->get_tab( "smite_menu" ),
				show_exhaust = summoner->get_tab( "exhaust_menu" ),
				show_ghost = summoner->get_tab( "ghost_menu" ),
				show_heal = summoner->get_tab( "heal_menu" ),
				show_barrier = summoner->get_tab( "barrier_menu" ),
				show_ignite = summoner->get_tab( "ignite_menu" ),
				show_flash = summoner->get_tab( "flash_menu" ),
				show_snowball = summoner->get_tab( "snowball_menu" );


			if ( !all_spell::only_show->get_bool( ) )
			{
				show_smite->is_hidden( ) = smite == nullptr;
				show_exhaust->is_hidden( ) = exhaust == nullptr;
				show_ghost->is_hidden( ) = ghost == nullptr;
				show_heal->is_hidden( ) = heal == nullptr;
				show_barrier->is_hidden( ) = barrier == nullptr;
				show_ignite->is_hidden( ) = ignite == nullptr;
				show_flash->is_hidden( ) = flash == nullptr;
				show_snowball->is_hidden( ) = snowball == nullptr;
			}
			else
			{
				show_smite->is_hidden( ) =
					show_exhaust->is_hidden( ) =
					show_ghost->is_hidden( ) =
					show_heal->is_hidden( ) =
					show_barrier->is_hidden( ) =
					show_ignite->is_hidden( ) =
					show_flash->is_hidden( ) =
					show_snowball->is_hidden( ) = false;
			}
		};

		all_spell::only_show->add_property_change_callback( on_show_all_click );
		on_show_all_click( all_spell::only_show );
#pragma endregion summoner_showall_callback	

#pragma region summoner_cleanse_callbacks
		const auto cleanse_danger_level_1_always_click = [ ]( TreeEntry* self ) {
			cleanse_spell::cleanse_danger_level_1_hp->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_1_enemy_range->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_1_enemy_count->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_1_delay->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_1_duration->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_1_combo->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_1_harass->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_1_other->is_hidden( ) = self->get_bool( );
		};
		cleanse_spell::cleanse_danger_level_1_alway_use->add_property_change_callback( cleanse_danger_level_1_always_click );
		cleanse_danger_level_1_always_click( cleanse_spell::cleanse_danger_level_1_alway_use );

		const auto cleanse_danger_level_2_always_click = [ ]( TreeEntry* self ) {

			cleanse_spell::cleanse_danger_level_2_hp->is_hidden( ) =
				cleanse_spell::cleanse_danger_level_2_enemy_range->is_hidden( ) =
				cleanse_spell::cleanse_danger_level_2_enemy_count->is_hidden( ) =
				cleanse_spell::cleanse_danger_level_2_delay->is_hidden( ) =
				cleanse_spell::cleanse_danger_level_2_duration->is_hidden( ) =
				cleanse_spell::cleanse_danger_level_2_combo->is_hidden( ) =
				cleanse_spell::cleanse_danger_level_2_harass->is_hidden( ) =
				cleanse_spell::cleanse_danger_level_2_other->is_hidden( ) = self->get_bool( );
		};
		cleanse_spell::cleanse_danger_level_2_alway_use->add_property_change_callback( cleanse_danger_level_2_always_click );
		cleanse_danger_level_2_always_click( cleanse_spell::cleanse_danger_level_2_alway_use );

		const auto cleanse_danger_level_3_always_click = [ ]( TreeEntry* self ) {
			cleanse_spell::cleanse_danger_level_3_hp->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_3_enemy_range->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_3_enemy_count->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_3_delay->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_3_duration->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_3_combo->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_3_harass->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_3_other->is_hidden( ) = self->get_bool( );
		};
		cleanse_spell::cleanse_danger_level_3_alway_use->add_property_change_callback( cleanse_danger_level_3_always_click );
		cleanse_danger_level_3_always_click( cleanse_spell::cleanse_danger_level_3_alway_use );

		const auto cleanse_danger_level_4_always_click = [ ]( TreeEntry* self ) {
			cleanse_spell::cleanse_danger_level_4_hp->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_4_enemy_range->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_4_enemy_count->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_4_delay->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_4_duration->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_4_combo->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_4_harass->is_hidden( ) = 
				cleanse_spell::cleanse_danger_level_4_other->is_hidden( ) = self->get_bool( );
		};
		cleanse_spell::cleanse_danger_level_4_alway_use->add_property_change_callback( cleanse_danger_level_4_always_click );
		cleanse_danger_level_4_always_click( cleanse_spell::cleanse_danger_level_4_alway_use );
#pragma endregion summoner_cleanse_callbacks

#pragma region summoner_smite_callbacks
		const auto on_smite_mode_click = [ ]( TreeEntry* mode )
		{
			smite_spell::use_smite_black_list->is_hidden( ) =
				smite_spell::smite_key->is_hidden( ) =
				smite_spell::hotkey_use_smite_key->is_hidden( ) =
				smite_spell::selector_position->is_hidden( ) = ( mode->get_int( ) == 1 );

			smite_spell::range->is_hidden( ) = smite_spell::range_style->is_hidden( ) = 
				smite_spell::thickness->is_hidden( ) =
				smite_spell::damage->is_hidden( ) =
				smite_spell::glow_ind_s->is_hidden( ) =
				smite_spell::glow_ind_p->is_hidden( ) =
				smite_spell::glow_out_s->is_hidden( ) =
				smite_spell::smite_2->is_hidden( ) =
				smite_spell::glow_out_p->is_hidden( ) = ( mode->get_int( ) == 1 );

			smite_spell::smite_cart_tab->is_hidden( ) =
				smite_spell::smite_kill_key->is_hidden( ) =
				smite_spell::smite_siege_minion->is_hidden( ) = ( mode->get_int( ) == 0 );
		};
		smite_spell::mode->add_property_change_callback( on_smite_mode_click );
		on_smite_mode_click( smite_spell::mode );
#pragma endregion summoner_smite_callbacks

#pragma endregion summoner_menu_callbacks
	}
	void all_logic()
	{
		static auto init = false;
		if (!init)
		{
			event_handler< events::on_process_spell_cast >::add_callback(OnProcessSpellCast);
			//antigapcloser::add_event_handler(on_gapcloser);
			init = true;
		}

		static auto t_last_update = -1.f;
		if (gametime->get_time() - t_last_update >= 4.f)
		{
			t_last_update = gametime->get_time();
			for (int i = neutral_camp_id::Blue_Order; i < neutral_camp_id::Max_Camps; i++)
			{
				auto camp_monsters_nids = camp_manager->get_camp_minions(i);
				if (camp_monsters_nids.empty())
					continue;

				std::vector< game_object_script > camp_monsters;
				for (auto&& nid : camp_monsters_nids)
				{
					auto monster = entitylist->get_object_by_network_id(nid);
					if (!monster || !monster->is_valid( ))
						continue;

					camp_monsters.push_back(monster);
				}

				if (camp_monsters.empty())
					continue;

				std::sort(camp_monsters.begin(), camp_monsters.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_bounding_radius() > b->get_bounding_radius();
					});

				auto&& monster = camp_monsters.front();
				auto name = monster->get_name();
				std::transform(name.begin(), name.end(), name.begin(),
					[](unsigned char c) { return std::tolower(c); });
				if (A_::is_epic_monster(name))
				{
					smite_spell::smite_epic->set_texture(monster->get_square_icon_portrait());
					smite_spell::hotkey_use_epic->set_texture(monster->get_square_icon_portrait());
				}
				else if (name.find("sru_blue") != std::string::npos)
				{
					smite_spell::red_buff->set_texture(monster->get_square_icon_portrait());
					smite_spell::hotkey_use_red_buff->set_texture(monster->get_square_icon_portrait());
				}
				else
				{
					switch (i)
					{
					case neutral_camp_id::Crab_Bottom:
					case neutral_camp_id::Crab_Top:
						smite_spell::small_crab->set_texture(monster->get_square_icon_portrait());
						smite_spell::hotkey_use_small_crab->set_texture(monster->get_square_icon_portrait());
						break;
					case neutral_camp_id::Wolves_Chaos:
					case neutral_camp_id::Wolves_Order:
						smite_spell::three_wolf->set_texture(monster->get_square_icon_portrait());
						smite_spell::hotkey_use_three_wolf->set_texture(monster->get_square_icon_portrait());
						break;
					case neutral_camp_id::Krugs_Chaos:
					case neutral_camp_id::Krugs_Order:
						smite_spell::stone->set_texture(monster->get_square_icon_portrait());
						smite_spell::hotkey_use_stone->set_texture(monster->get_square_icon_portrait());
						break;
					case neutral_camp_id::Raptors_Chaos:
					case neutral_camp_id::Raptors_Order:
						smite_spell::six_bird->set_texture(monster->get_square_icon_portrait());
						smite_spell::hotkey_use_six_bird->set_texture(monster->get_square_icon_portrait());
						break;
					case neutral_camp_id::Gromp_Chaos:
					case neutral_camp_id::Gromp_Order:
						smite_spell::gromp->set_texture(monster->get_square_icon_portrait());
						smite_spell::hotkey_use_gromp->set_texture(monster->get_square_icon_portrait());
						break;
					default:
						break;
					}
				}
			}

			auto minions = entitylist->get_enemy_minions();
			minions.erase(std::remove_if(minions.begin(), minions.end(), [](game_object_script x)
				{
					return !x || !x->is_valid( ) || x->get_bounding_radius() != 65.f;
				}), minions.end());
			if (!minions.empty())
			{
				smite_spell::smite_siege_minion->set_texture(minions.front()->get_square_icon_portrait());
				smite_spell::cannon->set_texture(minions.front()->get_square_icon_portrait());
				smite_spell::hotkey_use_cannon->set_texture(minions.front()->get_square_icon_portrait());
			}
		}

		smite_monster();
		if (myhero->is_dead()) return;

		key_exhaust_logic();
		exhaust_logic( );
		semi_ignite();
		semi_smite();
		semi_snowball();

		if (all_spell::use->get_bool()) return;
		ghost_logic();
		ignite_logic();
		cc_logic();
		heal_logic();
		barrier_logic();
		snowball_logic();
	}
	void all_draw()
	{
		if (myhero->is_dead()) return;
		draw_smite_state();
	}
	void all_new_draw()
	{
		if (myhero->is_dead()) return;
		draw_smite();
	}
	void all_unload()
	{
		event_handler<events::on_process_spell_cast>::remove_handler(OnProcessSpellCast);
		//antigapcloser::remove_event_handler(on_gapcloser);

		if (q)
		{
			q->set_spell_lock(false);
			plugin_sdk->remove_spell(q);
		}

		if (w)
		{
			w->set_spell_lock(false);
			plugin_sdk->remove_spell(w);
		}

		if (e)
		{
			e->set_spell_lock(false);
			plugin_sdk->remove_spell(e);
		}

		if (r)
		{
			r->set_spell_lock(false);
			plugin_sdk->remove_spell(r);
		}

		delete exhaust_spell::b_enabled;
	}
}