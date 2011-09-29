/*
 * renderer.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "renderer.h"

extern renderer_t * rendererInitProc();

static renderer_t * selectBestRender() {
	int numAttVert, numUnifVert, numUnifFrag, numVary, textureSize;

	/* Figure out if we should use GLSL or fixed path */
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttVert);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &numUnifVert);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &numUnifFrag);
	glGetIntegerv(GL_MAX_VARYING_VECTORS, &numVary);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &textureSize);

	printf("[KER] GL_MAX_VERTEX_ATTRIBS = %d \n", numAttVert);
	printf("[KER] GL_MAX_VERTEX_UNIFORM_VECTORS = %d \n", numUnifVert);
	printf("[KER] GL_MAX_FRAGMENT_UNIFORM_VECTORS = %d \n", numUnifFrag);
	printf("[KER] GL_MAX_VARYING_VECTORS = %d \n", numVary);
	printf("[KER] GL_MAX_TEXTURE_SIZE = %d\n", textureSize);

	return rendererInitProc();
}

BOOL RENDERER_Init(int w, int h) {
	engine->renderer = selectBestRender();

	if (engine->renderer) {
		printf("[REN] %s was selected\n", engine->renderer->name);

		return engine->renderer->init(w, h);
	}

	return NO;
}

BOOL RENDERER_Destroy() {
	return YES;
}

void REN_HostFrame() {
	world_object_instance_t * object;

	object = engine->world->objects;

	if (object) {
		engine->renderer->start_3D(&engine->camera);

		while (object) {
			if (object->is_active) {
				engine->renderer->render_object_instance(object);
			}

			object = object->next;
		}

		engine->renderer->end_3D();
	}
}

BOOL REN_MakeAvailable(object_t * obj) {
	return engine->renderer->register_object(obj);
}
