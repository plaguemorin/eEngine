/*
 * script_entity.c
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
#include "entities.h"
#include "renderer.h"
#include "world.h"

#include "script_priv.h"

static int entity_load(lua_State * L) {
    entity_t * entity;
    world_object_instance_t * woi;
    const char * path;
    int i;

    if (lua_gettop(L) != 1) {
        printf("entity_load takes 1 param");
        return 0;
    }

    path = lua_tostring(L, -1);

    /* Load the Object and make the mesh available to the renderer */
    entity = ENTITY_LoadObject(path);
    if (entity) {
        i = entity->num_objects;
        while (i--) {
            REN_MakeObjectAvailable(&entity->objects[i]);
        }
    }

    /* Attach it to the world */
    woi = WORLD_AttachObjectToWorld(entity);

    /* Set script data on */
    woi->script_data = (world_entity *) malloc(sizeof(world_entity));
    memset(woi->script_data, 0, sizeof(world_entity));

    /* Return it to script */
    lua_pushlightuserdata(L, woi);

    return 1;
}

static int entity_move(lua_State *L) {
    world_object_instance_t * p;
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

static const luaL_Reg entityLib[] = { { "move", entity_move }, { "load", entity_load }, { "rotate", entity_rotate }, { NULL, NULL } };

int luaopen_dengine_entity(lua_State *L) {
    luaL_register(L, "entity", entityLib);

    return 0;
}
