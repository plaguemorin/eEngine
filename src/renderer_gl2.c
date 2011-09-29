/*
 * renderer_gl2.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(__APPLE__)
#	include <OpenGLES/ES2/gl.h>
#	include <OpenGLES/ES2/glext.h>
#else
#	include <GL/gl.h>
#	include <GL/glext.h>
#endif

#include "engine.h"

#define DEG_TO_RAD (2.0f*3.14159265f/360.0f)

matrix_t projectionMatrix;
matrix_t modelViewMatrix;
matrix_t textureMatrix = { 1.0f / 32767, 0, 0, 0, 0, 1.0f / 32767, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }; //Unpacking matrix

typedef enum prog_location {
    OBJECT_MEMLOC_VRAM, OBJECT_MEMLOC_RAM,
} prog_location;

typedef struct renderer_prog_object_t {
    GLuint vboId;
    prog_location memory_location;
} renderer_prog_object_t;

static void Perspective(float fovy, float aspect, float zNear, float zFar, matrix_t projectionMatrix) {
    float f = (float) (1 / tan(fovy * DEG_TO_RAD / 2));

    projectionMatrix[0] = f / aspect;
    projectionMatrix[4] = 0;
    projectionMatrix[8] = 0;
    projectionMatrix[12] = 0;
    projectionMatrix[1] = 0;
    projectionMatrix[5] = f;
    projectionMatrix[9] = 0;
    projectionMatrix[13] = 0;
    projectionMatrix[2] = 0;
    projectionMatrix[6] = 0;
    projectionMatrix[10] = (zFar + zNear) / (zNear - zFar);
    projectionMatrix[14] = 2 * (zFar * zNear) / (zNear - zFar);
    projectionMatrix[3] = 0;
    projectionMatrix[7] = 0;
    projectionMatrix[11] = -1;
    projectionMatrix[15] = 0;
}

static void LookAt(vec3_t vEye, vec3_t vLookat, vec3_t vUp, matrix_t fModelView) {
    vec3_t vN, vU, vV;

    // determine the new n
    vectorSubtract(vEye, vLookat, vN);

    // determine the new u by crossing with the up vector
    vector_cross_product(vUp, vN, vU);

    // normalize both the u and n vectors
    normalize(vU);
    normalize(vN);

    // determine v by crossing n and u
    vector_cross_product(vN, vU, vV);

    // create a model view matrix
    fModelView[0] = vU[0];
    fModelView[4] = vU[1];
    fModelView[8] = vU[2];
    fModelView[12] = -dotProduct(vEye, vU);
    fModelView[1] = vV[0];
    fModelView[5] = vV[1];
    fModelView[9] = vV[2];
    fModelView[13] = -dotProduct(vEye, vV);
    fModelView[2] = vN[0];
    fModelView[6] = vN[1];
    fModelView[10] = vN[2];
    fModelView[14] = -dotProduct(vEye, vN);
    fModelView[3] = 0.0f;
    fModelView[7] = 0.0f;
    fModelView[11] = 0.0f;
    fModelView[15] = 1.0f;

}

/**
 * Check for OpenGL errors
 */
static void CheckErrorsF(char* step, char* details) {
    glFlush();
    GLenum err = glGetError();
    switch (err) {
    case GL_INVALID_ENUM:
        printf("[GLES2] Error GL_INVALID_ENUM %s, %s\n", step, details);
        break;
    case GL_INVALID_VALUE:
        printf("[GLES2] Error GL_INVALID_VALUE  %s, %s\n", step, details);
        break;
    case GL_INVALID_OPERATION:
        printf("[GLES2] Error GL_INVALID_OPERATION  %s, %s\n", step, details);
        break;
    case GL_OUT_OF_MEMORY:
        printf("[GLES2] Error GL_OUT_OF_MEMORY  %s, %s\n", step, details);
        break;
    case GL_NO_ERROR:
        break;
    default:
        printf("[GLES2] Error UNKNOWN  %s, %s \n", step, details);
        break;
    }
}

/**
 * Check for framebuffer errors
 */
static void CheckFBStatus() {
    GLenum status;

    glFlush();
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
        printf("[GLES2] GL_FRAMEBUFFER_COMPLETE\n");
        break;
    case 0x8CDB:
        printf("[GLES2] GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        printf("[GLES2] GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        printf("[GLES2] GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
        break;
        //case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:            printf("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        printf("[GLES2] GL_FRAMEBUFFER_UNSUPPORTED\n");
        break;
    default:
        printf("[GLES2] Unknown issue (%x).\n", status);
        break;
    }
}

static BOOL init(int w, int h) {
    glViewport(0, 0, w, h);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_ARRAY_BUFFER);

    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    CheckErrorsF("init", "no details");
    CheckFBStatus();

    return YES;
}

static BOOL registerObject(object_t * object) {
    renderer_prog_object_t * objData;

    if (!object) {
        printf("[GLES2] Object data was NULL nothing to upload\n");
        return NO;
    }

    if (object->renderer_data) {
        printf("[GLES2] Object %s seems already uploaded\n", object->name);
        return NO;
    }

    objData = malloc(sizeof(renderer_prog_object_t));
    if (!objData) {
        printf("[GLES2] Unable to allocate memory for object data\n");
        return NO;
    }

    object->renderer_data = objData;
    objData->memory_location = OBJECT_MEMLOC_VRAM;

    glGenBuffers(1, &objData->vboId);
    glBindBuffer(GL_ARRAY_BUFFER, objData->vboId);
    glBufferData(GL_ARRAY_BUFFER, object->num_verticies * sizeof(vertex_t), object->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CheckErrorsF("UploadObjectToGPU", object->name);

    return YES;
}

static void setup3d(camera_t * camera) {
    vec3_t vLookat;

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    Perspective(camera->fov, camera->aspect, camera->zNear, camera->zFar, projectionMatrix);
    glLoadMatrixf(projectionMatrix);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Setup the camera */
    vectorAdd(camera->position, camera->forward, vLookat);
    LookAt(camera->position, vLookat, camera->up, modelViewMatrix);
    glLoadMatrixf(modelViewMatrix);

    /* Setup Lights */
    glDisable(GL_LIGHTING);
    /*
     glLightfv(GL_LIGHT0, GL_POSITION, light.position);
     glLightfv(GL_LIGHT0, GL_AMBIENT, light.ambient);
     glLightfv(GL_LIGHT0, GL_DIFFUSE, light.diffuse);
     glLightfv(GL_LIGHT0, GL_SPECULAR, light.specula);

     glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, light.constantAttenuation);
     glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, light.linearAttenuation);
     */
}

static void render_render_matrix(world_object_instance_t * object) {
    float A, B, C, D, E, F, AD, BD;
    matrix_t mat;

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
    glLoadMatrixf(mat);

}

static void render(world_object_instance_t * object) {
    renderer_prog_object_t * objData;

    objData = object->object->renderer_data;

    glPushMatrix();
    render_render_matrix(object);

    if (objData->memory_location == OBJECT_MEMLOC_VRAM) {
        glBindBuffer(GL_ARRAY_BUFFER, objData->vboId);

        glNormalPointer(GL_SHORT, sizeof(vertex_t), (char *) (NULL + VERTEX_OFFSET_OF_NORMAL));
        glTexCoordPointer(2, GL_SHORT, sizeof(vertex_t), (char *) (NULL + VERTEX_OFFSET_OF_TEXTURECOORD));
        glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (char *) (NULL + VERTEX_OFFSET_OF_POSITION));
    } else {
        glNormalPointer(GL_SHORT, sizeof(vertex_t), object->object->vertices[0].normal);
        glTexCoordPointer(2, GL_SHORT, sizeof(vertex_t), object->object->vertices[0].textureCoord);
        glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), object->object->vertices[0].position);
    }

    glDrawElements(GL_TRIANGLES, object->object->num_indices, GL_UNSIGNED_SHORT, object->object->indices);

    glPopMatrix();
}

static void end3d() {

}

renderer_t * rendererInitProc() {
    renderer_t * renderer;
    renderer = malloc(sizeof(renderer_t));

    if (renderer) {
        renderer->name = "OpenGL Programmable Path";

        renderer->init = &init;
        renderer->register_object = &registerObject;

        renderer->start_3D = &setup3d;
        renderer->render_object_instance = &render;
        renderer->end_3D = &end3d;
    }

    return renderer;
}
