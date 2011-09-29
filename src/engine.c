/**
 *
 * Base engine file
 * 
 * engine.c 
 */

#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "script.h"
#include "entities.h"
#include "world.h"
#include "renderer.h"

engine_t * engine;

BOOL engine_init(int w, int h) {
	engine = malloc(sizeof(engine_t));
	memset(engine, 0, sizeof(engine_t));

	engine->renderHeight = h;
	engine->renderWidth = w;

	SCRIPTING_Init();
	ENTITY_Init();
	WORLD_Init();
	RENDERER_Init(w, h);

	SCRIPTING_AfterLoaded();

	return YES;
}

void engine_update() {

}

void engine_drawframe() {
	REN_HostFrame();
}
