#include <cmath>
#include <map>

#include "A_.h"

#undef M_PI

//https://github.com/nicolausYes/easing-functions
float easeInSine( float t ) {
    return sin( 1.5707963f * t );
}

float easeOutSine( float t ) {
    return 1.f + sin( 1.5707963f * ( --t ) );
}

float easeInOutSine( float t ) {
    return 0.5f * ( 1.f + sin( PI * ( t - 0.5f ) ) );
}

float easeInQuad( float t ) {
    return t * t;
}

float easeOutQuad( float t ) {
    return t * ( 2 - t );
}

float easeInOutQuad( float t ) {
    return t < 0.5f ? 2.f * t * t : t * ( 4.f - 2.f * t ) - 1;
}

float easeInCubic( float t ) {
    return t * t * t;
}

float easeOutCubic( float t ) {
    return 1 + ( --t ) * t * t;
}

float easeInOutCubic( float t ) {
    return t < 0.5f ? 4.f * t * t * t : 1.f + ( --t ) * ( 2.f * ( --t ) ) * ( 2.f * t );
}

float easeInQuart( float t ) {
    t *= t;
    return t * t;
}

float easeOutQuart( float t ) {
    t = ( --t ) * t;
    return 1.f - t * t;
}

float easeInOutQuart( float t ) {
    if ( t < 0.5f ) {
        t *= t;
        return 8.f * t * t;
    }
    else {
        t = ( --t ) * t;
        return 1.f - 8.f * t * t;
    }
}

float easeInQuint( float t ) {
    float t2 = t * t;
    return t * t2 * t2;
}

float easeOutQuint( float t ) {
    float t2 = ( --t ) * t;
    return 1.f + t * t2 * t2;
}

float easeInOutQuint( float t ) {
    float t2;
    if ( t < 0.5f ) {
        t2 = t * t;
        return 16.f * t * t2 * t2;
    }
    else {
        t2 = ( --t ) * t;
        return 1.f + 16.f * t * t2 * t2;
    }
}

float easeInExpo( float t ) {
    return ( powf( 2.f, 8.f * t ) - 1.f ) / 255.f;
}

float easeOutExpo( float t ) {
    return 1.f - powf( 2.f, -8.f * t );
}

float easeInOutExpo( float t ) {
    if ( t < 0.5f ) {
        return ( powf( 2.f, 16.f * t ) - 1.f ) / 510.f;
    }
    else {
        return 1.f - 0.5f * powf( 2.f, -16.f * ( t - 0.5f ) );
    }
}

float easeInCirc( float t ) {
    return 1 - sqrt( 1.f - t );
}

float easeOutCirc( float t ) {
    return sqrt( t );
}

float easeInOutCirc( float t ) {
    if ( t < 0.5f ) {
        return ( 1.f - sqrt( 1.f - 2.f * t ) ) * 0.5f;
    }
    else {
        return ( 1.f + sqrt( 2.f * t - 1.f ) ) * 0.5f;
    }
}

float easeInBack( float t ) {
    return t * t * ( 2.70158f * t - 1.70158f );
}

float easeOutBack( float t ) {
    return 1.f + ( --t ) * t * ( 2.70158f * t + 1.70158f );
}

float easeInOutBack( float t ) {
    if ( t < 0.5f ) {
        return t * t * ( 7.f * t - 2.5f ) * 2.f;
    }
    else {
        return 1.f + ( --t ) * t * 2.f * ( 7.f * t + 2.5f );
    }
}

float easeInElastic( float t ) {
    float t2 = t * t;
    return t2 * t2 * sin( t * PI * 4.5f );
}

float easeOutElastic( float t ) {
    float t2 = ( t - 1.f ) * ( t - 1.f );
    return 1.f - t2 * t2 * cos( t * PI * 4.5f );
}

float easeInOutElastic( float t ) {
    float t2;
    if ( t < 0.45f ) {
        t2 = t * t;
        return 8.f * t2 * t2 * sin( t * PI * 9.f );
    }
    else if ( t < 0.55f ) {
        return 0.5f + 0.75f * sin( t * PI * 4.f );
    }
    else {
        t2 = ( t - 1.f ) * ( t - 1.f );
        return 1.f - 8 * t2 * t2 * sin( t * PI * 9.f );
    }
}

float easeInBounce( float t ) {
    return powf( 2.f, 6.f * ( t - 1.f ) ) * abs( sin( t * PI * 3.5f ) );
}

float easeOutBounce( float t ) {
    return 1.f - pow( 2.f, -6.f * t ) * abs( cos( t * PI * 3.5f ) );
}

float easeInOutBounce( float t ) {
    if ( t < 0.5f ) {
        return 8.f * pow( 2.f, 8.f * ( t - 1.f ) ) * abs( sin( t * PI * 7.f ) );
    }
    else {
        return 1.f - 8.f * powf( 2.f, -8.f * t ) * abs( sin( t * PI * 7.f ) );
    }
}

easingFunction getEasingFunction( easing_functions function )
{
    static std::map< easing_functions, easingFunction > easingFunctions;
    if ( easingFunctions.empty( ) )
    {
        easingFunctions.insert( std::make_pair( EaseInSine, easeInSine ) );
        easingFunctions.insert( std::make_pair( EaseOutSine, easeOutSine ) );
        easingFunctions.insert( std::make_pair( EaseInOutSine, easeInOutSine ) );
        easingFunctions.insert( std::make_pair( EaseInQuad, easeInQuad ) );
        easingFunctions.insert( std::make_pair( EaseOutQuad, easeOutQuad ) );
        easingFunctions.insert( std::make_pair( EaseInOutQuad, easeInOutQuad ) );
        easingFunctions.insert( std::make_pair( EaseInCubic, easeInCubic ) );
        easingFunctions.insert( std::make_pair( EaseOutCubic, easeOutCubic ) );
        easingFunctions.insert( std::make_pair( EaseInOutCubic, easeInOutCubic ) );
        easingFunctions.insert( std::make_pair( EaseInQuart, easeInQuart ) );
        easingFunctions.insert( std::make_pair( EaseOutQuart, easeOutQuart ) );
        easingFunctions.insert( std::make_pair( EaseInOutQuart, easeInOutQuart ) );
        easingFunctions.insert( std::make_pair( EaseInQuint, easeInQuint ) );
        easingFunctions.insert( std::make_pair( EaseOutQuint, easeOutQuint ) );
        easingFunctions.insert( std::make_pair( EaseInOutQuint, easeInOutQuint ) );
        easingFunctions.insert( std::make_pair( EaseInExpo, easeInExpo ) );
        easingFunctions.insert( std::make_pair( EaseOutExpo, easeOutExpo ) );
        easingFunctions.insert( std::make_pair( EaseInOutExpo, easeInOutExpo ) );
        easingFunctions.insert( std::make_pair( EaseInCirc, easeInCirc ) );
        easingFunctions.insert( std::make_pair( EaseOutCirc, easeOutCirc ) );
        easingFunctions.insert( std::make_pair( EaseInOutCirc, easeInOutCirc ) );
        easingFunctions.insert( std::make_pair( EaseInBack, easeInBack ) );
        easingFunctions.insert( std::make_pair( EaseOutBack, easeOutBack ) );
        easingFunctions.insert( std::make_pair( EaseInOutBack, easeInOutBack ) );
        easingFunctions.insert( std::make_pair( EaseInElastic, easeInElastic ) );
        easingFunctions.insert( std::make_pair( EaseOutElastic, easeOutElastic ) );
        easingFunctions.insert( std::make_pair( EaseInOutElastic, easeInOutElastic ) );
        easingFunctions.insert( std::make_pair( EaseInBounce, easeInBounce ) );
        easingFunctions.insert( std::make_pair( EaseOutBounce, easeOutBounce ) );
        easingFunctions.insert( std::make_pair( EaseInOutBounce, easeInOutBounce ) );

    }

    auto it = easingFunctions.find( function );
    return it == easingFunctions.end( ) ? nullptr : it->second;
}