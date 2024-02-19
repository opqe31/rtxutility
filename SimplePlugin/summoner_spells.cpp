#include "cooldown_manager.hpp"

auto c_cooldown_tracker::get_real_cooldown_sum( game_object_script sender, float base_cooldown ) -> float
{
	auto summoner_haste = 0.f;

	//boots
	if ( sender->has_item( { ItemId::Ionian_Boots_of_Lucidity, ItemId::Ionian_Boots_of_Lucidity_Arena } ) != spellslot::invalid )
		summoner_haste += 12.f;

	//aram -> better to check buff
	static auto is_aram = missioninfo->get_map_id( ) == game_map_id::HowlingAbyss;
	if ( is_aram )
		summoner_haste += 70.f;

	//cosmic insight
	if ( sender->has_perk( 8347 ) ) //CosmicInsight
		summoner_haste += 18.f;

	//item_haste perk

	auto reduced_cooldown = base_cooldown * ( 100.f / ( 100.f + summoner_haste ) );
	return std::clamp( reduced_cooldown, 0.f, base_cooldown * 1.5f );
}

auto c_cooldown_tracker::simplify_sum_hash( uint32_t in ) ->uint32_t
{
	switch ( in )
	{
		default:
			break;

		case spell_hash( "SummonerSmite" ):
		case spell_hash( "S5_SummonerSmitePlayerGanker" ): 
		case spell_hash( "SummonerSmiteAvatarOffensive" ): 
		case spell_hash( "SummonerSmiteAvatarUtility" ):
		case spell_hash( "SummonerSmiteAvatarDefensive" ):
			return spell_hash( "SummonerSmite" );

		case spell_hash( "S12_SummonerTeleportUpgrade" ):
			return spell_hash( "SummonerTeleport" );

		case spell_hash( "SnowballFollowupCast" ):
			return spell_hash( "SummonerSnowball" );
	}
	return in;
}

auto c_cooldown_tracker::sum_cd( game_object_script sender, uint32_t hash_raw ) -> std::optional<float>
{
	if ( !sender->is_enemy( ) && ( !e_override_ally || !e_override_ally->get_bool( ) ) )
		return std::nullopt;

	auto cd_manager_custom = e_manager_sums ? e_manager_sums->get_bool( ) : false;
	if ( !cd_manager_custom )
		return std::nullopt;

	auto it = m_cooldown_sums.find( sender->get_network_id( ) );
	if ( it == m_cooldown_sums.end( ) )
	{
		if ( !e_ignore_core_trackers || !e_ignore_core_trackers->get_bool( ) )
			return std::nullopt;
		else
			return 0.f;
	}

	auto it_cd = it->second.find( simplify_sum_hash( hash_raw ) );
	if ( it_cd == it->second.end( ) )
	{
		if ( !e_ignore_core_trackers || !e_ignore_core_trackers->get_bool( ) )
			return std::nullopt;
		else
			return 0.f;
	}

	if ( it_cd->second.recast_end.has_value( ) && gametime->get_time( ) <= it_cd->second.recast_end.value( ) )
		return std::max( 0.f, it_cd->second.recast_end.value( ) - gametime->get_time( ) );

	if ( !it_cd->second.cooldown_end.has_value( ) )
		return std::nullopt;

	return std::max( it_cd->second.cooldown_end.value( ) - gametime->get_time( ), 0.f );
}

auto c_cooldown_tracker::sum_cd_start( game_object_script sender, uint32_t hash_raw ) -> std::optional<float>
{
	if ( !sender->is_enemy( ) && ( !e_override_ally || !e_override_ally->get_bool( ) ) )
		return std::nullopt;

	if ( !e_manager_sums || !e_manager_sums->get_bool( ) )
		return std::nullopt;

	auto not_found = [ & ]( ) -> std::optional<float> {
		if ( !e_ignore_core_trackers || !e_ignore_core_trackers->get_bool( ) )
			return std::nullopt;
		else
			return 0.f;
	};

	auto it = m_cooldown_sums.find( sender->get_network_id( ) );
	if ( it == m_cooldown_sums.end( ) )
		return not_found( );

	auto it_cd = it->second.find( simplify_sum_hash( hash_raw ) );
	if ( it_cd == it->second.end( ) )
		return not_found( );

	return it_cd->second.cooldown_start;
}

const std::map< c_cooldown_tracker::e_summoner_type, uint32_t > m_enum_hashes = 
{
	{ c_cooldown_tracker::e_summoner_type::flash, spell_hash( "SummonerFlash" ) },
	{ c_cooldown_tracker::e_summoner_type::ignite, spell_hash( "SummonerDot" ) },
	{ c_cooldown_tracker::e_summoner_type::exhaust, spell_hash( "SummonerExhaust" ) },
	{ c_cooldown_tracker::e_summoner_type::cleanse, spell_hash( "SummonerBoost" ) },
	{ c_cooldown_tracker::e_summoner_type::ghost, spell_hash( "SummonerHaste" ) },
	{ c_cooldown_tracker::e_summoner_type::heal, spell_hash( "SummonerHeal" ) },
	{ c_cooldown_tracker::e_summoner_type::barrier, spell_hash( "SummonerBarrier" ) },
	{ c_cooldown_tracker::e_summoner_type::smite, spell_hash( "SummonerSmite" ) },
	{ c_cooldown_tracker::e_summoner_type::teleport, spell_hash( "SummonerTeleport" ) },
	{ c_cooldown_tracker::e_summoner_type::snowball, spell_hash( "SummonerSnowball" ) },
	{ c_cooldown_tracker::e_summoner_type::clarity, spell_hash( "SummonerMana" ) }
};

auto c_cooldown_tracker::update_summoner_cooldown( game_object_script sender, e_summoner_type sum_type, float extra_t, bool force, std::optional<float> opt_recast_time ) -> void
{
	auto it_hash = m_enum_hashes.find( sum_type );
	if ( it_hash == m_enum_hashes.end( ) )
	{
#if defined( __TEST )
		myhero->print_chat( 0x3, "c_cooldown_tracker::update_summoner_cooldown -> hash (%d)", static_cast< int >( sum_type ) );
#endif
		return;
	}

	auto hash = it_hash->second;

	auto spell = database->get_spell_by_hash( hash );
	if ( !spell )
	{
#if defined( __TEST )
		myhero->print_chat( 0x3, "c_cooldown_tracker::update_summoner_cooldown -> spell (%d)", static_cast< int >( sum_type ) );
#endif
		return;
	}

	auto it = this->m_cooldown_sums.find( sender->get_network_id( ) );
	if ( it == m_cooldown_sums.end( ) )
	{
		m_cooldown_sums [ sender->get_network_id( ) ] = std::map<uint32_t, s_spell_time> { };
		it = this->m_cooldown_sums.find( sender->get_network_id( ) );
	}

	if ( it == m_cooldown_sums.end( ) )
		return;

	auto cooldown = *spell->CooldownTime( );
	if ( sum_type != e_summoner_type::smite )
		cooldown = get_real_cooldown_sum( sender, cooldown );

	auto it_cd = it->second.find( hash );
	if ( it_cd == it->second.end( ) )
	{
		if ( !opt_recast_time.has_value( ) )
			it->second [ hash ] = s_spell_time { gametime->get_time( ) + cooldown + extra_t, std::nullopt };
		else
			it->second [ hash ] = s_spell_time { std::nullopt, opt_recast_time.value( ) + extra_t };

		it->second [ hash ].cooldown_start = gametime->get_time( );
	}
	else
	{
		if ( !opt_recast_time.has_value( ) )
		{
			if ( !it_cd->second.cooldown_end.has_value( ) || it_cd->second.cooldown_end.value( ) < gametime->get_time( ) || force )
				it_cd->second.cooldown_end = gametime->get_time( ) + cooldown + extra_t;

			if ( force )
				it_cd->second.recast_end = std::nullopt;

			if ( !it_cd->second.cooldown_end.has_value( ) || std::abs( ( it_cd->second.cooldown_end.value( ) - gametime->get_time( ) ) - ( cooldown + extra_t ) ) > TOL_COOLDOWN_START )
				it_cd->second.cooldown_start = gametime->get_time( );
		}
		else
		{
			it_cd->second.recast_end = gametime->get_time( ) + opt_recast_time.value( ) + extra_t;
		}
	}
#ifdef __TEST
	if ( !opt_recast_time.has_value( ) )
		myhero->print_chat( 0x3, "c_cooldown_tracker::update_summoner_cooldown( %s, %d ) -> %.1f ( %.1f | %.1f ) | extra_t: %.1f", sender->get_model_cstr( ), static_cast< int >( sum_type ), cooldown, *spell->CooldownTime( ), *spell->mAmmoRechargeTime( ), extra_t );
	else
		myhero->print_chat( 0x3, "c_cooldown_tracker::update_summoner_recast( %s, %d ) -> %.1f", sender->get_model_cstr( ), static_cast< int >( sum_type ), opt_recast_time.value( ) );
#endif
}

//
//	Init
//
auto c_cooldown_tracker::init_sums( ) -> void
{
	auto fill_sum_map = [ & ]( game_object_script x ) -> void
	{
		auto&& it = m_cooldown_sums [ x->get_network_id( ) ];
		for ( auto i = spellslot::summoner1; i <= spellslot::summoner2; i = static_cast< spellslot >( static_cast< int >( i ) + 1 ) )
		{
			auto spell = x->get_spell( i );
			if ( !spell )
				continue;

			it [ spell->get_name_hash( ) ] = s_spell_time { gametime->get_time( ) + spell->cooldown( ) - 0.05f, std::nullopt };
		}
	};

	for ( auto&& x : entitylist->get_all_heroes( ) )
	{
		if ( !x || !x->is_valid( ) )
			continue;

		m_cooldown_sums [ x->get_network_id( ) ] = std::map<uint32_t, s_spell_time> { };
		fill_sum_map( x );
	}
}

//
//	Callbacks
//
auto c_cooldown_tracker::on_create_missile_sums( game_object_script sender ) -> void
{
	auto missile_owner = entitylist->get_object( sender->missile_get_sender_id( ) );
	if ( !missile_owner || !missile_owner->is_valid( ) || !missile_owner->is_ai_hero( ) )
		return;

	if ( sender->missile_get_sender_id( ) == myhero->get_id( ) )
	{
		switch ( buff_hash_real( sender->get_name_cstr( ) ) )
		{
			default:
				break;

			case buff_hash( "PoroSnowballMissile" ):
				update_summoner_cooldown( missile_owner, e_summoner_type::snowball, 0.f, false );
				break;
		}
	}
}

auto c_cooldown_tracker::on_create_effect_sums( game_object_script effect, game_object_script emitter, const effect_create_data_client& create_data, uint32_t hash ) -> void
{
	switch ( buff_hash_real( effect->get_name_cstr( ) ) )
	{
		default:
			break;

		case buff_hash( "global_ss_flash.troy" ):
		case buff_hash( "global_ss_flash_02.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::flash, 0.f );
			break;

		case buff_hash( "Global_SS_Cleanse_Avatar.troy" ):
		case buff_hash( "Global_SS_Cleanse.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::cleanse, 0.f );
			break;

		case buff_hash( "Global_SS_Ghost.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::ghost, 0.f, false );
			break;

		case buff_hash( "global_ss_heal.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::heal, 0.f );
			break;

		case buff_hash( "Global_SS_Barrier.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::barrier, 0.f );
			break;

		case buff_hash( "global_ss_teleport_turret_blue.troy" ):
		case buff_hash( "global_ss_teleport_turret_red.troy" ):
		case buff_hash( "global_ss_teleport_target_blue.troy" ):
		case buff_hash( "global_ss_teleport_target_red.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::teleport, 0.f );
			break;

		case buff_hash( "global_ss_clarity.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::clarity, 0.f );
			break;

		case buff_hash( "Summoner_Snowball_Arrive_Sound.troy" ):
			update_summoner_cooldown( emitter, e_summoner_type::snowball, 0.f, false );
			break;
	}

	switch ( hash )
	{
		default:
			break;

		case 17810116: //Global_SS_Ignite
			update_summoner_cooldown( emitter, e_summoner_type::ignite, 0.f );
			break;
		case 2099993144: //Global_SS_Ignite_Tick
			update_summoner_cooldown( emitter, e_summoner_type::ignite, 0.f, false );
			break;

		case 2435266028: //Global_SS_Exhaust
			update_summoner_cooldown( emitter, e_summoner_type::exhaust, 0.f );
			break;

		case 3751416071: //Perks_HextechFlashtraption_Channel
			break;

		case 1782626493: //Perks_HextechFlashtraption_Cas
			break;

		case 3150565910: //SRU_Jungle_Companions_Skin01_BA_SmiteAttributionBeam, SRU_Jungle_Companions_Base_BA_SmiteAttributionBeam, SRU_Jungle_Companions_Skin02_BA_SmiteAttributionBeam
		case 2661345555: //SS_BaseSmite
		case 4266712792: //SRU_Jungle_Companions_Skin02_Passive_Smite_Lizard, SRU_Jungle_Companions_Skin01_Passive_Smite, ..
		case 1893163736: //SRU_Jungle_Companions_Base_SS_LizardSmite, SRU_Jungle_Companions_Skin01_SS_CatSmite, ..
			update_summoner_cooldown( emitter, e_summoner_type::smite, 0.f );
			break;

		case 1554230781: //Summoner_Snowball_Tagged
			update_summoner_cooldown( emitter, e_summoner_type::snowball, 0.f, true, 3.f );
			break;
		case 1571910067: //Summoner_Snowball_Dash_Arrival
		{
			auto it_hash = m_enum_hashes.find( e_summoner_type::snowball );
			if ( it_hash != m_enum_hashes.end( ) )
			{
				auto spell = database->get_spell_by_hash( it_hash->second );
				if ( spell )
				{
					auto cooldown = this->get_real_cooldown_spell( emitter, *spell->CooldownTime( ) );
					update_summoner_cooldown( emitter, e_summoner_type::snowball, -cooldown*0.15f, true );
				}
			}
			break;
		}
	}
}

auto c_cooldown_tracker::on_buff_gain_sums( game_object_script emitter, buff_instance_script buff, float time_elapsed ) -> void
{
	switch ( buff->get_hash_name( ) )
	{
		default:
			break;

		case buff_hash( "SummonerDot" ):
			update_summoner_cooldown( emitter, e_summoner_type::ignite, -time_elapsed, false );
			break;

		case buff_hash( "SummonerBoost" ):
			update_summoner_cooldown( emitter, e_summoner_type::cleanse, -time_elapsed, false );
			break;

		case buff_hash( "summonerexhaustslow" ):
		case buff_hash( "SummonerExhaust" ):
		case buff_hash( "summonerexhaustdebuff" ):
			update_summoner_cooldown( emitter, e_summoner_type::exhaust, -time_elapsed, false );
			break;

		case buff_hash( "SummonerHaste" ):
			update_summoner_cooldown( emitter, e_summoner_type::ghost, -time_elapsed, false );
			break;
	}
}

auto c_cooldown_tracker::on_buff_lose_sums( game_object_script emitter, buff_instance_script buff, float time_elapsed ) -> void
{
	switch ( buff->get_hash_name( ) )
	{
		default:
			break;

		case buff_hash( "SummonerDot" ):
			update_summoner_cooldown( emitter, e_summoner_type::ignite, -time_elapsed, false );
			break;

		case buff_hash( "SummonerBoost" ):
			update_summoner_cooldown( emitter, e_summoner_type::cleanse, -time_elapsed, false );

		case buff_hash( "summonerexhaustslow" ):
		case buff_hash( "SummonerExhaust" ):
		case buff_hash( "summonerexhaustdebuff" ):
			update_summoner_cooldown( emitter, e_summoner_type::exhaust, -time_elapsed, false );
			break;

		case buff_hash( "SummonerHaste" ):
			update_summoner_cooldown( emitter, e_summoner_type::ghost, -time_elapsed, false );
			break;
	}
}