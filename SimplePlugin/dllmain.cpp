#include "utility.h"
#include "fake_cursor.h"
#include "notify.h"
#include "side.hpp"
#include "map_data.hpp"

PLUGIN_NAME( "[] Utils" );

#if defined( _VBETA ) || defined ( __TEST )
PLUGIN_TYPE( plugin_type::misc );
#else
PLUGIN_TYPE( plugin_type::utility );
#endif

/*
    [POST BUILD EVENTS]
    copy to both paths

    $(SolutionDir)$(Platform)\$(Configuration)\
    $(TargetFileName)

    Lune
    IF EXIST "C:\Users\PC\AppData\Roaming\94136856dcf5071e3f98755e70787c92\plugins\x64" ( cmd /C xcopy /y "$(SolutionDir)$(Platform)\$(Configuration)\$(TargetFileName)" "C:\Users\PC\AppData\Roaming\94136856dcf5071e3f98755e70787c92\plugins\x64" && echo Copied for Lune ) ELSE (echo [!]Copy failed for Lune)

    
    IF EXIST "C:\Users\user\AppData\Roaming\6e4bbc915bf60b7128238d790b3a248f\plugins\x64" ( cmd /C xcopy /y "$(SolutionDir)$(Platform)\$(Configuration)\$(TargetFileName)" "C:\Users\user\AppData\Roaming\6e4bbc915bf60b7128238d790b3a248f\plugins\x64" && echo Copied for  ) ELSE (echo [!]Copy failed for )
*/

PLUGIN_API bool on_sdk_load( plugin_sdk_core* plugin_sdk_good )
{
    DECLARE_GLOBALS( plugin_sdk_good )

    g_map_data->load( );
    utility::load( );
    g_side_hud->load( );
    g_notify->load( );

#ifdef __TEST
    for ( auto&& spell : { myhero->get_spell( spellslot::summoner1 ), myhero->get_spell( spellslot::summoner2 ) } )
    {
        if ( spell )
            myhero->print_chat( 0x3, "summoner: %s", spell->get_name( ).c_str( ) );
    }

    myhero->print_chat( 0x3, "base_skin_name: %s", myhero->get_base_skin_name( ).c_str( ) );
     /*auto play_sound = sound->load_sound( L"//failure.wav" );
     if ( play_sound != nullptr )
     {
         sound->clear_queue( );
         sound->play_sound( play_sound, 1.f );
     }
     else
         console->print_error( "sound not found %.1f", gametime->get_time( ) );*/
#endif

    return true;
}

PLUGIN_API void on_sdk_unload()
{
    g_side_hud->unload( );
    utility::unload();
    g_notify->unload( );

    delete g_side_hud;
    delete g_wards; delete g_cursor;
    delete g_notify; delete g_map_data;
}