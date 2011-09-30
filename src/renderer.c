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

unsigned int frames = 0;
float timeLeft = 0;
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

void REN_Update(float deltaTime) {
	timeLeft -= deltaTime;

	if (timeLeft <= 0.0f) {
		engine->framesPerSeconds = frames;
		frames = 0.0f;
		timeLeft = 1000;
	}
}

void REN_HostFrame() {
	char fpsText[256];
	world_object_instance_t * object;

	frames++;
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

	engine->renderer->start_2D(engine->renderWidth, engine->renderHeight);
	snprintf(fpsText, 255, "FPS: %d", engine->framesPerSeconds);
	engine->renderer->printString(1, 1, engine->defaultFont, fpsText);
	engine->renderer->end_2D();
}

BOOL REN_MakeObjectAvailable(object_t * obj) {
	return engine->renderer->register_object(obj);
}

BOOL REN_MakeTextureAvailable(texture_t * tex) {
	return engine->renderer->register_texture(tex);
}
