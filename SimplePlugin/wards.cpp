#include "utility.h"
#include "A_.h"

c_wards* g_wards = new c_wards( );

auto is_ward( ItemId item_id ) -> bool
{
	return ( item_id == ItemId::Harrowing_Crescent || item_id == ItemId::Black_Mist_Scythe ||
		item_id == ItemId::Targons_Buckler || item_id == ItemId::Bulwark_of_the_Mountain ||
		item_id == ItemId::Frostfang || item_id == ItemId::Shard_of_True_Ice ) ||
		( item_id == ItemId::Control_Ward ||
			item_id == ItemId::Watchful_Wardstone || item_id == ItemId::Stirring_Wardstone ) ||
		( item_id == ItemId::Stealth_Ward );
}

auto c_wards::get_control_ward( ) -> spellslot
{
	auto control_ward = myhero->has_item( {
		ItemId::Control_Ward,
		ItemId::Watchful_Wardstone,
		ItemId::Stirring_Wardstone
		} );

	if ( control_ward == spellslot::invalid )
		return control_ward;

	auto spell_state = myhero->get_spell_state( control_ward );
	return spell_state == spell_state::Ready ? control_ward : spellslot::invalid;
}

auto c_wards::get_stealth_ward( )->spellslot
{
	auto slot_ward = myhero->has_item( {
		ItemId::Harrowing_Crescent, ItemId::Black_Mist_Scythe,
		ItemId::Targons_Buckler, ItemId::Bulwark_of_the_Mountain,
		ItemId::Frostfang, ItemId::Shard_of_True_Ice
		} );

	if ( slot_ward == spellslot::invalid )
		slot_ward = myhero->has_item( ItemId::Stealth_Ward );

	if ( slot_ward == spellslot::invalid )
		return slot_ward;

	auto spell_state = myhero->get_spell_state( slot_ward );
	return spell_state == spell_state::Ready ? slot_ward : spellslot::invalid;
}

auto c_wards::get_ward( ) -> spellslot
{
	auto slot_control = spellslot::invalid;
	auto slot_ward = spellslot::invalid;
	for ( auto slot = spellslot::item_1; slot <= spellslot::item_6; slot = static_cast< spellslot >( static_cast< int >( slot ) + 1 ) )
	{
		auto item = myhero->get_item( slot );
		auto spell_state = myhero->get_spell_state( slot );
		if ( item && spell_state == spell_state::Ready )
		{
			auto item_id = static_cast< ItemId >( item->get_item_id( ) );
			if ( item_id == ItemId::Harrowing_Crescent || item_id == ItemId::Black_Mist_Scythe ||
				item_id == ItemId::Targons_Buckler || item_id == ItemId::Bulwark_of_the_Mountain ||
				item_id == ItemId::Frostfang || item_id == ItemId::Shard_of_True_Ice )
				slot_ward = slot;

			if ( item_id == ItemId::Control_Ward ||
				item_id == ItemId::Watchful_Wardstone || item_id == ItemId::Stirring_Wardstone )
				slot_control = slot;
		}
	}

	if ( slot_ward == spellslot::invalid )
	{
		auto item_trinket = myhero->get_item( spellslot::trinket );
		if ( item_trinket && myhero->get_spell_state( spellslot::trinket ) == spell_state::Ready )
		{
			auto item_id = static_cast< ItemId >( item_trinket->get_item_id( ) );
			if ( item_id == ItemId::Stealth_Ward )
				slot_ward = spellslot::trinket;
		}
	}

	return slot_ward == spellslot::invalid ? slot_control : slot_ward;
}

auto c_wards::on_issue_order( _issue_order_type& type ) -> void
{
	if ( type != _issue_order_type::MoveTo )
		return;

	//myhero->print_chat( 0x3, "MoveTo" );

	last_assist_spot = std::nullopt;

	last_slot = spellslot::invalid;
	last_cast = -8.f;

	b_reach_proj = false;
}

auto c_wards::on_tick( ) -> void
{
	if ( !last_assist_spot.has_value( ) || last_slot == spellslot::invalid )
		return;

	if ( gametime->get_time( ) - last_cast >= 4.f )
		return;

	auto item = myhero->get_item( last_slot );
	if ( !item )
		return;

	if ( myhero->get_spell_state( last_slot ) != spell_state::Ready )
		return;

	auto item_id = static_cast< ItemId >( item->get_item_id( ) );
	if ( !is_ward( item_id ) )
		return;

	if ( last_assist_spot.value( ).walk_position.distance( myhero ) > 0.f )
		return;

	if ( !orbwalker->none_mode( ) )
		return;

	myhero->issue_order( _issue_order_type::Stop );

	auto cast_position = /*myhero->get_position( ).extend( */last_assist_spot.value( ).cast_position/*, 600.f )*/;

	myhero->cast_spell( last_slot, cast_position, false );
	myhero->issue_order( _issue_order_type::Stop );
	last_assist_spot = std::nullopt;
	last_slot = spellslot::invalid;
	last_cast = -8.f;
}

auto c_wards::on_cast_spell( spellslot spell_slot, bool* process, bool enabled ) -> void
{
	if ( !enabled )
		return;

	auto item = myhero->get_item( spell_slot );
	if ( !item )
		return;

	auto item_id = static_cast< ItemId >( item->get_item_id( ) );
	if ( !is_ward( item_id ) )
		return;

	auto spots = g_wards->v_trick_wards;
	if ( spots.empty( ) )
		return;

	std::sort( spots.begin( ), spots.end( ), [ ]( s_trick_wards a, s_trick_wards b )
	{
		return a.walk_position.distance( myhero ) < b.walk_position.distance( myhero );
	} );

	auto&& walk_position = spots.front( ).walk_position;
	if ( walk_position.distance( myhero ) > WARD_CAST_ASSIST_RADIUS )
		return;

	*process = false;
	last_assist_spot = spots.front( );
	auto pos = myhero->get_position( ).extend( walk_position, -200 );
	if ( myhero->get_distance( walk_position ) < 70 && myhero->get_distance( walk_position ) > 5 )
	{
		myhero->issue_order( pos, false, false );
	}
	if ( myhero->get_distance( walk_position ) >= 70 )
	{
		myhero->issue_order( walk_position, false, false );
	}
	last_cast = gametime->get_time( );
	last_slot = spell_slot;
}

auto c_wards::auto_ward( int auto_level, bool auto_control, bool auto_stealth, bool hotkey ) -> void
{
	static auto t_last_autoward = -8.f;
	if ( gametime->get_time( ) - t_last_autoward <= 0.2f )
		return;

	t_last_autoward = gametime->get_time( );
	if ( !myhero || myhero->is_dead( ) )
		return;

	if ( m_ward_points.empty( ) )
		return;

	auto ward_control = get_control_ward( ),
		ward_stealth = get_stealth_ward( );

	if ( ward_control == spellslot::invalid && ward_stealth == spellslot::invalid )
		return;

	auto&& current_wards = entitylist->get_all_minions( );
	auto is_warded = [ & ]( vector v_position ) -> bool
	{
		auto closest_ward = std::find_if( current_wards.begin( ), current_wards.end( ), [ v_position ]( game_object_script x )
		{
			if ( !x || !x->is_valid( ) )
				return false;

			auto name_hash = buff_hash_real( x->get_name_cstr( ) );
			if ( name_hash == buff_hash( "FakeCrab" ) && x->get_distance( v_position ) <= 500.f )
				return true;

			if ( x->is_ally( ) && x->get_distance( v_position ) <= 200.f )
			{
				if ( ( name_hash == buff_hash( "VisionWard" ) || ( name_hash == buff_hash( "SightWard" ) && x->get_max_health( ) != 1 ) || name_hash == buff_hash( "JammerDevice" ) ) &&
					x->get_mana( ) > 15 )
					return true;
			}
			return false;
		} );

		return ( closest_ward != current_wards.end( ) );
	};

	auto get_closest_spot = [ & ]( ) -> std::optional<vector>
	{
		auto m_spots = m_ward_points;
		m_spots.erase( std::remove_if( m_spots.begin( ), m_spots.end( ), [ & ]( c_wards::s_ward_point p ) {
			if ( p.is_control_ward )
				return true;

			auto v_position = vector( p.position.x, p.position.z, p.position.y );
			if ( myhero->get_distance( v_position ) > 590.f )
				return true;

			if ( is_warded( v_position ) )
				return true;

			return false;
		} ), m_spots.end( ) );

		if ( m_spots.empty( ) )
			return std::nullopt;

		std::sort( m_spots.begin( ), m_spots.end( ), [ from = hud->get_hud_input_logic( )->get_game_cursor_position( ) ]( c_wards::s_ward_point p1, c_wards::s_ward_point p2 ) {
			auto v_position1 = vector( p1.position.x, p1.position.z, p1.position.y ),
				v_position2 = vector( p2.position.x, p2.position.z, p2.position.y );

		return v_position1.distance( from ) < v_position2.distance( from );
		} );

		auto&& p = m_spots.front( );

		return vector( p.position.x, p.position.z, p.position.y );
	};

	if ( hotkey && ward_stealth != spellslot::invalid )
	{
		auto closest = get_closest_spot( );
		if ( closest.has_value( ) )
		{
			myhero->cast_spell( ward_stealth, closest.value( ) );
			t_last_autoward = gametime->get_time( ) + 2.f;
		}
	}

	for ( auto&& p : m_ward_points )
	{
		if ( p.is_control_ward && ( ward_control == spellslot::invalid ) )
			continue;

		if ( !p.is_control_ward && ( ward_stealth == spellslot::invalid ) )
			continue;

		auto v_position = vector( p.position.x, p.position.z, p.position.y );
		if ( myhero->get_distance( v_position ) > 590.f )
			continue;

		if ( is_warded( v_position ) )
			continue;

		auto is_auto_ward = false;
		if ( auto_level > 0 )
			is_auto_ward = ( ( p.is_control_ward && auto_control && ward_control != spellslot::invalid ) || ( !p.is_control_ward && auto_stealth && ward_stealth != spellslot::invalid ) ) && ( p.is_automatic || auto_level > 1 );

		if ( !is_auto_ward )
			continue;

		myhero->cast_spell( p.is_control_ward ? ward_control : ward_stealth, v_position );
		t_last_autoward = gametime->get_time( ) + 2.f;
	}
}

auto c_wards::draw_ward_points( bool enabled_yellow, bool control_enabled, int auto_level, bool auto_control, bool auto_stealth ) -> void
{
	if ( !myhero || myhero->is_dead( ) )
		return;

	if ( m_ward_points.empty( ) )
		return;

	auto ward_control = get_control_ward( ),
		ward_stealth = get_stealth_ward( );

	auto current_wards = entitylist->get_all_minions( );
	for ( auto&& p : m_ward_points )
	{
		auto cant_ward = ( ward_control == spellslot::invalid && ward_stealth == spellslot::invalid );

		if ( p.is_control_ward && ( !control_enabled || ward_control == spellslot::invalid ) )
			cant_ward = true;

		if ( !p.is_control_ward && ( !enabled_yellow || ward_stealth == spellslot::invalid ) )
			cant_ward = true;

		if ( !p.position.is_valid( ) )
			continue;

		auto v_position = vector( p.position.x, p.position.z, p.position.y );
		if ( myhero->get_distance( v_position ) > 1750.f )
			cant_ward = true;

		//add crab ward check
		auto closest_ward = std::find_if( current_wards.begin( ), current_wards.end( ), [ v_position ]( game_object_script x ) 
		{
			if ( x && x->is_valid( ) && x->is_ally( ) && x->get_distance( v_position ) <= 150.f )
			{
				auto name_hash = spell_hash_real( x->get_name_cstr( ) );
				if ( ( name_hash == spell_hash( "VisionWard" ) || name_hash == spell_hash( "SightWard" ) || name_hash == spell_hash( "JammerDevice" ) ) &&
					x->get_mana( ) > 15 )
				return true;
			}

			return false;
		} );

		if ( closest_ward != current_wards.end( ) )
			cant_ward = true;

		auto item = database->get_item_by_id( p.is_control_ward ? ItemId::Control_Ward : ItemId::Stealth_Ward );
		if ( !item )
			cant_ward = true;

		auto texture = item->get_texture( );
		if ( !texture.first )
			continue;

		auto w2s = vector::zero;
		renderer->world_to_screen( v_position, w2s );
		if ( !w2s.is_valid( ) || !w2s.is_on_screen( ) )
			continue;

		if ( cant_ward )
		{
			static auto easing_fn = getEasingFunction( easing_functions::EaseOutSine );

			if ( gametime->get_time( ) - p.t_last_animation >= 0.02f )
			{
				p.fl_current_alpha = std::clamp( easing_fn( p.fl_current_alpha ), 0.001f, 0.999f );
				p.t_last_animation = gametime->get_time( );
			}
		}
		else
		{
			static auto easing_fn = getEasingFunction( easing_functions::EaseInSine );

			if ( gametime->get_time( ) - p.t_last_animation >= 0.02f )
			{
				p.fl_current_alpha = std::clamp( easing_fn( p.fl_current_alpha ), 0.001f, 0.999f );
				p.t_last_animation = gametime->get_time( );
			}
		}

		if ( p.fl_current_alpha <= 0.01f )
			continue;

		static auto icon_size = vector( 35, 35 );
		auto text_color = p.is_control_ward ? MAKE_COLOR( 37, 204, 247, static_cast<int>( 255.f * p.fl_current_alpha ) ) : MAKE_COLOR( 61, 61, 61, static_cast< int >( 255.f * p.fl_current_alpha ) );
		if ( myhero->get_distance( v_position ) <= 750.f )
			text_color = A_::animate_color( text_color, 60 * p.fl_current_alpha, 255 * p.fl_current_alpha );

		draw_manager->add_image( texture.first, w2s - icon_size / 2, icon_size, 45.f, { texture.second.x, texture.second.y }, { texture.second.z, texture.second.w }, { 1.f, 1.f, 1.f, p.fl_current_alpha } );
//#ifdef __TEST
//		draw_manager->add_text_on_screen( w2s, MAKE_COLOR( 255, 255, 255, 255 ), 22, "%s (%s)\nauto: %s\nbush: %s", p.is_control_ward ? "control_ward" : "stealth_ward",
//			p.is_control_ward ? ( ward_control != spellslot::invalid ? "ready" : "not ready" ) : ( ward_stealth != spellslot::invalid ? "ready" : "not ready" ),
//			p.is_automatic ? "true":"false", p.is_bush ? "true":"false" );
//#endif
		auto is_auto_ward = false;
		if ( auto_level > 0 )
			is_auto_ward = ( ( p.is_control_ward && auto_control ) || ( !p.is_control_ward && auto_stealth ) ) && ( p.is_automatic || auto_level > 1 );

		if ( is_auto_ward )
		{
			auto text_size = draw_manager->calc_text_size( 16, "auto" );
			draw_manager->add_text_on_screen( w2s + vector( -text_size.x / 2, -text_size.y / 2 ),
				A_::set_alpha( text_color, static_cast< unsigned long >( 255.f * p.fl_current_alpha ) ),
				16, "auto" );
		}
	}
}

auto c_wards::draw_trick_wards( bool enabled ) -> void
{
	if ( !myhero || myhero->is_dead( ) || !enabled )
		return;

#ifdef __TEST
	if ( last_assist_spot.has_value( ) )
	{
		draw_manager->add_line( last_assist_spot.value( ).cast_position, last_assist_spot.value( ).walk_position, MAKE_COLOR( 255, 255, 255, 220 ), 3.f );
		auto path = myhero->get_path( last_assist_spot.value( ).walk_position );
		if ( path.size( ) > 1 )
		{
			for ( auto i = 0; i < path.size( ) - 1; i++ )
			{
				if ( i != 0 )
					draw_manager->add_circle( path [ i ], 12.f, MAKE_COLOR( 0, 0, 0, 220 ), 1.f );

				draw_manager->add_line( path [ i ], path [ i + 1 ], MAKE_COLOR( 0, 255, 120, 220 ), 1.f );
			}
		}
	}
#endif
	
	if ( g_wards->v_trick_wards.empty( ) ) //map_id
		return;

	auto cant_ward = ( get_ward( ) == spellslot::invalid );

	static auto texture = draw_manager->load_texture_from_file( L"_utils\\ward.png" );
	if ( !texture )
		return;

	for ( auto&& spot : g_wards->v_trick_wards )
	{
		if ( cant_ward || spot.walk_position.distance( myhero ) > 1750.f )
		{
			static auto easing_fn = getEasingFunction( easing_functions::EaseOutSine );

			if ( gametime->get_time( ) - spot.t_last_animation >= 0.02f )
			{
				spot.fl_current_alpha = std::clamp( easing_fn( spot.fl_current_alpha ), 0.001f, 0.999f );
				spot.t_last_animation = gametime->get_time( );
			}
		}
		else
		{
			static auto easing_fn = getEasingFunction( easing_functions::EaseInSine );

			if ( gametime->get_time( ) - spot.t_last_animation >= 0.02f )
			{
				spot.fl_current_alpha = std::clamp( easing_fn( spot.fl_current_alpha ), 0.001f, 0.999f );
				spot.t_last_animation = gametime->get_time( );
			}
		}

		if ( spot.fl_current_alpha <= 0.01f )
			continue;

		auto inside = spot.walk_position.distance( myhero ) <= WARD_CAST_ASSIST_RADIUS;

		auto dcolor = inside ? A_::animate_color( MAKE_COLOR( 156, 136, 255, 255 ), spot.fl_current_alpha * 80.f, spot.fl_current_alpha * 255.f ) :
			MAKE_COLOR( 223, 249, 251, static_cast< int >( spot.fl_current_alpha * 255.f ) );

		draw_manager->add_circle_with_glow( spot.walk_position,
			dcolor, 80.f, 2.f, glow_data( 0.82f, 0.45f, 1.f, 0.33f ) );

#ifdef __TEST
		//draw_manager->add_text( hud->get_hud_input_logic( )->get_game_cursor_position( ), MAKE_COLOR( 255, 255, 255, 255 ), 22, "%.3f|%.3f|%.3f",
		//	hud->get_hud_input_logic( )->get_game_cursor_position( ).x, hud->get_hud_input_logic( )->get_game_cursor_position( ).z, hud->get_hud_input_logic( )->get_game_cursor_position( ).y );
#endif

		if ( texture )
		{
			auto w2s = vector::zero;
			renderer->world_to_screen( spot.walk_position, w2s );
			if ( w2s.is_valid( ) && w2s.is_on_screen( ) )
			{
				static auto icon_size = vector( 60, 60 );
				draw_manager->add_image( texture->texture, w2s - icon_size / 2, icon_size, 0.f, vector::zero, { 1.f, 1.f }, { 1.f, 1.f, 1.f, spot.fl_current_alpha }, { 0, 0, 0, 0 } );
			}
		}

		if ( inside )
		{
			auto ward_texture = database->get_item_by_id( ItemId::Stealth_Ward )->get_texture( );
			static auto size = vector( 60 * 0.75f, 60 * 0.75f );

			auto w2s = vector::zero;
			renderer->world_to_screen( spot.cast_position - ( size / 2 ), w2s );

			if ( !w2s.is_valid( ) || !w2s.is_on_screen( ) )
				continue;

			draw_manager->add_image( ward_texture.first, w2s, size, 45.f, { ward_texture.second.x, ward_texture.second.y }, { ward_texture.second.z, ward_texture.second.w } );
		}
	}
}