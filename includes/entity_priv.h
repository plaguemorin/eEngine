/*
 * entity_priv.h
 *
 *  Created on: 2011-10-17
 *      Author: plaguemorin
 */

#ifndef ENTITY_PRIV_H_
#define ENTITY_PRIV_H_

typedef struct ent_entity_loaded_t {
    char * name;

    scene_node_t * node_graph_copy;

    struct ent_entity_loaded_t * next;
} entity_loaded;

BOOL MD5_LoadMD5(filehandle_t * file, scene_node_t *);
BOOL OBJ_LoadOBJ(filehandle_t * file, scene_node_t *);
BOOL TDS_Load3DS(filehandle_t * file, scene_node_t *);

#endif /* ENTITY_PRIV_H_ */
