/*
 * world.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "entities.h"
#include "world.h"

BOOL WORLD_Init() {
	engine->world = malloc(sizeof(world_t));
	memset(engine->world, 0, sizeof(world_t));

	return YES;
}

BOOL WORLD_Destroy() {

	return YES;
}


world_object_instance_t * WORLD_AttachObjectToWorld(object_t * obj) {
	world_object_instance_t * point;

	point = engine->world->objects;
	if (point) {
		while(point->next) point = point->next;
		point->next = malloc(sizeof(world_object_instance_t));
		point = point->next;
	} else {
		point = malloc(sizeof(world_object_instance_t));
		engine->world->objects = point;
	}
	memset(point, 0, sizeof(world_object_instance_t));
	engine->world->num_objects++;

	point->is_active = TRUE;
	point->object = obj;
	point->has_collision = NO;

	matrix_load_identity(point->transformation);

	return point;
}
