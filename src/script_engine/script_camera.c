/*
 * world.c
 *
 *    lua_register(L, "world_add", world_add);
    lua_register(L, "camera_set_defaults", camera_set_defaults);

 *  Created on: 2011-10-17
 *      Author: plaguemorin
 */
#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "script_priv.h"

static int camera_set_defaults(lua_State * L) {
    engine->camera.aspect = (float) engine->renderWidth / (float) engine->renderHeight;
    engine->camera.zFar = 1000.0f;
    engine->camera.zNear = 1.0f;
    engine->camera.fov = 45.0f;

    vectorSet(engine->camera.position, 0, 0, 0);
    vectorSet(engine->camera.up, 0, 1.0f, 0);
    vectorSet(engine->camera.forward, 0, 0, -1);

    return 0;
}

static const luaL_Reg camLib[]  = {
        { "set_defaults", camera_set_defaults },
        { NULL, NULL }
};

int luaopen_dengine_camera (lua_State * L) {
    luaL_register(L, "camera", camLib);
    return 1;
}
