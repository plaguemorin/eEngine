/**
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h" 
#include "3d_math.h"

#include "entities.h"

static object_t * alloc_new_object(char * name) {
	object_t * obj;

	obj = malloc(sizeof(object_t));
	if (obj) {
		memset(obj, 0, sizeof(object_t));

		if (name) {
			obj->name = malloc(sizeof(char) * (strlen(name) + 1));

			if (obj->name) {
				strcpy(obj->name, name);
			}
		}
	}

	return obj;
}

BOOL ENTITY_Init() {

	return YES;
}

BOOL ENTITY_Update() {

	return YES;
}

BOOL ENTITY_Destroy() {

	return YES;
}

object_t * ENTITY_NewDummyObject() {
	object_t * obj;
	int i;

#	define X .525731112119133606
#	define Z .850650808352039932
	static float vdata[12][3] = { { -X, 0.0, Z }, { X, 0.0, Z }, { -X, 0.0, -Z }, { X, 0.0, -Z }, { 0.0, Z, X }, { 0.0, Z, -X }, { 0.0, -Z, X }, { 0.0, -Z, -X }, { Z, X, 0.0 }, {
			-Z, X, 0.0 }, { Z, -X, 0.0 }, { -Z, -X, 0.0 } };

	static unsigned short tindices[20][3] = { { 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 }, { 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 }, { 7, 10,
			3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 }, { 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 } };
#	undef X
#	undef Z
	obj = alloc_new_object("dummy object");

	obj->num_verticies = 12;
	obj->vertices = malloc(sizeof(vertex_t) * obj->num_verticies);
	for (i = 0; i < obj->num_verticies; i++) {
		vectorSet(obj->vertices[i].position, vdata[i][0], vdata[i][1], vdata[i][2]);
	}

	obj->num_indices = 20 * 3;
	obj->indices = malloc(sizeof(unsigned short) * obj->num_indices);
	for (i = 0; i <= obj->num_indices; i++) {
		obj->indices[i] = tindices[i / 3][i % 3];
	}

	return obj;
}
