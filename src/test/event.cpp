#define CATCH_CONFIG_MAIN

#include <gs/event.h>
#include <catch.hpp>

using namespace gs;

TEST_CASE( "Event Triggering", "[event]" )
{
	event<int> e;
	
	bool triggered = false;
	
	auto token = e.subscribe( [&]( int code )
	{
		if( code == 0 )
		{
			triggered = true;
		}
	} );
	
	e( 0 );

	REQUIRE( triggered == true );

	triggered = false;
	e.unsubscribe( token );
	
	e( 0 );

	REQUIRE( triggered == false );
}