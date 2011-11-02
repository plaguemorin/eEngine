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

#include "global.h"
#include "3d_math.h"
#include "engine.h" 
#include "filesystem.h"
#include "lexer.h"
#include "entities.h"
#include "material.h"
#include "scene_node.h"

#include "entity_priv.h"

static entity_loaded * loaded_entities;

/**
 * Allocate a new entity with a name
 *
 * Can return null if allocation failed
 */
static scene_node_t * alloc_new_entity(const char * name) {
    scene_node_t * obj;
    entity_loaded * el;

    obj = SCENE_NewNode((char *) name, NODE_TYPE_DUMMY);

    if (loaded_entities == NULL) {
        el = loaded_entities = (entity_loaded *) malloc(sizeof(entity_loaded));
    } else {
        el = loaded_entities;
        while (el->next != NULL) {
            el = el->next;
        }
        el->next = (entity_loaded *) malloc(sizeof(entity_loaded));
        el = el->next;
    }

    if (el) {
        memset(el, 0, sizeof(entity_loaded));
        el->node_graph_copy = obj;
    } else {
        SCENE_FreeNode(obj);
        obj = NULL;
        /* CRASH */
    }

    return obj;
}

static scene_node_t * find_entity(const char * name) {
    entity_loaded * el;

    el = loaded_entities;
    while (el && el->name && strcmp(name, el->name) != 0) {
        el = el->next;
    }

    return (el) ? SCENE_DeepCopy(el->node_graph_copy) : NULL;
}

BOOL ENTITY_Init() {
    loaded_entities = NULL;

    return YES;
}

BOOL ENTITY_Update() {

    return YES;
}

BOOL ENTITY_Destroy() {
    entity_loaded * el;
    entity_loaded * elNext;

    el = loaded_entities;
    while (el) {
        elNext = el->next;
        SCENE_FreeNode(el->node_graph_copy);
        free(el->name);
        free(el);
        el = elNext;
    }

    return YES;
}

static void ENTITY_ComputeMinMaxMesh(mesh_t * mesh) {
    unsigned int i;

    vectorSet(mesh->bounding_box.max, 0, 0, 0);
    vectorSet(mesh->bounding_box.min, 0, 0, 0);

    for (i = 0; i < mesh->num_verticies; i++) {
        mesh->bounding_box.min[0] = MIN(mesh->bounding_box.min[0], mesh->vertices[i].position[0]);
        mesh->bounding_box.min[1] = MIN(mesh->bounding_box.min[1], mesh->vertices[i].position[1]);
        mesh->bounding_box.min[2] = MIN(mesh->bounding_box.min[2], mesh->vertices[i].position[2]);

        mesh->bounding_box.max[0] = MAX(mesh->bounding_box.max[0], mesh->vertices[i].position[0]);
        mesh->bounding_box.max[1] = MAX(mesh->bounding_box.max[1], mesh->vertices[i].position[1]);
        mesh->bounding_box.max[2] = MAX(mesh->bounding_box.max[2], mesh->vertices[i].position[2]);
    }
}

static void ENTITY_ComputeMinMaxNode(scene_node_t * node) {
    unsigned int i;

    /* Figure out the min/max if it's a mesh */
    if (node->type == NODE_TYPE_STATIC_MESH) {
        ENTITY_ComputeMinMaxMesh(node->object.mesh);

        node->bounding_box.min[0] = node->object.mesh->bounding_box.min[0];
        node->bounding_box.min[1] = node->object.mesh->bounding_box.min[1];
        node->bounding_box.min[2] = node->object.mesh->bounding_box.min[2];

        node->bounding_box.max[0] = node->object.mesh->bounding_box.max[0];
        node->bounding_box.max[1] = node->object.mesh->bounding_box.max[1];
        node->bounding_box.max[2] = node->object.mesh->bounding_box.max[2];
    } else {
        /* Reset self */
        vectorSet(node->bounding_box.min, 0, 0, 0);
        vectorSet(node->bounding_box.max, 0, 0, 0);
    }

    /* Compute child first */
    for (i = 0; i < node->num_children; i++) {
        ENTITY_ComputeMinMaxNode(node->child[i]);

        node->bounding_box.min[0] = MIN(node->bounding_box.min[0], node->child[i]->bounding_box.min[0]);
        node->bounding_box.min[1] = MIN(node->bounding_box.min[1], node->child[i]->bounding_box.min[1]);
        node->bounding_box.min[2] = MIN(node->bounding_box.min[2], node->child[i]->bounding_box.min[2]);

        node->bounding_box.max[0] = MAX(node->bounding_box.max[0], node->child[i]->bounding_box.max[0]);
        node->bounding_box.max[1] = MAX(node->bounding_box.max[1], node->child[i]->bounding_box.max[1]);
        node->bounding_box.max[2] = MAX(node->bounding_box.max[2], node->child[i]->bounding_box.max[2]);
    }
}

/*
 * Loads an entity
 */
BOOL ENTITY_LoadObject(const char * path, scene_node_t * entity) {
    filehandle_t * file;
    BOOL outcome;

    outcome = FALSE;

    /* entity = find_entity(path);
     if (!entity) { */
    file = FS_OpenFileRead(path);
    if (!file) {
        printf("[ENT] Unable to open file %s\n", path);
        return NO;
    }

    /* TODO: Replace with command-style pattern */
    if (strcmp("md5mesh", file->fileExtention) == 0) {
        outcome = MD5_LoadMD5(file, entity);
    }

    if (strcmp("obj", file->fileExtention) == 0) {
        outcome = OBJ_LoadOBJ(file, entity);
    }

    if (strcmp("3ds", file->fileExtention) == 0) {
        outcome = TDS_Load3DS(file, entity);
    }

    if (outcome == TRUE) {
        ENTITY_ComputeMinMaxNode(entity);
        printf("[ENT] '%s' Min: %.2f %.2f %.2f Max: %.2f %.2f %.2f\n", entity->name, entity->bounding_box.min[0], entity->bounding_box.min[1], entity->bounding_box.min[2],
                entity->bounding_box.max[0], entity->bounding_box.max[1], entity->bounding_box.max[2]);
    } else {
        printf("[ENT] Failed\n");
//        SCENE_FreeNode(entity);
        //entity = NULL;
    }

    FS_Close(file);
    /* } */

    return outcome;
}

