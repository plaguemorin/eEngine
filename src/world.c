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
#include "world.h"

static scene_node_t * WORLD_CreateNewChildNodeOf(scene_node_t * parent) {
    scene_node_t * ewi;
    scene_node_t * new_scene_node;

    new_scene_node = (scene_node_t*) malloc(sizeof(scene_node_t));

    if (new_scene_node) {
        memset(new_scene_node, 0, sizeof(scene_node_t));
        new_scene_node->is_active = TRUE;

        ewi = (parent) ? parent : engine->world->objects;
        if (ewi->child) {
            ewi = ewi->child;
            while (ewi->next)
                ewi = ewi->next;
            ewi->next = new_scene_node;
        } else {
            ewi->child = new_scene_node;
        }
    }

    return new_scene_node;

}

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

    engine->world->objects = (scene_node_t*) malloc(sizeof(scene_node_t));
    memset(engine->world->objects, 0, sizeof(scene_node_t));
    engine->world->objects->type = NODE_TYPE_ROOT;
    engine->world->objects->is_active = TRUE;

    return YES;
}

void WORLD_Update(float delta) {

}

BOOL WORLD_Destroy() {

    return YES;
}

scene_node_t * WORLD_AttachObjectToWorld(scene_node_t * parent, entity_t * obj) {
    scene_node_t * ewi;

    if (!obj) {
        printf("[WORLD] No object to load !\n");
        return NULL;
    }

    engine->world->num_objects++;
    ewi = WORLD_CreateNewChildNodeOf(parent);

    ewi->type = NODE_TYPE_STATIC_MESH;
    ewi->object.mesh = obj;
    ewi->has_collision = NO;

    obj->num_references++;

    return ewi;
}

scene_node_t * WORLD_AddDummyNode(scene_node_t * parent) {
    scene_node_t * ewi;
    ewi = WORLD_CreateNewChildNodeOf(parent);

    ewi->type = NODE_TYPE_DUMMY;
    ewi->has_collision = NO;

    return ewi;
}
