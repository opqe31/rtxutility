#pragma once

#include "../plugin_sdk/plugin_sdk.hpp"
#include <optional>

#ifndef BUILD_DEFS_H

// Example of __DATE__ string: "Jul 27 2012"
// Example of __TIME__ string: "21:06:19"

#define COMPUTE_BUILD_YEAR \
    ( \
        (__DATE__[ 7] - '0') * 1000 + \
        (__DATE__[ 8] - '0') *  100 + \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )


#define COMPUTE_BUILD_DAY \
    ( \
        ((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + \
        (__DATE__[5] - '0') \
    )


#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define COMPUTE_BUILD_MONTH \
    ( \
        (BUILD_MONTH_IS_JAN) ?  1 : \
        (BUILD_MONTH_IS_FEB) ?  2 : \
        (BUILD_MONTH_IS_MAR) ?  3 : \
        (BUILD_MONTH_IS_APR) ?  4 : \
        (BUILD_MONTH_IS_MAY) ?  5 : \
        (BUILD_MONTH_IS_JUN) ?  6 : \
        (BUILD_MONTH_IS_JUL) ?  7 : \
        (BUILD_MONTH_IS_AUG) ?  8 : \
        (BUILD_MONTH_IS_SEP) ?  9 : \
        (BUILD_MONTH_IS_OCT) ? 10 : \
        (BUILD_MONTH_IS_NOV) ? 11 : \
        (BUILD_MONTH_IS_DEC) ? 12 : \
        /* error default */  99 \
    )

#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')


#define BUILD_DATE_IS_BAD (__DATE__[0] == '?')

#define BUILD_YEAR  ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_YEAR)
#define BUILD_MONTH ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY   ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_DAY)

#define BUILD_TIME_IS_BAD (__TIME__[0] == '?')

#define BUILD_HOUR  ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN)
#define BUILD_SEC   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC)


#endif

#define WARD_CAST_ASSIST_RADIUS 300.f
class c_wards
{
public:
    struct s_trick_wards
    {
        vector walk_position;
        vector cast_position;

        float fl_current_alpha = 0.001f;
        float t_last_animation = -20.f;
    };

    std::vector< s_trick_wards > v_trick_wards;
    spellslot get_ward( );
    spellslot get_control_ward( );
    spellslot get_stealth_ward( );
    void load_trick_wards( );
    void draw_trick_wards( bool enabled );

    void on_cast_spell( spellslot spell_slot, bool* process, bool enabled );
    void on_tick( );
    void on_issue_order( _issue_order_type& type );
    void update_wardpoints( );
    struct s_ward_point 
    {
        vector position { };

        int value { };

        bool is_bush { };
        bool is_control_ward { };
        bool is_automatic { };

        geometry::polygon vision_area{ };
        geometry::polygon farsight_vision_area{ };

        float last_ward_time { };
        float end_time { };

        bool is_registered { };

        float fl_current_alpha = 0.001f;
        float t_last_animation = -20.f;
    };

    std::vector<s_ward_point> m_ward_points = { };
    std::vector<vector>m_ward_positions = { };
    void draw_ward_points( bool enabled, bool control_enabled, int auto_level, bool auto_control, bool auto_stealth );
    void auto_ward( int auto_level, bool auto_control, bool auto_stealth, bool hotkey );
private:
    std::optional<s_trick_wards> last_assist_spot = std::nullopt;
    bool b_reach_proj = false;

    float last_cast = -8.f;
    spellslot last_slot = spellslot::invalid;

    bool m_initialized_autoward = false;
    bool m_registration_completed = false;
    float next_autoward_update_time = 0.f;
};
extern c_wards* g_wards;

namespace utility
{
    void load();
    void unload();

    constexpr const auto COLOR_DARK_GRAY = MAKE_COLOR( 53, 59, 72, 255 );
    constexpr const auto COLOR_LGRAY = MAKE_COLOR( 178, 190, 195, 255 );
    constexpr const auto COLOR_GRAY = MAKE_COLOR( 99, 110, 114, 255 );
    constexpr const auto COLOR_WHITE = MAKE_COLOR( 236, 240, 241, 255 );
    constexpr const auto COLOR_LGREEN = MAKE_COLOR( 46, 204, 113, 255 );
    constexpr const auto COLOR_LPURPLE = MAKE_COLOR( 162, 155, 254, 255 );

    constexpr const auto COLOR_RED = MAKE_COLOR( 255, 121, 121, 255 );
    constexpr const auto COLOR_CORAL = MAKE_COLOR( 250, 177, 160, 255 );
    constexpr const auto COLOR_APPLE_GREEN = MAKE_COLOR( 106, 176, 76, 255 );
    constexpr const auto COLOR_ORANGE = MAKE_COLOR( 230, 126, 34, 255 );
    constexpr const auto COLOR_YELLOW = MAKE_COLOR( 241, 196, 15, 255 );
};

class c_utility {
public:
    float clr_default_coral [ 4 ] = { 250.f / 255.f, 177.f / 255.f, 160.f / 255.f, 1.f };
};

extern c_utility* g_utils;