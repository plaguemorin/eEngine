/**
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "engine.h" 
#include "script.h"
#include "entities.h"
#include "renderer.h"
#include "world.h"

#include <lualib.h>

/* Decrecing, updateMethodName is called when this is less or 0.0 */
static float nextTime;

/* Initial nextTime reset (When nextTime hits 0, this value is put in it) */
static float deltaTimeUpdate;

/* Incressing, passed to updateMethodName */
static float timeSinceLastCall;

/* The method to call every deltaTimeUpdate */
static char updateMethodName[256];

/* The method to call when a button is pressed */
static char buttonPressedMethodName[256];

/* The method to call when the cursor is moved */
static char moveMethodName[256];

typedef struct s_keybinding_t {
	unsigned long keySym;
	char * function;

	struct s_keybinding_t * next;
} keybinding_t;

#define COMMAND_TYPE_KEY		1
#define COMMAND_TYPE_TOUCH		2
#define COMMAND_TYPE_MOVE		3

typedef struct s_command_t {
	char type;

	// for Type == KEY
	unsigned long keySym;

	// Used for both TYPE_MOVE and TYPE_TOUCH
	int x;
	int y;

	// Only for type TYPE_TOUCH
	char buttonNumber;

	struct s_command_t * next;
} command_t;

static keybinding_t * keybindings;
static command_t * commands;

// NATIVE METHOD
extern unsigned long keySymForString(const char * keyName);

static command_t * NewCommand(char type);
static void report_errors(lua_State *L, int status);
static int quit(lua_State *L);
static int entity_loadDummy(lua_State * L);
static int world_add(lua_State * L);
static int entity_move(lua_State *L);
static int camera_set_defaults(lua_State * L);
static int set_update_callback(lua_State *L);
static int set_move_callback(lua_State *L);
static int set_touch_callback(lua_State *L);
static int entity_rotate(lua_State * L);
static int bind(lua_State *L);

BOOL SCRIPTING_Init() {
	lua_State *L = lua_open();

	luaopen_io(L); // provides io.*
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_loadlib(L);

	/* Core */
	lua_register(L, "quit", quit);

	/* Callbacks */
	lua_register(L, "bind", bind);
	lua_register(L, "set_update_callback", set_update_callback);
	lua_register(L, "set_move_callback", set_move_callback);
	lua_register(L, "set_press_callback", set_touch_callback);

	/* World */
	lua_register(L, "world_add", world_add);
	lua_register(L, "camera_set_defaults", camera_set_defaults);

	/* Entity */
	lua_register(L, "entity_move", entity_move);
	lua_register(L, "entity_loadDummy", entity_loadDummy);
	lua_register(L, "entity_rotate", entity_rotate);

	nextTime = 0;
	deltaTimeUpdate = 1.0f / 15.0f;
	updateMethodName[0] = 0;
	buttonPressedMethodName[0] = 0;
	moveMethodName[0] = 0;
	keybindings = NULL;

	engine->lua = L;

	return YES;
}

BOOL SCRIPTING_Destory() {
	keybinding_t * nextKeyB;
	command_t * nextCmd;
	lua_close(engine->lua);

	while (keybindings) {
		nextKeyB = keybindings->next;
		free(keybindings->function);
		free(keybindings);
		keybindings = nextKeyB;
	}

	while (commands) {
		nextCmd = commands->next;
		free(commands);
		commands = nextCmd;
	}

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

void SCRIPTING_Update(float delta) {
	command_t * cmdNext;
	keybinding_t * kn;
	int status;

	nextTime -= delta;
	timeSinceLastCall += delta;

	while (commands) {
		switch (commands->type) {
			case COMMAND_TYPE_KEY:
				kn = keybindings;
				while (kn && kn->keySym != commands->keySym) {
					kn = kn->next;
				}

				if (kn) {
					lua_getglobal(engine->lua, kn->function);
					status = lua_pcall(engine->lua, 0, 0, 0);
					if (status != 0) {
						report_errors(engine->lua, status);
					}
				}
				break;

			case COMMAND_TYPE_MOVE:
				if (moveMethodName[0] != 0) {
					/* function to be called */
					lua_getglobal(engine->lua, moveMethodName);
					lua_pushnumber(engine->lua, commands->x);
					lua_pushnumber(engine->lua, commands->y);

					status = lua_pcall(engine->lua, 2, 0, 0);
					if (status != 0) {
						report_errors(engine->lua, status);
					}
				}
				break;

			case COMMAND_TYPE_TOUCH:
				if (buttonPressedMethodName[0] != 0) {
					/* function to be called */
					lua_getglobal(engine->lua, buttonPressedMethodName);
					lua_pushnumber(engine->lua, commands->buttonNumber);
					lua_pushnumber(engine->lua, commands->x);
					lua_pushnumber(engine->lua, commands->y);

					status = lua_pcall(engine->lua, 3, 0, 0);
					if (status != 0) {
						report_errors(engine->lua, status);
					}
				}
				break;
		}

		cmdNext = commands->next;
		free(commands);
		commands = cmdNext;
	}

	if (nextTime <= 0.0f) {
		if (updateMethodName[0] != 0) {
			/* function to be called */
			lua_getglobal(engine->lua, updateMethodName);
			lua_pushnumber(engine->lua, timeSinceLastCall);

			status = lua_pcall(engine->lua, 1, 0, 0);
			if (status != 0) {
				report_errors(engine->lua, status);
			}
		}
		// Call update
		nextTime = deltaTimeUpdate;
		timeSinceLastCall = 0;
	}
}

static command_t * NewCommand(char type) {
	command_t * cmd;
	cmd = commands;

	if (cmd) {
		while (cmd->next) {
			cmd = cmd->next;
		}
		cmd->next = malloc(sizeof(command_t));
		cmd = cmd->next;
	} else {
		cmd = commands = malloc(sizeof(command_t));
	}
	cmd->next = NULL;
	cmd->type = type;
	return cmd;
}

void SCRIPTING_Key(unsigned long keySym) {
	command_t * cmd;
	cmd = NewCommand(COMMAND_TYPE_KEY);
	cmd->keySym = keySym;
}

void SCRIPTING_Touch(char buttonNum, int x, int y) {
	command_t * cmd;
	cmd = NewCommand(COMMAND_TYPE_TOUCH);
	cmd->buttonNumber = buttonNum;
	cmd->x = x;
	cmd->y = y;
}

void SCRIPTING_Move(int dX, int dY) {
	command_t * cmd;
	cmd = NewCommand(COMMAND_TYPE_MOVE);
	cmd->x = dX;
	cmd->y = dY;
}

static void report_errors(lua_State *L, int status) {
	if (status != 0) {
		printf(" %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		// remove error message
	}
}


static int quit(lua_State * L) {
	engine->isRunning = NO;
	return 0;
}

static int entity_loadDummy(lua_State * L) {
	object_t * p;

	p = ENTITY_NewDummyObject();
	REN_MakeObjectAvailable(p);

	lua_pushlightuserdata(L, p);

	return 1;
}

static int world_add(lua_State * L) {
	object_t * p;
	world_object_instance_t * i;
	int argc = lua_gettop(L);

	if (argc != 1) {
		printf("world_add takes 1 param");
		return 0;
	}

	if (lua_isuserdata(L, -1)) {
		p = lua_touserdata(L, -1);
	} else {
		printf("Well, param 1 was not a user data\n");
		return 0;
	}

	printf("[SCRIPT] Adding object '%s' to the world\n", p->name);

	i = WORLD_AttachObjectToWorld(p);
	lua_pushlightuserdata(L, i);

	return 1;
}

static int entity_move(lua_State *L) {
	world_object_instance_t * p;
	float x, y, z;
	int argc = lua_gettop(L);

	if (argc != 4) {
		printf("method move_entity takes 4 parameters: <entity>, <x>, <y>, <z>\n");
		return 0;
	}

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

		vectorSet(p->position, x, y, z);
	}

	return 0;
}

static int entity_rotate(lua_State * L) {
	world_object_instance_t * p;
	float x, y, z;
	int argc = lua_gettop(L);

	if (argc != 4) {
		printf("method entity_rotate takes 4 parameters: <entity>, <rot_x>, <rot_y>, <rot_z>\n");
		return 0;
	}

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

		vectorSet(p->rotation, x, y, z);
	}

	return 0;
}

static int camera_set_defaults(lua_State * L) {
	engine->camera.aspect = (float) engine->renderWidth / (float) engine->renderHeight;
	engine->camera.zFar = 100.0f;
	engine->camera.zNear = 1.0f;
	engine->camera.fov = 45.0f;

	vectorSet(engine->camera.position, 0, 0, 0);
	vectorSet(engine->camera.up, 0, 1.0f, 0);

	return 0;
}

static int set_update_callback(lua_State *L) {
	int argc = lua_gettop(L);

	if (argc != 1) {
		printf("set_update_callback takes 1 param: the name of the method to call every 1/10 of seconds\n");
		return 0;
	}

	if (lua_isstring(L, -1)) {
		strncpy(updateMethodName, lua_tostring(L, -1), sizeof(updateMethodName) / sizeof(updateMethodName[0]));
		nextTime = 0.0f;
		printf("[SCRIPT] Update method is '%s'\n", updateMethodName);
	} else {
		printf("set_update_callback takes a string");
		return 0;
	}
	return 0;
}

static int set_move_callback(lua_State *L) {
	int argc = lua_gettop(L);

	if (argc != 1) {
		printf("set_move_callback takes 1 param: the name of the method to call\n");
		return 0;
	}

	if (lua_isstring(L, -1)) {
		strncpy(moveMethodName, lua_tostring(L, -1), sizeof(moveMethodName) / sizeof(moveMethodName[0]));
		printf("[SCRIPT] Move callback method is '%s'\n", moveMethodName);
	} else {
		printf("set_move_callback takes a string");
		return 0;
	}
	return 0;
}

static int set_touch_callback(lua_State *L) {
	int argc = lua_gettop(L);

	if (argc != 1) {
		printf("set_touch_callback takes 1 param: the name of the method to call\n");
		return 0;
	}

	if (lua_isstring(L, -1)) {
		strncpy(buttonPressedMethodName, lua_tostring(L, -1), sizeof(buttonPressedMethodName) / sizeof(buttonPressedMethodName[0]));
		printf("[SCRIPT] Touch callback method is '%s'\n", buttonPressedMethodName);
	} else {
		printf("set_touch_callback takes a string");
		return 0;
	}
	return 0;
}

static int bind(lua_State *L) {
	keybinding_t * keyb;
	int argc = lua_gettop(L);
	unsigned long keySym;
	const char * keyName;
	const char * functionName;

	if (argc != 2) {
		printf("bind takes 2 params: bind( <key name>, <function name> )\n");
		return 0;
	}

	if (!lua_isstring(L, -1) || !lua_isstring(L, -2)) {
		printf("bind parameters are strings\n");
		return 0;
	}

	keyName = lua_tostring(L, -2);
	functionName = lua_tostring(L, -1);
	keySym = keySymForString(keyName);

	printf("[SCRIPT] Binding key \"%s\" to script function named \"%s\" (native key code: %lu)\n", keyName, functionName, keySym);

	if (keybindings) {
		keyb = keybindings;
		while (keyb->next) {
			keyb = keyb->next;
		}
		keyb->next = malloc(sizeof(keybinding_t));
		keyb = keyb->next;
	} else {
		keyb = keybindings = malloc(sizeof(keybinding_t));
	}

	keyb->next = NULL;
	keyb->keySym = keySym;
	keyb->function = malloc((strlen(functionName) + 1) * sizeof(char));
	strcpy(keyb->function, functionName);

	return 0;
}
