/*
 * renderer.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <math.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "renderer.h"

unsigned int frames = 0;
float timeLeft = 0;
extern renderer_t * rendererInitFixed();
extern renderer_t * rendererInitProg();

static void render_render_matrix(world_object_instance_t * object, matrix_t mat);

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

    return rendererInitProg();
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
    if (engine->renderer->shutdown) {
        return engine->renderer->shutdown();
    }
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
    int i;
    char fpsText[256];
    matrix_t mat;
    world_object_instance_t * object;

    frames++;
    object = engine->world->objects;

    if (object) {
        engine->renderer->start_3D(&engine->camera);

        while (object) {
            if (object->is_active) {
                render_render_matrix(object, mat);

                i = object->object->num_objects;
                while (i--) {
                    engine->renderer->render_object(&object->object->objects[i], mat);
                }
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
    BOOL outcome;
    outcome = YES;

    if (obj->material) {
        if (obj->material->texture_diffuse && !REN_MakeTextureAvailable(obj->material->texture_diffuse)) {
            printf("[REN] Unable to make texture available\n");
            outcome = NO;
        }
    }

    if (outcome) {
        engine->renderer->register_object(obj);
    }

    return outcome;
}

BOOL REN_MakeTextureAvailable(texture_t * tex) {
    if (engine->renderer->register_texture(tex)) {
        /*
         free(tex->data);
         tex->data = NULL;
         */
        return YES;
    }

    return NO;
}

static void render_render_matrix(world_object_instance_t * object, matrix_t mat) {
    float A, B, C, D, E, F, AD, BD;

    A = cos(object->rotation[0]);
    B = sin(object->rotation[0]);
    C = cos(object->rotation[1]);
    D = sin(object->rotation[1]);
    E = cos(object->rotation[2]);
    F = sin(object->rotation[2]);

    AD = A * D;
    BD = B * D;

    mat[0] = C * E;
    mat[1] = -C * F;
    mat[2] = -D;
    mat[4] = -BD * E + A * F;
    mat[5] = BD * F + A * E;
    mat[6] = -B * C;
    mat[8] = AD * E + B * F;
    mat[9] = -AD * F + B * E;
    mat[10] = A * C;

    mat[3] = mat[7] = mat[11] = mat[12] = mat[13] = mat[14] = 0;
    mat[15] = 1;

    matrixTranslate(mat, object->position[0], object->position[1], object->position[2]);
}
