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

    /* Return it to script */
    lua_pushlightuserdata(L, entity);

    return 1;
}

static const luaL_Reg entityLib[] = {  { "load", entity_load }, { NULL, NULL } };

int luaopen_dengine_entity(lua_State *L) {
    luaL_register(L, "entity", entityLib);

    return 0;
}
