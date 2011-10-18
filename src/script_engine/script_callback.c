/*
 * script_callback.c
 *
 *    lua_register(L, "set_update_callback", set_update_callback);
    lua_register(L, "set_move_callback", set_move_callback);
    lua_register(L, "set_press_callback", set_touch_callback);

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

static int set_update_callback(lua_State *L) {
    int argc = lua_gettop(L);

    if (argc != 1) {
        printf("set_update_callback takes 1 param: the name of the method to call every 1/10 of seconds\n");
        return 0;
    }

    if (lua_isstring(L, -1)) {
        strncpy(updateMethodName, lua_tostring(L, -1), sizeof(updateMethodName) / sizeof(updateMethodName[0]));
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

static const luaL_Reg callBackLib[]  = {
        { "set_update_callback", set_update_callback },
        { "set_move_callback", set_move_callback },
        { "set_touch_callback", set_touch_callback },
        { NULL, NULL }
};

int luaopen_dengine_callback (lua_State * L) {
//    luaL_register(L, "core", callBackLib);
    return 1;
}
