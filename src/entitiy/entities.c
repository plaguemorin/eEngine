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

#include "entity_priv.h"

static entity_loaded * loaded_entities;

/**
 * Allocate a new entitu with a name
 *
 * Can return null if allocation failed
 */
static entity_t * alloc_new_entity(const char * name) {
    entity_t * obj;
    entity_loaded * el;

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
        el->entity = obj;
    } else {
        free(obj->name);
        free(obj);
        obj = NULL;
    }

    return obj;
}

static entity_t * find_entity(const char * name) {
    entity_loaded * el;

    el = loaded_entities;
    while (el && el->entity && strcmp(name, el->entity->name) != 0) {
        el = el->next;
    }

    return (el) ? el->entity : NULL;
}

BOOL ENTITY_Init() {
    loaded_entities = NULL;

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

    entity = find_entity(path);
    if (!entity) {
        file = FS_OpenFileRead(path);
        if (!file) {
            printf("[ENT] Unable to open file %s\n", path);
            return NULL;
        }

        entity = alloc_new_entity(path);
        if (entity) {
            if (MD5_LoadMD5(file, entity) != TRUE) {
                free(entity->name);
                free(entity);

                entity = NULL;
            }
        }
    }

    return entity;
}

