/**
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include "engine.h" 
#include "script.h"
#include "entities.h"
#include "world.h"

#include <lualib.h>

static void report_errors(lua_State *L, int status);
static int entity_loadDummy(lua_State * L);
static int world_add(lua_State * L);
static int move_entity(lua_State *L);
static int camera_set_defaults(lua_State * L);

BOOL SCRIPTING_Init() {
	lua_State *L = lua_open();

	luaopen_io(L); // provides io.*
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_loadlib(L);

	lua_register(L, "entity_loadDummy", entity_loadDummy);
	lua_register(L, "world_add", world_add);
	lua_register(L, "move_entity", move_entity);
	lua_register(L, "camera_set_defaults", camera_set_defaults);

	engine->lua = L;

	return YES;
}

BOOL SCRIPTING_Destory() {
	lua_close(engine->lua);

	return YES;
}

BOOL SCRIPTING_AfterLoaded() {
	int s;

	// Load initial script
	s = luaL_loadfile(engine->lua, "script.lua");

	// If there was no error, then execute it
	if (s == 0) {
		s = lua_pcall(engine->lua, 0, LUA_MULTRET, 0);
	}

	// Try and report errors at this state
	report_errors(engine->lua, s);

	return (s == 0) ? YES : NO;
}

static void report_errors(lua_State *L, int status) {
	if (status != 0) {
		printf(" %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		// remove error message
	}
}

static int entity_loadDummy(lua_State * L) {
	int argc;
	object_t * p;

	argc = lua_gettop(L);
	p = ENTITY_NewDummyObject();
	REN_MakeAvailable(p);

	lua_pushlightuserdata(L, p);

	return 1;
}

static int world_add(lua_State * L) {
	object_t * p;
	world_object_instance_t * i;
	int argc = lua_gettop(L);

	if (lua_isuserdata(L, -1)) {
		p = lua_touserdata(L, -1);
	} else {
		printf("Well, param 1 was not a user data\n");
		return 0;
	}

	printf("Adding object '%s' to the world\n", p->name);

	i = WORLD_AttachObjectToWorld(p);
	lua_pushlightuserdata(L, i);

	return 1;
}

static int move_entity(lua_State *L) {
	world_object_instance_t * p;
	float x, y, z;
	int argc = lua_gettop(L);

	if (lua_isuserdata(L, -4)) {
		p = lua_touserdata(L, -4);
	} else {
		printf("Well, param 1 was not a user data\n");
		return 0;
	}

	if (lua_isnumber(L, -1) && lua_isnumber(L, -2) && lua_isnumber(L, -3)) {
		z = lua_tonumber(L, -1);
		y = lua_tonumber(L, -2);
		x = lua_tonumber(L, -3);

		matrixTranslate(p->transformation, x, y, z);

		matrix_print(p->transformation);
	}

	return 0;
}

static int camera_set_defaults(lua_State * L) {
	engine->camera.aspect = (float)engine->renderWidth / (float)engine->renderHeight;
	engine->camera.zFar = 100.0f;
	engine->camera.zNear = 1.0f;
	engine->camera.fov = 45.0f;

	vectorSet(engine->camera.position, 0, 0, 0);
	vectorSet(engine->camera.up, 0, 1.0f, 0);

	return 0;
}
