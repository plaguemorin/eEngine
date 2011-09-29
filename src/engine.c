/**
 *
 * Base engine file
 * 
 * engine.c 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "script.h"
#include "entities.h"
#include "world.h"
#include "renderer.h"

engine_t * engine;

int E_Sys_Milliseconds(void);

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
	float delta;
	int currentTime = (float)E_Sys_Milliseconds();

	delta = currentTime - engine->lastRenderTime;

	REN_Update(delta);
	SCRIPTING_Update(delta);

	engine->lastRenderTime = currentTime;
}

void engine_drawframe() {
	REN_HostFrame();
}

#undef malloc
void * debug_malloc(size_t s, const char * file, int line) {

	printf("[MALLOC] %lu bytes %s:%d\n", s, file, line);

	return malloc(s);
}

#ifndef WIN32
#include <sys/time.h>
int E_Sys_Milliseconds(void) {
	struct timeval tp;
	static int secbase;

	gettimeofday(&tp, 0);

	if (!secbase) {
		secbase = tp.tv_sec;
		return tp.tv_usec / 1000;
	}

	return (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;
}
#else
#include "windows.h"
#include "MMSystem.h"
int E_Sys_Milliseconds( void )
{
	return (int)timeGetTime();
}
#endif

