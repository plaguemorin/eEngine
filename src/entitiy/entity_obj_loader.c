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
    unsigned int v[3];
    unsigned int t[3];

    struct obj_face_t * next;
} obj_face_t;

typedef struct obj_mesh_t {
    char * name;

    obj_vertex_t * vertex;
    obj_texcoord_t * texcoord;
    obj_face_t * faces;

    unsigned int num_vertex, num_texcoord, num_faces;

    struct obj_mesh_t * next;
} obj_mesh_t;

static void OBJ_ReadMesh(obj_mesh_t *mesh) {

}

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

    mesh->num_vertex++;
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
        mesh->texcoord = texcoord = (obj_texcoord_t *) malloc(sizeof(obj_texcoord_t));
    }

    texcoord->texcoord[0] = LE_readReal();
    texcoord->texcoord[1] = 1.0f - LE_readReal();
    texcoord->next = NULL;

    mesh->num_texcoord++;
}

static void OBJ_ReadFace(obj_mesh_t * mesh) {
    unsigned int vertexIndex;
    unsigned int textureCoordIndex;
    unsigned char i;
    char * token;
    char * nextSubToken;
    obj_face_t * face;

    if (mesh->faces) {
        face = mesh->faces;
        while (face->next != NULL) {
            face = face->next;
        }
        face->next = (obj_face_t *) malloc(sizeof(obj_face_t));
        face = face->next;
    } else {
        mesh->faces = face = (obj_face_t *) malloc(sizeof(obj_face_t));
    }

    for (i = 0; i < 3; i++) {
        token = LE_readToken();
        vertexIndex = 0;
        textureCoordIndex = 0;

        /* The vertex is always present */
        vertexIndex = strtoul(token, &nextSubToken, 0);

        /* Then we may have a vertex normal pointer */
        if (nextSubToken != NULL && *nextSubToken != 0) {
            textureCoordIndex = strtoul(nextSubToken + 1, &nextSubToken, 0);

            /* And texture coordinate entry */
            if (nextSubToken != NULL && *nextSubToken != 0) {
                (void) strtoul(nextSubToken + 1, &nextSubToken, 0);
            }
        }

        face->v[i] = vertexIndex;
        face->t[i] = textureCoordIndex;
    }

    face->next = NULL;
    mesh->num_faces++;
}

static obj_texcoord_t * OBJ_FindTextureIndex(obj_mesh_t * mesh, unsigned int index) {
    unsigned int i;
    obj_texcoord_t * t;

    i = 0;
    t = mesh->texcoord;
    while (t && i != index) {
        t = t->next;
        i++;
    }

    return t;
}

static void OBJ_Convert(obj_mesh_t * mesh, object_t * object) {
    unsigned int i;
    obj_vertex_t * vertex;
    obj_texcoord_t * tex;
    obj_face_t * face;

    object->num_verticies = mesh->num_vertex;
    object->num_indices = mesh->num_faces * 3;

    object->vertices = (vertex_t *) malloc(object->num_verticies * sizeof(vertex_t));
    object->indices = (unsigned short *) malloc(object->num_indices * sizeof(unsigned short));

    /* Convert vertices */
    vertex = mesh->vertex;
    for (i = 0; i < object->num_verticies; i++, vertex = vertex->next) {
        object->vertices[i].position[0] = vertex->vec[0];
        object->vertices[i].position[1] = vertex->vec[1];
        object->vertices[i].position[2] = vertex->vec[2];
    }

    /* Convert faces */
    face = mesh->faces;
    for (i = 0; i < object->num_indices; i += 3, face = face->next) {
        /* Set texture coord */
        tex = OBJ_FindTextureIndex(mesh, face->t[0] - 1);
        object->vertices[face->v[0] - 1].textureCoord[0] = tex->texcoord[0];
        object->vertices[face->v[0] - 1].textureCoord[1] = tex->texcoord[1];

        tex = OBJ_FindTextureIndex(mesh, face->t[1] - 1);
        object->vertices[face->v[1] - 1].textureCoord[0] = tex->texcoord[0];
        object->vertices[face->v[1] - 1].textureCoord[1] = tex->texcoord[1];

        tex = OBJ_FindTextureIndex(mesh, face->t[2] - 1);
        object->vertices[face->v[2] - 1].textureCoord[0] = tex->texcoord[0];
        object->vertices[face->v[2] - 1].textureCoord[1] = tex->texcoord[1];

        /* Copy indices */
        object->indices[i + 0] = face->t[0] - 1;
        object->indices[i + 1] = face->t[1] - 1;
        object->indices[i + 2] = face->t[2] - 1;
    }

}

static obj_mesh_t * OBJ_FreeAndReturnNext(obj_mesh_t * mesh) {
    obj_mesh_t * next;
    obj_vertex_t * vertex;
    obj_vertex_t * vertexNext;
    obj_face_t * face;
    obj_face_t * faceNext;
    obj_texcoord_t * texcoord;
    obj_texcoord_t * texcoordNext;

    /* Free */
    next = mesh->next;
    face = mesh->faces;
    vertex = mesh->vertex;
    texcoord = mesh->texcoord;

    while (vertex) {
        vertexNext = vertex->next;
        free(vertex);
        vertex = vertexNext;
    }

    while (face) {
        faceNext = face->next;
        free(face);
        face = faceNext;
    }

    while (texcoord) {
        texcoordNext = texcoord->next;
        free(texcoord);
        texcoord = texcoordNext;
    }

    free(mesh);
    return next;
}

BOOL OBJ_LoadOBJ(filehandle_t * file, entity_t * entity) {
    obj_mesh_t * mesh;
    obj_mesh_t * start;
    unsigned int num_mesh;
    unsigned int i;

    LE_pushLexer();
    LE_init(file);

    start = mesh = NULL;
    num_mesh = 0;

    while (LE_hasMoreData()) {
        LE_readToken();

        /* A Mesh */
        if (!strcmp("g", LE_getCurrentToken())) {
            /* A wild mesh appears ! */
            if (mesh) {
                mesh->next = (obj_mesh_t *) malloc(sizeof(obj_mesh_t));
                mesh = mesh->next;
            } else {
                start = mesh = (obj_mesh_t *) malloc(sizeof(obj_mesh_t));
            }

            memset(mesh, 0, sizeof(obj_mesh_t));
            /* Should set the name but maybe not included */

            OBJ_ReadMesh(mesh);
            num_mesh++;
        }

        /* All under this required a valid mesh object */
        if (!mesh) {
            continue;
        }

        /* Vertex */
        if (!strcmp("v", LE_getCurrentToken())) {
            OBJ_ReadVertex(mesh);
        }

        /* Texture Coords */
        if (!strcmp("vt", LE_getCurrentToken())) {
            OBJ_ReadTexCoord(mesh);
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
        }

        LE_SkipRestOfLine();
    }

    /* Convert num_mesh from OBJ format to internal format */
    entity->num_objects = num_mesh;
    entity->objects = (object_t *) malloc(entity->num_objects * sizeof(object_t));
    memset(entity->objects, 0, entity->num_objects * sizeof(object_t));

    mesh = start;
    for (i = 0; i < num_mesh; i++) {
        OBJ_Convert(mesh, &entity->objects[i]);
        mesh = OBJ_FreeAndReturnNext(mesh);
    }

    LE_popLexer();
    return (mesh == NULL) ? NO : YES;
}
