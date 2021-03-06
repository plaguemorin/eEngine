/*
 * entity_md5_loader.c
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
#include "scene_node.h"

#include "entity_priv.h"

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
static void MD5_ReadMesh(scene_node_t *, unsigned int meshNumber, object_joint_t * joints);
static void MD5_ReadJoints(unsigned int num_joints, object_joint_t * joints);
static void MD5_GenerateSkin(mesh_t* mesh, object_joint_t* bones, md5_vertex_t * vertex, unsigned int num_weight, md5_weight_t * weight);

BOOL MD5_LoadMD5(filehandle_t * file, scene_node_t * entity) {
    int versionNumber = 0;
    object_joint_t * joints;
    unsigned int num_joints = 0;
    unsigned short meshNumber = 0;

    LE_pushLexer();
    LE_init(file);

    /* loop on all the tokens */
    while (LE_hasMoreData()) {
        LE_readToken();

        /* Make sure this is a version we support */
        if (!strcmp("MD5Version", LE_getCurrentToken())) {
            versionNumber = LE_readReal();
            if (versionNumber != 10) {
                printf("[ENT] : Bad model version (%d)\n", versionNumber);
                LE_popLexer();
                return FALSE;
            }

            continue;
        }

        /* Number of objects */
        if (!strcmp("numMeshes", LE_getCurrentToken())) {
            LE_readReal();
            continue;
        }

        /* Number of "bones" */
        if (!strcmp("numJoints", LE_getCurrentToken())) {
            num_joints = LE_readReal();
            joints = (object_joint_t *) malloc(num_joints * sizeof(object_joint_t));
            continue;
        }

        /* A Mesh was found */
        if (!strcmp("mesh", LE_getCurrentToken())) {
            MD5_ReadMesh(entity, meshNumber++, joints);
            continue;
        }

        /* Joints */
        if (!strcmp("joints", LE_getCurrentToken())) {
            MD5_ReadJoints(num_joints, joints);
        }
    }

    LE_popLexer();
    free(joints);

    return TRUE;
}

static void MD5_ReadMesh(scene_node_t * parent, unsigned int objNum, object_joint_t * joints) {
    mesh_t * mesh;
    md5_vertex_t * vertex;
    md5_weight_t * weight;
    scene_node_t * node;
    int j, numWeight, maxWeightPerVertex;

    maxWeightPerVertex = 0;
    node = SCENE_NewNodeWithParent(parent, NULL, NODE_TYPE_STATIC_MESH);
    mesh = node->object.mesh;
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
                mesh->indices[j + 0] = LE_readReal();
                mesh->indices[j + 1] = LE_readReal();
                mesh->indices[j + 2] = LE_readReal();
            }
            continue;
        }

        /* Read a vertex */
        if (!strcmp("vert", LE_getCurrentToken())) {
            j = LE_readReal(); /* ID */

            if (j < mesh->num_verticies) {
                mesh->vertices[j].textureCoord[0] = LE_readReal() * 32767.0f;
                mesh->vertices[j].textureCoord[1] = LE_readReal() * 32767.0f;

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
    MD5_GenerateSkin(mesh, joints, vertex, numWeight, weight);
    free(vertex);
    free(weight);
}

static void MD5_ReadJoints(unsigned int num_joints, object_joint_t * joints) {
    unsigned int i;
    char * token;
    object_joint_t * joint;

    LE_readToken(); /* { */

    for (i = 0; i <= num_joints - 1; i++) {
        joint = &joints[i];

        /* Read & Copy joint name */
        LE_readToken();
        token = LE_getCurrentToken();
        joint->name = malloc(sizeof(char) * (strlen(token) + 1));
        if (joint->name) {
            strcpy(joint->name, token);
        }

        joint->parent = LE_readReal();
        joint->parent_joint = (object_joint_t *) &joints[joint->parent];

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

static void MD5_GenerateSkin(mesh_t* mesh, object_joint_t* joints, md5_vertex_t * vertex, unsigned int num_weight, md5_weight_t * weights) {
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

        //printf("[DEBUG] Vertex %d = %03.5f  %03.5f  %03.5f\n", i, currentVertex->position[0], currentVertex->position[1], currentVertex->position[2]);
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
