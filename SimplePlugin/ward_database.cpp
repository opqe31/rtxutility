#include "utility.h"
#include "../plugin_sdk/plugin_sdk.hpp"

auto c_wards::load_trick_wards( ) -> void
{
    static auto map_id = missioninfo->get_map_id( );
    if ( map_id != game_map_id::SummonersRift )
        return;

    v_trick_wards =
    {

    };

    /*for ( auto i = 0; i < v_trick_wards.size( ); i++ )
    {
        v_trick_wards [ i ].cast_position.y = v_trick_wards [ i ].cast_position.z;
        v_trick_wards [ i ].cast_position.z = navmesh->get_height_for_position( v_trick_wards [ i ].cast_position.x, v_trick_wards [ i ].cast_position.y );

        v_trick_wards[i].walk_position.y =  v_trick_wards[i].walk_position.z;
        v_trick_wards [ i ].walk_position.z = navmesh->get_height_for_position( v_trick_wards [ i ].walk_position.x, v_trick_wards [ i ].walk_position.y );
    }*/
}

auto c_wards::update_wardpoints( ) -> void
{
    static auto map_id = missioninfo->get_map_id( );
    if ( map_id != game_map_id::SummonersRift )
        return;

    if ( !m_initialized_autoward ||
        next_autoward_update_time <= gametime->get_time( ) )
    {
        if ( m_initialized_autoward )
            m_ward_points.clear( );
        else
        {
            m_ward_positions = { };

            console->print( "initialize auto_ward: %d", m_ward_positions.size( ) );
        }

        

        m_registration_completed = false;
        m_initialized_autoward = true;
    }
}