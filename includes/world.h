/*
 * world.h
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#ifndef WORLD_H_
#define WORLD_H_


BOOL WORLD_Init();
void WORLD_Update(float delta);
BOOL WORLD_Destroy();
scene_node_t * WORLD_AttachObjectToWorld(scene_node_t * parent, entity_t *);
scene_node_t * WORLD_AddDummyNode(scene_node_t * parent);

#endif /* WORLD_H_ */
