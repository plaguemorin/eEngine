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

BOOL WORLD_Init() {
    engine->world = (world_t *) malloc(sizeof(world_t));
    memset(engine->world, 0, sizeof(world_t));

    engine->camera.aspect = (float) engine->renderWidth / (float) engine->renderHeight;
    engine->camera.zFar = 1000.0f;
    engine->camera.zNear = 1.0f;
    engine->camera.fov = 45.0f;

    vectorSet(engine->camera.position, 0, 0, 0);
    vectorSet(engine->camera.up, 0, 1.0f, 0);
    vectorSet(engine->camera.forward, 0, 0, -1);

    return YES;
}

void WORLD_Update(float delta) {

}

BOOL WORLD_Destroy() {

    return YES;
}

world_object_instance_t * WORLD_AttachObjectToWorld(entity_t * obj) {
    world_object_instance_t * entity_world_instance;

    entity_world_instance = engine->world->objects;
    if (entity_world_instance) {
        while (entity_world_instance->next)
            entity_world_instance = entity_world_instance->next;
        entity_world_instance->next = malloc(sizeof(world_object_instance_t));
        entity_world_instance = entity_world_instance->next;
    } else {
        entity_world_instance = malloc(sizeof(world_object_instance_t));
        engine->world->objects = entity_world_instance;
    }
    memset(entity_world_instance, 0, sizeof(world_object_instance_t));
    engine->world->num_objects++;

    entity_world_instance->is_active = TRUE;
    entity_world_instance->object = obj;
    entity_world_instance->has_collision = NO;

    vectorSet(entity_world_instance->position, 0, 0, 0);
    vectorSet(entity_world_instance->rotation, 0, 0, 0);

    obj->num_references++;

    return entity_world_instance;
}
