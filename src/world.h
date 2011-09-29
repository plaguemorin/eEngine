/*
 * world.h
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "3d_math.h"

BOOL WORLD_Init();
BOOL WORLD_Destroy();
world_object_instance_t * WORLD_AttachObjectToWorld(object_t *);


#endif /* WORLD_H_ */
