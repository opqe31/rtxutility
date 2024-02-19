#include "../plugin_sdk/plugin_sdk.hpp"
#include "exhaust_spelldb.h"


namespace exhaust_spelldb
{
	std::map<champion_id, std::vector<spell_data>> load()
	{
		std::map<champion_id, std::vector<spell_data>> data =
		{
			{
				champion_id::Tristana,
				{
					spell_data( "E", 50, true, buff_hash( "tristanaecharge" ) ),
				}
			},
			{
				champion_id::Akshan,
				{
					spell_data ("R", 50, true, std::nullopt, buff_hash( "AkshanR" ) ),
				}
			},
			{
				champion_id::Camille,
				{
					spell_data( "R", 50, true, buff_hash( "camillerrange" ) ),
				}
			},
			{
				champion_id::DrMundo,
				{
					spell_data("R", 50, true, std::nullopt, buff_hash( "DrMundoR" ) ),
				}
			},
			{
				champion_id::FiddleSticks,
				{
					spell_data( "R", 100, true, std::nullopt, buff_hash( "FiddleSticksR" ) ),
				}
			},
			{
				champion_id::Hecarim,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "hecarimultsound" ) ),
				}
			},
			{
				champion_id::Illaoi,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "IllaoiR" ) ),
				}
			},
			{
				champion_id::Jax,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "JaxRPassive" ) ),
				}
			},
			{
				champion_id::Katarina,
				{
					spell_data( "R", 100, true, std::nullopt, buff_hash( "katarinarsound" ) ),
				}
			},
			{
				champion_id::Kennen,
				{
					spell_data("R", 70, true, std::nullopt, buff_hash( "KennenShurikenStorm" ) ),
				}
			},
			{
				champion_id::Lucian,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "LucianR" ) ),
				}
			},
			{
				champion_id::MasterYi,
				{
					spell_data("R", 70, true, std::nullopt, buff_hash( "Highlander" ) ),
				}
			},
			{
				champion_id::MissFortune,
				{
					spell_data("R", 70, true, std::nullopt, buff_hash( "missfortunebulletsound" ) ),
				}
			},
			{
				champion_id::MonkeyKing,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "MonkeyKingSpinToWin" ) )
				}
			},
			{
				champion_id::Nasus,
				{
					spell_data( "R", 40, true, std::nullopt, buff_hash( "NasusR" ) ),
				}
			},
			{
				champion_id::Neeko,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "NeekoR" ) ),
					spell_data( "R2", 50, true, std::nullopt, buff_hash( "neekor2" ) ),
				}
			},
			{
				champion_id::Nilah,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "NilahR" ) ),
				}
			},
			{
				champion_id::Nunu,
				{
					spell_data( "R", 50, true, std::nullopt, std::nullopt, spell_hash( "NunuR" ) ),
				}
			},
			{
				champion_id::Olaf,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "OlafRagnarok" ) ),
				}
			},
			{
				champion_id::Riven,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "RivenFengShuiEngine" ) ),
				}
			},
			{
				champion_id::Samira,
				{
					spell_data( "R", 100, true, std::nullopt, buff_hash( "SamiraR" ) ),
				}
			},
			{
				champion_id::Rengar,
				{
					spell_data( "Q", 30, true, std::nullopt, buff_hash( "RengarQ" ) ),
					spell_data( "Q empowered", 60, true, std::nullopt, buff_hash( "RengarQEmp" ) ),
					spell_data( "R debuff", 90, true, buff_hash( "rengarrshred" ) ),
				}
			},
			{
				champion_id::Swain,
				{
					spell_data( "R", 30, true, std::nullopt, buff_hash( "SwainR" ) ),
				}
			},
			{
				champion_id::Warwick,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "warwickrsound" ) ),
				}
			},
			{
				champion_id::Zed,
				{
					spell_data( "R", 100, true, buff_hash( "zedrdeathmark" ) ),
				}
			},
			{
				champion_id::Zeri,
				{
					spell_data( "R", 100, true, std::nullopt, buff_hash( "ZeriR" ) ),
				}
			},
			{
				champion_id::Aatrox,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "AatroxR" ) ),
				}
			},
			{
				champion_id::Volibear,
				{
					spell_data( "R", 30, true, std::nullopt, buff_hash( "VolibearR" ) ),
				}
			},
			{
				champion_id::Tryndamere,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "UndyingRage" ) ) ,
				}
			},
			{
				champion_id::Fiora,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "fiorarbuff" ) ),
				}
			},
			{
				champion_id::Renekton,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "RenektonReignOfTheTyrant" ) ),
				}
			},
			{
				champion_id::Shyvana,
				{
					spell_data( "R cast", 50, true, std::nullopt, std::nullopt, spell_hash( "ShyvanaTransformCast" ) ),
					spell_data( "R leap", 50, true, std::nullopt, std::nullopt, spell_hash( "ShyvanaTransformLeap" ) ),
				}
			},
			{
				champion_id::Trundle,
				{
					spell_data("R", 50, true, std::nullopt, buff_hash( "trundlepainbuff" ) ),
				}
			},
			{
				champion_id::Diana,
				{
					spell_data( "R", 50, true, std::nullopt, std::nullopt, spell_hash( "DianaR" ) ),
				}
			},
			{
				champion_id::Vayne,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "VayneInquisition" ) ),
				}
			},
			{
				champion_id::Velkoz,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "VelkozR" ) ),
				}
			},
			{
				champion_id::KSante,
				{
					spell_data("R", 50, true, std::nullopt, buff_hash( "KSanteRTransform" ) ),
				}
			},
			{
				champion_id::Mordekaiser,
				{
					spell_data( "R", 50, true, std::nullopt, buff_hash( "mordekaiserr_statsteal" ) ),
				}
			},
		};
		return data;
	}
}