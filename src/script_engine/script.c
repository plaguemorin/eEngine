/**
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h" 
#include "script.h"
#include "entities.h"
#include "renderer.h"
#include "world.h"

/* Include the Lua API header files. */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if (LUA_VERSION_NUM < 501)
#error LUA IS TOO OLD, 5.1 REQUIRED
#endif

#include "script_priv.h"

/* Decrecing, updateMethodName is called when this is less or 0.0 */
static float nextTime;

/* Initial nextTime reset (When nextTime hits 0, this value is put in it) */
static float deltaTimeUpdate;

/* Incressing, passed to updateMethodName */
static float timeSinceLastCall;

/* The method to call every deltaTimeUpdate */
char updateMethodName[256];

/* The method to call when a button is pressed */
char buttonPressedMethodName[256];

/* The method to call when the cursor is moved */
char moveMethodName[256];

/* List of key bindings */
keybinding_t * keybindings;

static command_t * commands;

static command_t * new_command_of_type(char type);
static void report_errors(lua_State *L, int status);

BOOL SCRIPTING_Init() {
    lua_State *L = luaL_newstate();

    luaopen_dengine_entity(L);
    luaopen_dengine_core(L);
    luaopen_dengine_callback(L);
    luaopen_dengine_camera(L);

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

static command_t * new_command_of_type(char type) {
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
    cmd = new_command_of_type(COMMAND_TYPE_KEY);
    cmd->keySym = keySym;
}

void SCRIPTING_Touch(char buttonNum, int x, int y) {
    command_t * cmd;
    cmd = new_command_of_type(COMMAND_TYPE_TOUCH);
    cmd->buttonNumber = buttonNum;
    cmd->x = x;
    cmd->y = y;
}

void SCRIPTING_Move(int dX, int dY) {
    command_t * cmd;
    cmd = new_command_of_type(COMMAND_TYPE_MOVE);
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

