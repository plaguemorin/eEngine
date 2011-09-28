
/**
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include "engine.h" 
#include "script.h"

static void report_errors(lua_State *L, int status);

BOOL SCRIPTING_Init(engine_t * engine) {
	lua_State *L = lua_open();
	
	luaopen_io(L); // provides io.*
    luaopen_base(L);
    luaopen_table(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_loadlib(L);
    
    engine->lua = L;

	return YES;
}

BOOL SCRIPTING_Destory(engine_t * engine) {
    lua_close(engine->lua);
	
	return YES;
}

BOOL SCRIPTING_AfterLoaded(engine_t * engine) {
	int s;
	
	// Load initial script
    s = luaL_loadfile(engine->lua, "script.lua");

    // If there was no error, then execute it
    if ( s==0 ) {
      s = lua_pcall(engine->lua, 0, LUA_MULTRET, 0);
    }
    
    // Try and report errors at this state
    report_errors(engine->lua, s);
    
    return (s == 0) ? YES : NO;
}

static void report_errors(lua_State *L, int status) {
  if ( status!=0 ) {
    printf(" %s\n", lua_tostring(L, -1));
    lua_pop(L, 1); // remove error message
  }
}
