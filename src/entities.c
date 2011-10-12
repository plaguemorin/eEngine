/**
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX
#	define MAX(a,b)		(((a) > (b)) ? (a) : (b))
#endif

#include "engine.h" 
#include "3d_math.h"
#include "filesystem.h"
#include "lexer.h"
#include "entities.h"
#include "material.h"

typedef struct md5_vertex_t {
    int start; /* start weight */
    int count; /* weight count */
} md5_vertex_t;

typedef struct md5_weight_t {
    int boneId;
    float bias;
    vec3_t boneSpacePos;
    vec3s_t boneSpaceNormal;
    vec3s_t boneSpaceTangent;

    vec3_t modelSpacePos;
    vec3_t modelSpaceNormal;
    vec3_t modelSpaceTangent;
} md5_weight_t;

static void cleanUpDoubleQuotes(char* string);
static object_t * alloc_new_object(const char * name);
static entity_t * alloc_new_entity(const char * name);
static void MD5_ReadMesh(entity_t * entity, unsigned int meshNumber);
static void MD5_ReadJoints(entity_t * entity);
static void MD5_GenerateSkin(object_t* mesh, object_joint_t* bones, md5_vertex_t * vertex, unsigned int num_weight, md5_weight_t * weight);

/**
 * Allocate a new object with a name
 *
 * Can return null if allocation failed
 */
static object_t * alloc_new_object(const char * name) {
    object_t * obj;

    obj = malloc(sizeof(object_t));
    if (obj) {
        memset(obj, 0, sizeof(object_t));

        if (name) {
            obj->name = malloc(sizeof(char) * (strlen(name)));

            if (obj->name) {
                strcpy(obj->name, name);
            }
        }
    }

    return obj;
}

/**
 * Allocate a new entitu with a name
 *
 * Can return null if allocation failed
 */
static entity_t * alloc_new_entity(const char * name) {
    entity_t * obj;

    obj = malloc(sizeof(entity_t));
    if (obj) {
        memset(obj, 0, sizeof(entity_t));

        if (name) {
            obj->name = malloc(sizeof(char) * (strlen(name)));

            if (obj->name) {
                strcpy(obj->name, name);
            }
        }
    }

    return obj;
}

BOOL ENTITY_Init() {

    return YES;
}

BOOL ENTITY_Update() {

    return YES;
}

BOOL ENTITY_Destroy() {

    return YES;
}

/*
 * Loads an entity
 */
entity_t * ENTITY_LoadObject(const char * path) {
    filehandle_t * file;
    entity_t * entity;
    int versionNumber = 0;
    unsigned short meshNumber = 0;

    file = FS_OpenFileRead(path);
    if (!file) {
        printf("[ENT] Unable to open file %s\n", path);
        return NULL;
    }

    LE_pushLexer();
    LE_init(file);

    entity = alloc_new_entity(path);
    if (!entity) {
        LE_popLexer();
        FS_Close(file);
        return NULL;
    }

    /* loop on all the tokens */
    while (LE_hasMoreData()) {
        LE_readToken();

        /* Make sure this is a version we support */
        if (!strcmp("MD5Version", LE_getCurrentToken())) {
            versionNumber = LE_readReal();
            if (versionNumber != 10) {
                printf("[ENT] : %s has a bad model version (%d)\n", path, versionNumber);
                FS_Close(file);
                return 0;
            }

            continue;
        }

        /* Number of objects */
        if (!strcmp("numMeshes", LE_getCurrentToken())) {
            entity->num_objects = LE_readReal();
            entity->objects = (object_t *) malloc(entity->num_objects * sizeof(object_t));
            continue;
        }

        /* Number of "bones" */
        if (!strcmp("numJoints", LE_getCurrentToken())) {
            entity->num_joints = LE_readReal();
            entity->joints = (object_joint_t *) malloc(entity->num_joints * sizeof(object_joint_t));
            continue;
        }

        /* A Mesh was found */
        if (!strcmp("mesh", LE_getCurrentToken())) {
            MD5_ReadMesh(entity, meshNumber++);
            continue;
        }

        /* Joints */
        if (!strcmp("joints", LE_getCurrentToken())) {
            MD5_ReadJoints(entity);
        }

    }

    return entity;
}

entity_t * ENTITY_NewDummyObject() {
    object_t * obj;
    entity_t * entity;
    int i;

#include "teapot.h"

    obj = alloc_new_object("dummy object");

    obj->num_verticies = (sizeof(vdata) / sizeof(vdata[0]) / 3) + 1;
    obj->vertices = malloc(1 + sizeof(vertex_t) * obj->num_verticies);
    for (i = 0; i < obj->num_verticies; i++) {
        vectorSet(obj->vertices[i].position, vdata[(i*3)], vdata[(i*3)+1], vdata[(i*3)+2]);
        vectorSet(obj->vertices[i].normal, vnormals[(i*3)], vnormals[(i*3)+1], vnormals[(i*3)+2]);
        obj->vertices[i].textureCoord[0] = vtext[(i * 2)];
        obj->vertices[i].textureCoord[1] = vtext[(i * 2) + 1];
    }

    obj->num_indices = (sizeof(tindices) / sizeof(tindices[0])) + 1;
    obj->indices = malloc(1 + sizeof(unsigned short) * obj->num_indices);
    for (i = 0; i <= obj->num_indices; i++) {
        obj->indices[i] = tindices[i];
    }

    entity = alloc_new_entity("dummy entity");
    entity->joints = NULL;
    entity->num_joints = 0;
    entity->num_objects = 1;
    entity->objects = obj;

    return entity;
}

static void MD5_ReadMesh(entity_t * entity, unsigned int objNum) {
    object_t * mesh;
    md5_vertex_t * vertex;
    md5_weight_t * weight;
    int j, numWeight, maxWeightPerVertex;

    maxWeightPerVertex = 0;
    mesh = &entity->objects[objNum];
    memset(mesh, 0, sizeof(object_t));
    LE_readToken(); /* { */

    while (strcmp("}", LE_getCurrentToken())) {
        LE_readToken();

        /* Send the shader name to the Material Sub-system */
        if (!strcmp("shader", LE_getCurrentToken())) {
            LE_readToken();
            cleanUpDoubleQuotes(LE_getCurrentToken());

            /* Load material in getCurrentToken */
            mesh->material = MAT_LoadMaterial(LE_getCurrentToken());
            continue;
        }

        /* When the token Num Vertices is found, allocate the memory needed for temporary and final vertices */
        if (!strcmp("numverts", LE_getCurrentToken())) {
            mesh->num_verticies = LE_readReal();
            vertex = (md5_vertex_t*) malloc(mesh->num_verticies * sizeof(md5_vertex_t));
            mesh->vertices = (vertex_t *) malloc(mesh->num_verticies * sizeof(vertex_t));
            if (!vertex || !mesh->vertices) {
                free(vertex);
                free(mesh->vertices);
                printf("[ENT] Unable to allocate memory for %d vertices\n", mesh->num_verticies);
                mesh->num_verticies = 0;
            }
            continue;
        }

        /* Allocate memory for the triangles when the token number of triangles is found */
        if (!strcmp("numtris", LE_getCurrentToken())) {
            mesh->num_indices = LE_readReal() * 3;
            mesh->indices = (unsigned short *) malloc(mesh->num_indices * sizeof(unsigned short));
            if (!mesh->indices) {
                printf("[ENT] Unable to allocate memory for %d indices\n", mesh->num_indices);
                mesh->num_indices = 0;
            }
            continue;
        }

        /* Allocate temporary weights for reading when the token number of weights is found */
        if (!strcmp("numweights", LE_getCurrentToken())) {
            numWeight = LE_readReal();
            weight = (md5_weight_t *) malloc(numWeight * sizeof(md5_weight_t));
            if (!weight) {
                printf("[ENT] Unable to allocate memory for %d weights\n", numWeight);
                numWeight = 0;
            }
            continue;
        }

        /* Read a triangle */
        if (!strcmp("tri", LE_getCurrentToken())) {
            j = LE_readReal() * 3; /* ID */

            if (j < mesh->num_indices) {
                mesh->indices[j] = LE_readReal();
                mesh->indices[j + 1] = LE_readReal();
                mesh->indices[j + 2] = LE_readReal();
            }
            continue;
        }

        /* Read a vertex */
        if (!strcmp("vert", LE_getCurrentToken())) {
            j = LE_readReal(); /* ID */

            if (j < mesh->num_verticies) {
                mesh->vertices[j].textureCoord[0] = LE_readReal();
                mesh->vertices[j].textureCoord[1] = LE_readReal();

                vertex[j].start = LE_readReal();
                vertex[j].count = LE_readReal();

                maxWeightPerVertex = MAX(maxWeightPerVertex, vertex[j].count);
            }
            continue;
        }

        /* Read a weight */
        if (!strcmp("weight", LE_getCurrentToken())) {
            j = LE_readReal(); /* ID */
            if (j < numWeight) {
                weight[j].boneId = LE_readReal();
                weight[j].bias = LE_readReal();
                weight[j].boneSpacePos[0] = LE_readReal();
                weight[j].boneSpacePos[1] = LE_readReal();
                weight[j].boneSpacePos[2] = LE_readReal();
            }
            continue;
        }
    }

    /* Convert the mesh to a known model */
    MD5_GenerateSkin(mesh, entity->joints, vertex, numWeight, weight);
    free(vertex);
    free(weight);
}

static void MD5_ReadJoints(entity_t * entity) {
    unsigned int i;
    char * token;
    object_joint_t * joint;

    LE_readToken(); /* { */

    for (i = 0; i <= entity->num_joints - 1; i++) {
        joint = &entity->joints[i];

        /* Read & Copy joint name */
        LE_readToken();
        token = LE_getCurrentToken();
        joint->name = malloc(sizeof(char) * (strlen(token) + 1));
        if (joint->name) {
            strcpy(joint->name, token);
        }

        joint->parent = LE_readReal();
        joint->parent_joint = (object_t *) &entity->joints[joint->parent];

        joint->position[0] = LE_readReal();
        joint->position[1] = LE_readReal();
        joint->position[2] = LE_readReal();

        joint->orientation[0] = LE_readReal();
        joint->orientation[1] = LE_readReal();
        joint->orientation[2] = LE_readReal();
        Quat_computeW(joint->orientation);
    }

    LE_readToken(); /* } */
}

static void MD5_GenerateSkin(object_t* mesh, object_joint_t* joints, md5_vertex_t * vertex, unsigned int num_weight, md5_weight_t * weights) {
    md5_weight_t * w;
    object_joint_t * bone;
    vertex_t* currentVertex;
    vec3_t normalAccumulator;
    vec3_t tangentAccumulator;
    int i, j;

    w = weights;
    for (i = 0; i < num_weight; i++, w++) {
        bone = &joints[w->boneId];
        Quat_rotate_point(bone->orientation, w->boneSpacePos, w->modelSpacePos);
        vectorAdd(w->modelSpacePos, bone->position, w->modelSpacePos);

        Quat_rotate_short_point(bone->orientation, w->boneSpaceNormal, w->modelSpaceNormal);
        Quat_rotate_short_point(bone->orientation, w->boneSpaceTangent, w->modelSpaceTangent);
    }

    currentVertex = mesh->vertices;
    for (i = 0; i < mesh->num_verticies; ++i) {
        vectorClear(currentVertex->position);
        vectorClear(normalAccumulator);
        vectorClear(tangentAccumulator);

        /* Compute the position based on the weights */
        for (j = 0; j < vertex[i].count; j++) {
            w = &weights[vertex[i].start + j];

            /* Calculate transformed vertex for this weight */
            currentVertex->position[0] += w->modelSpacePos[0] * w->bias;
            currentVertex->position[1] += w->modelSpacePos[1] * w->bias;
            currentVertex->position[2] += w->modelSpacePos[2] * w->bias;

            /* Copy weight and bone info */
            if (j < MAX_JOINT_PER_VERTEX) {
                currentVertex->weights[j] = w->bias;
                currentVertex->boneId[j] = w->boneId;
            }

            /* Same thing for normal */
            vectorAdd(normalAccumulator, w->modelSpaceNormal, normalAccumulator);
            vectorAdd(tangentAccumulator, w->modelSpaceTangent, tangentAccumulator);
        }

        /* Normalise and copy the normal and tangent */
        normalize(normalAccumulator);
        vectorCopy(normalAccumulator, currentVertex->normal);
        normalize(tangentAccumulator);
        vectorCopy(tangentAccumulator, currentVertex->tangent);

        currentVertex++;
    }
}

static void cleanUpDoubleQuotes(char* string) {
    char* cursor;
    int i;
    cursor = string;
    for (i = 0; i < strlen(string); i++) {
        if (string[i] != '"') *cursor++ = string[i];
    }
    *cursor = '\0';
}

