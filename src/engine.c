/**
 *
 * Base engine file
 * 
 * engine.c 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "script.h"
#include "entities.h"
#include "world.h"
#include "texture.h"
#include "renderer.h"
#include "font.h"
#include "filesystem.h"

engine_t * engine;

float E_Sys_Milliseconds();

BOOL engine_init(int w, int h) {
    engine = malloc(sizeof(engine_t));
    memset(engine, 0, sizeof(engine_t));

    engine->isRunning = YES;
    engine->renderHeight = h;
    engine->renderWidth = w;

    FS_Init();
    RENDERER_Init(w, h);
    SCRIPTING_Init();
    TEX_Init();
    ENTITY_Init();
    WORLD_Init();
    FONT_Init();

    SCRIPTING_AfterLoaded();

    return YES;
}

void engine_shutdown() {
    engine->isRunning = NO;

    WORLD_Destroy();
    ENTITY_Destroy();
    SCRIPTING_Destory();
    RENDERER_Destroy();

    free(engine);
    engine = NULL;
}

BOOL engine_update() {
    float delta;
    float currentTime;

    currentTime = E_Sys_Milliseconds();
    delta = (float) (currentTime - engine->lastRenderTime);

    SCRIPTING_Update(delta);
    WORLD_Update(delta);
    REN_Update(delta);

    engine->lastRenderTime = currentTime;

    return engine->isRunning;
}

void engine_drawframe() {
    REN_HostFrame();
}

void engine_report_key(unsigned long keySym) {
    SCRIPTING_Key(keySym);
}

void engine_report_touch(char buttonNum, int x, int y) {
    printf("Button %d pressed at %d, %d\n", buttonNum, x, y);
    SCRIPTING_Touch(buttonNum, x, y);
}

void engine_report_move(int deltaX, int deltaY) {
    SCRIPTING_Move(deltaX, deltaY);
}

#ifndef WIN32
#include <sys/time.h>
float E_Sys_Milliseconds() {
    struct timeval tp;
    static int secbase;

    gettimeofday(&tp, 0);

    if (!secbase) {
        secbase = tp.tv_sec;
        return tp.tv_usec / 1000.0f;
    }

    return (tp.tv_sec - secbase) * 1000.0f + tp.tv_usec / 1000.0f;
}
#else
#include "windows.h"
#include "MMSystem.h"
float E_Sys_Milliseconds()
{
    return (float)timeGetTime();
}
#endif

