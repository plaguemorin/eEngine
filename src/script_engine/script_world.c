/*
 * script_world.c
 *
 *  Created on: 2011-10-21
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
#include "entities.h"
#include "renderer.h"
#include "world.h"

#include "script_priv.h"

static int node_move(lua_State *L) {
    scene_node_t * p;
    float x, y, z;
    int argc;

    argc = lua_gettop(L);

    if (argc != 4) {
        printf("method move_entity takes 4 parameters: <entity>, <x>, <y>, <z>\n");
        return 0;
    }

    if (lua_isuserdata(L, -4)) {
        p = lua_touserdata(L, -4);
    } else {
        printf("[SCRIPT] node_move: Well, param 1 was not a user data\n");
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

static int node_rotate(lua_State * L) {
    scene_node_t * p;
    float x, y, z;
    int argc = lua_gettop(L);

    if (argc != 4) {
        printf("method entity_rotate takes 4 parameters: <entity>, <rot_x>, <rot_y>, <rot_z>\n");
        return 0;
    }

    if (lua_isuserdata(L, -4)) {
        p = lua_touserdata(L, -4);
    } else {
        printf("[SCRIPT] node_rotate: Well, param 1 was not a user data\n");
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

static int node_addanimesh(lua_State * L) {
    scene_node_t * woi;
    scene_node_t * attachTo;
    entity_t * entity;

    if (lua_gettop(L) != 2) {
        printf("addAnimatedMesh takes 2 params");
        return 0;
    }

    if (lua_isuserdata(L, -1)) {
        entity = (entity_t *) lua_touserdata(L, -1);
    } else {
        printf("[SCRIPT] node_addanimesh: Well, param 2 was not a user data\n");
        return 0;
    }

    if (lua_isuserdata(L, -2)) {
        attachTo = (scene_node_t *) lua_touserdata(L, -2);
    } else {
        printf("[SCRIPT] node_addanimesh: Well, param 1 was not a user data\n");
        return 0;
    }

    /* Attach it to the world */
    woi = WORLD_AttachObjectToWorld(attachTo, entity);

    /* Set script data on */
    woi->script_data = (world_entity *) malloc(sizeof(world_entity));
    memset(woi->script_data, 0, sizeof(world_entity));

    /* Return it to script */
    lua_pushlightuserdata(L, woi);

    return 1;
}

static int node_dummy(lua_State * L) {
    scene_node_t * woi;
    scene_node_t * attachTo;

    if (lua_gettop(L) != 1) {
        printf("addDummy takes 1 params");
        return 0;
    }

    if (lua_isuserdata(L, -1)) {
        attachTo = (scene_node_t *) lua_touserdata(L, -1);
    } else if (lua_isnil(L, -1)) {
        attachTo = NULL;
    } else {
        printf("[SCRIPT] addDummy: Well, param 1 was not a user data\n");
        return 0;
    }

    woi = WORLD_AddDummyNode(attachTo);

    /* Return it to script */
    lua_pushlightuserdata(L, woi);

    return 1;
}

static const luaL_Reg entityLib[] = { { "move", node_move }, { "rotate", node_rotate }, { "addAnimatedMesh", node_addanimesh }, { "addDummy", node_dummy }, { NULL, NULL } };

int luaopen_dengine_world(lua_State *L) {
    luaL_register(L, "world", entityLib);

    return 0;
}
