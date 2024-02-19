#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"
#include <optional>

namespace exhaust_spelldb
{
	struct spell_data
	{
		const char* display_name;
		int hp;
		bool menu;

		std::optional<uint32_t> buff_hash_ally = std::nullopt;
		std::optional<uint32_t> buff_hash_enemy = std::nullopt;
		std::optional<uint32_t> spell_active_hash = std::nullopt;

		auto is_active( game_object_script enemy, int health, bool allies = true ) -> bool
		{
			if ( !enemy || !enemy->is_valid( ) )
				return false;

			auto low_hp_ally = false;
			if ( allies && ( buff_hash_enemy.has_value( ) || spell_active_hash.has_value( ) ) )
			{
				auto allies = entitylist->get_ally_heroes( );
				low_hp_ally = std::find_if( allies.begin( ), allies.end( ), [ & ]( game_object_script x ) {
					return x && x->is_valid( ) && !x->is_dead( ) && !x->is_invulnerable( ) && !x->is_me( ) && x->get_distance( enemy ) <= 650.f;
				} ) != allies.end( );
			}

			if ( buff_hash_enemy.has_value( ) && enemy->has_buff( buff_hash_enemy.value( ) ) && 
				( low_hp_ally || health >= 100 || myhero->get_health_percent( ) <= static_cast< float >( health ) ) )
				return true;

			if ( spell_active_hash.has_value( ) && ( low_hp_ally || health >= 100 || myhero->get_health_percent( ) <= static_cast< float >( health ) ) )
			{
				auto active_spell = enemy->get_active_spell( );
				if ( active_spell )
				{
					auto sdata = active_spell->get_spell_data( );
					if ( sdata && sdata->get_name_hash( ) == spell_active_hash.value( ) )
						return true;
				}
			}

			if ( buff_hash_ally.has_value( ) )
			{
				for ( auto&& x : entitylist->get_ally_heroes( ) )
				{
					if ( !x || !x->is_valid( ) || x->is_dead( ) )
						continue;

					if ( !x->is_me( ) && !allies )
						continue;

					if ( ( health >= 100 || x->get_health_percent( ) <= static_cast<float>( health ) ) && 
						x->has_buff( buff_hash_ally.value( ) ) )
						return true;
				}
			}

			return false;
		}

		spell_data( const char* menu_name, int activate_hp, bool enabled_by_default = false, 

			std::optional<uint32_t> opt_buff_hash_ally = std::nullopt, 
			std::optional<uint32_t> opt_buff_hash_self = std::nullopt,
			std::optional<uint32_t> opt_spell_hash_self = std::nullopt )
		{
			display_name = menu_name;
			hp = activate_hp;
			menu = enabled_by_default;

			buff_hash_ally = opt_buff_hash_ally;
			buff_hash_enemy = opt_buff_hash_self;
			spell_active_hash = opt_spell_hash_self;
		}
	};

	std::map<champion_id, std::vector<spell_data>> load();
};