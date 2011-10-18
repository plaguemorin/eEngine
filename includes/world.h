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
world_object_instance_t * WORLD_AttachObjectToWorld(entity_t *);

#endif /* WORLD_H_ */
