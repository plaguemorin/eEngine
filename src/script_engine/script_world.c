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
#include "scene_node.h"

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

static int node_addmesh(lua_State * L) {
    size_t size;

    luaL_checklstring(L, 1, &size);


    return 0;
}

static int node_dummy(lua_State * L) {
    scene_node_t * node;
    size_t strSize;
    const char * name;

    if (!lua_isnil(L, 1)) {
        name = luaL_checklstring(L, 1, &strSize);
    } else {
        name = NULL;
    }

    node = SCENE_NewNode(name, NODE_TYPE_DUMMY);

    return 0;
}

static int node_test(lua_State *L) {
    size_t size;
    int a,b,c;
    const char * string;

    string = luaL_checklstring(L, 1, &size);
    a = luaL_checkinteger(L, 2);
    b = luaL_checkinteger(L, 3);
    c = luaL_checkinteger(L, 4);

    printf("test(\"%s\", %d, %d, %d);\n", string, a, b, c);

    return 0;
}

static const luaL_Reg entityLib[] = { {"test", node_test}, { "move", node_move }, { "rotate", node_rotate }, { "addNodeMesh", node_addmesh }, { "addNodeDummy", node_dummy }, { NULL, NULL } };

int luaopen_dengine_world(lua_State *L) {
    luaL_register(L, "world", entityLib);

    return 0;
}
