/*
 * entity_obj_loader.c
 *
 *  Created on: 2011-10-17
 *      Author: plaguemorin
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX
#   define MAX(a,b)     (((a) > (b)) ? (a) : (b))
#endif

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "filesystem.h"
#include "lexer.h"
#include "entities.h"
#include "material.h"

typedef struct obj_vertex_t {
    vec3_t vec;

    struct obj_vertex_t * next;
} obj_vertex_t;

typedef struct obj_texcoord_t {
    vec2_t texcoord;

    struct obj_texcoord_t * next;
} obj_texcoord_t;

typedef struct obj_face_t {
    unsigned int v1, v2, v3;
    unsigned int t1, t2, t3;

    struct obj_face_t * next;
} obj_face_t;

typedef struct obj_mesh_t {
    char * name;

    obj_vertex_t * vertex;
    obj_texcoord_t * texcoord;
    obj_face_t * faces;

    struct obj_mesh_t * next;
} obj_mesh_t;

static void OBJ_ReadVertex(obj_mesh_t * mesh) {
    obj_vertex_t * vertex;

    if (mesh->vertex) {
        vertex = mesh->vertex;
        while (vertex->next != NULL) {
            vertex = vertex->next;
        }
        vertex->next = (obj_vertex_t *) malloc(sizeof(obj_vertex_t));
        vertex = vertex->next;
    } else {
        mesh->vertex = vertex = (obj_vertex_t *) malloc(sizeof(obj_vertex_t));
    }

    vertex->vec[0] = LE_readReal();
    vertex->vec[1] = LE_readReal();
    vertex->vec[2] = LE_readReal();
    vertex->next = NULL;

    /* May have x, y, z, w - w is optional and we don't need it */
    LE_SkipRestOfLine();
}

static void OBJ_ReadTexCoord(obj_mesh_t * mesh) {
    obj_texcoord_t * texcoord;

    if (mesh->texcoord) {
        texcoord = mesh->texcoord;
        while (texcoord->next != NULL) {
            texcoord = texcoord->next;
        }
        texcoord->next = (obj_texcoord_t *) malloc(sizeof(obj_texcoord_t));
        texcoord = texcoord->next;
    } else {
        mesh->vertex = texcoord = (obj_texcoord_t *) malloc(sizeof(obj_texcoord_t));
    }

    texcoord->texcoord[0] = LE_readReal();
    texcoord->texcoord[1] = 1.0f - LE_readReal();
    texcoord->next = NULL;

    /* May have u,v,w - w is optional and we don't need it */
    LE_SkipRestOfLine();
}

static void OBJ_ReadFace(obj_mesh_t * mesh) {
    unsigned int vertexIndex;
    unsigned int textureCoordIndex;
    unsigned char i;
    char * token;
    char * nextSubToken;

    for (i = 0; i < 3; i++) {
        token = LE_readToken();
        vertexIndex = 0;
        textureCoordIndex = 0;

        vertexIndex = strtoul(token, &nextSubToken, 0);
        if (nextSubToken != NULL) {
            strtoul(nextSubToken, &nextSubToken, 0);
            if (nextSubToken) {
                textureCoordIndex = strtoul(nextSubToken, &nextSubToken, 0);
            }
        }

    }

    LE_SkipRestOfLine();
}

BOOL OBJ_LoadOBJ(filehandle_t * file, entity_t * entity) {
    obj_mesh_t * mesh;
    unsigned int num_vertex, num_texcoord, num_faces;

    LE_pushLexer();
    LE_init(file);

    mesh = NULL;

    while (LE_hasMoreData()) {
        LE_readToken();
        if (!strcmp("g", LE_getCurrentToken())) {
            /* A wild mesh appears ! */
            if (mesh) {
                mesh->next = (obj_mesh_t *) malloc(sizeof(obj_mesh_t));
                mesh = mesh->next;
            } else {
                mesh = (obj_mesh_t *) malloc(sizeof(obj_mesh_t));
            }

            memset(mesh, 0, sizeof(obj_mesh_t));
            /* Should set the name but maybe not included */
        }

        /* All under this required a valid mesh object */
        if (!mesh) {
            break;
        }

        /* Vertex */
        if (!strcmp("v", LE_getCurrentToken())) {
            OBJ_ReadVertex(mesh);
            num_vertex++;
        }

        /* Texture Coords */
        if (!strcmp("vt", LE_getCurrentToken())) {
            OBJ_ReadTexCoord(mesh);
            num_texcoord++;
        }

        /* Don't care about normals */
        if (!strcmp("vn", LE_getCurrentToken())) {
            LE_readReal();
            LE_readReal();
            LE_readReal();
        }

        /* Face */
        if (!strcmp("f", LE_getCurrentToken())) {
            OBJ_ReadFace(mesh);
            num_faces++;
        }

    }

    LE_popLexer();
    return (mesh == NULL) ? NO : YES;
}
