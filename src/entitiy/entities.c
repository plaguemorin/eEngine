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

/*
 * Loads an entity
 */
scene_node_t * ENTITY_LoadObject(const char * path) {
    filehandle_t * file;
    scene_node_t * entity;
    BOOL outcome = FALSE;

    entity = find_entity(path);
    if (!entity) {
        file = FS_OpenFileRead(path);
        if (!file) {
            printf("[ENT] Unable to open file %s\n", path);
            return NULL;
        }

        /* TODO: Replace with command-style pattern */
        entity = alloc_new_entity(path);
        if (entity) {
            if (strcmp("md5mesh", file->fileExtention) == 0) {
                outcome = MD5_LoadMD5(file, entity);
            }

            if (strcmp("obj", file->fileExtention) == 0) {
                outcome = OBJ_LoadOBJ(file, entity);
            }

            if (strcmp("3ds", file->fileExtention) == 0) {
                outcome = TDS_Load3DS(file, entity);
            }

            if (outcome != TRUE) {
                SCENE_FreeNode(entity);
                entity = NULL;
            }
        }

        FS_Close(file);
    }

    return entity;
}

