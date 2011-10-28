/*
 * world.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "entities.h"
#include "filesystem.h"
#include "world.h"
#include "scene_node.h"

BOOL WORLD_Init() {
    engine->world = (world_t *) malloc(sizeof(world_t));
    memset(engine->world, 0, sizeof(world_t));

    engine->camera.aspect = (float) engine->renderWidth / (float) engine->renderHeight;
    engine->camera.zFar = 1000.0f;
    engine->camera.zNear = 1.0f;
    engine->camera.fov = 45.0f;

    vectorSet(engine->camera.position, 0, 0, -1);
    vectorSet(engine->camera.up, 0, 1.0f, 0);
    vectorSet(engine->camera.forward, 0, 0, 1);

    engine->world->root_node = (scene_node_t*) malloc(sizeof(scene_node_t));
    memset(engine->world->root_node, 0, sizeof(scene_node_t));
    engine->world->root_node->type = NODE_TYPE_ROOT;
    engine->world->root_node->is_active = TRUE;

    return YES;
}

void WORLD_Update(float delta) {

}

BOOL WORLD_Destroy() {

    return YES;
}
