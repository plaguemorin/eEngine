/*
 * renderer_gl.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(__APPLE__)
#	include <OpenGLES/ES1/gl.h>
#	include <OpenGLES/ES1/glext.h>
#else
#	include <GL/gl.h>
#	include <GL/glext.h>
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#endif

#include "global.h"
#include "3d_math.h"
#include "engine.h"

#define DEG_TO_RAD (2.0f*3.14159265f/360.0f)

static matrix_t projectionMatrix;
static matrix_t modelViewMatrix;

static unsigned int last_texture;
static unsigned int last_vbo;

typedef enum prog_location {
    OBJECT_MEMLOC_VRAM, OBJECT_MEMLOC_RAM,
} prog_location;

typedef struct renderer_prog_object_t {
    GLuint vertexVboId;
    GLuint indiciesVboId;
    prog_location memory_location;
} renderer_prog_object_t;

typedef struct renderer_prog_texture_t {
    GLuint texId;
} renderer_prog_texture_t;

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
    /*
     glEnable(GL_CULL_FACE);
     glCullFace(GL_FRONT);
     */

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_ARRAY_BUFFER);

    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    CheckErrorsF("init", "no details");
    CheckFBStatus();

    last_vbo = last_texture = 0;

    return YES;
}

static BOOL registerObject(mesh_t * object) {
    renderer_prog_object_t * objData;

    if (!object) {
        printf("[GLES2] Object data was NULL nothing to upload\n");
        return NO;
    }

    if (object->renderer_data) {
        return NO;
    }

    objData = malloc(sizeof(renderer_prog_object_t));
    if (!objData) {
        printf("[GLES2] Unable to allocate memory for object data\n");
        return NO;
    }

    object->renderer_data = objData;
    objData->memory_location = OBJECT_MEMLOC_VRAM;

    glGenBuffers(1, &objData->vertexVboId);
    glGenBuffers(1, &objData->indiciesVboId);

    glBindBuffer(GL_ARRAY_BUFFER, objData->vertexVboId);
    glBufferData(GL_ARRAY_BUFFER, object->num_verticies * sizeof(vertex_t), object->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objData->indiciesVboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, object->num_indices * sizeof(unsigned short), object->indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CheckErrorsF("UploadObjectToGPU", "");

    return YES;
}

static BOOL registerTexture(texture_t * tex) {
    renderer_prog_texture_t * texData;

    if (!tex) {
        printf("[GLES2] Texture was null\n");
        return NO;
    }

    if (tex->renderer_data) {
        printf("[GLES2] Texture is already registered\n");
        return NO;
    }

    texData = malloc(sizeof(renderer_prog_texture_t));
    if (!texData) {
        printf("[GLES2] Unable to allocate memory for texture data\n");
        return NO;
    }

    tex->renderer_data = texData;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texData->texId);
    glBindTexture(GL_TEXTURE_2D, texData->texId);

    switch (tex->type) {
        case TEXTURE_TYPE_RGB:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex->data);
            break;

        case TEXTURE_TYPE_RGBA:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
            break;

        case TEXTURE_TYPE_BGR:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_BGR, GL_UNSIGNED_BYTE, tex->data);
            break;

        case TEXTURE_TYPE_BGRA:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex->data);
            break;

        default:
            printf("[GLES2] Unknown texture format \n");
            free(tex->data);
            return NO;
            break;
    }

    /* Using mipMapping to reduce bandwidth consumption */
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return YES;
}

static void useTexture(texture_t * tex) {
    renderer_prog_texture_t * texData;
    if (tex) {
        texData = (renderer_prog_texture_t *) tex->renderer_data;

        if (texData) {
            if (last_texture != texData->texId) {
                glBindTexture(GL_TEXTURE_2D, texData->texId);
                last_texture = texData->texId;
            }
        }
    }

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

    glColor4f(1, 1, 1, 1);
}

static void use_material(material_t * mat) {
    if (mat->texture_diffuse) {
        useTexture(mat->texture_diffuse);
    }
}

static void render(mesh_t * object, matrix_t mat) {
    renderer_prog_object_t * objData;

    objData = object->renderer_data;

    if (object->material) {
        use_material(object->material);
    }

    glPushMatrix();
    glLoadMatrixf(mat);

    if (objData->memory_location == OBJECT_MEMLOC_VRAM) {
        if (last_vbo != objData->vertexVboId) {
            glBindBuffer(GL_ARRAY_BUFFER, objData->vertexVboId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objData->indiciesVboId);
            last_vbo = objData->vertexVboId;

            glNormalPointer(GL_FLOAT, sizeof(vertex_t), (char *) (NULL + VERTEX_OFFSET_OF_NORMAL));
            glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (char *) (NULL + VERTEX_OFFSET_OF_TEXTURECOORD));
            glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (char *) (NULL + VERTEX_OFFSET_OF_POSITION));

            glDrawElements(GL_TRIANGLES, object->num_indices, GL_UNSIGNED_SHORT, 0);
        }
    } else {
        /*
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glNormalPointer(GL_FLOAT, sizeof(vertex_t), object->vertices[0].normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), object->vertices[0].textureCoord);
        glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), object->vertices[0].position);

        glDrawElements(GL_TRIANGLES, object->num_indices, GL_UNSIGNED_SHORT, object->indices);
        */
    }

    glPopMatrix();
}

static void end3d() {
    last_vbo = last_texture = 0;
}

static void start2D(int renderWidth, int renderHeight) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

}

static void end2D() {

}

static void printString(int x, int y, font_t * font, const char * txt) {
    char * p = (char *) txt;
    float cx, cy;

    unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };

    useTexture(font->texture);

    glPushMatrix();
    glTranslatef(0, y, 0);
    glScalef(font->size, font->size, 0);

    while (*p) {
        cx = (float) (*p % 16) / 16.0f;
        cy = (float) (*p / 16) / 16.0f;

        unsigned short vertices[] = { 0 + x, 0, 16 + x, 0, 16 + x, 16, 0 + x, 16 };
        float texcoord[] = { cx, 1 - cy - 0.0625f, cx + 0.0625f, 1 - cy - 0.0625f, cx + 0.0625f, 1 - cy, cx, 1 - cy };

        glVertexPointer(2, GL_UNSIGNED_SHORT, 0, vertices);
        glTexCoordPointer(2, GL_FLOAT, 0, texcoord);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        /*
         glBegin(GL_TRIANGLES);
         for (i = 0; i < 6; i++) {
         j = indices[i] * 2;
         glTexCoord2f(texcoord[j], texcoord[j + 1]);
         glVertex2i(vertices[j], vertices[j + 1]);
         }
         glEnd();
         */

        x = x + font->nCharWidth[(int) *p];
        p++;
    }

    glPopMatrix();
    glFlush();

}

renderer_t * rendererInitFixed() {
    renderer_t * renderer;
    renderer = malloc(sizeof(renderer_t));

    if (renderer) {
        renderer->name = "OpenGL Fixed Path";

        renderer->init = &init;
        renderer->shutdown = NULL;

        renderer->register_object = &registerObject;
        renderer->register_texture = &registerTexture;

        renderer->start_3D = &setup3d;
        renderer->render_object = &render;
        renderer->end_3D = &end3d;

        renderer->start_2D = &start2D;
        renderer->printString = &printString;
        renderer->end_2D = &end2D;
    }

    return renderer;
}
