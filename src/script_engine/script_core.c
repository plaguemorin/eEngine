/*
 * script_core.c
 *
 *    lua_register(L, "quit", quit);
    lua_register(L, "bind", bind);
 *
 *
 *  Created on: 2011-10-17
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "script_priv.h"

static int quit(lua_State *L);
static int bind(lua_State *L);

// NATIVE METHOD
extern unsigned long keySymForString(const char * keyName);

static int quit(lua_State * L) {
    engine->isRunning = NO;
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

static const luaL_Reg coreLib[]  = {
        { "quit", quit },
        { "bind", bind },
        { NULL, NULL }
};

int luaopen_dengine_core (lua_State * L) {
    luaL_register(L, "core", coreLib);
    return 1;
}
