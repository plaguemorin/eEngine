/*
 * entity_priv.h
 *
 *  Created on: 2011-10-17
 *      Author: plaguemorin
 */

#ifndef ENTITY_PRIV_H_
#define ENTITY_PRIV_H_

typedef struct ent_entity_loaded_t {
    entity_t * entity;

    struct ent_entity_loaded_t * next;
} entity_loaded;

BOOL MD5_LoadMD5(filehandle_t * file, entity_t *);
BOOL OBJ_LoadOBJ(filehandle_t * file, entity_t *);

#endif /* ENTITY_PRIV_H_ */
