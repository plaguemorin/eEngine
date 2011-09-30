/*
 * renderer.h
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include "3d_math.h"

BOOL RENDERER_Init(int w, int h);
BOOL RENDERER_Destroy();

void REN_Update(float);
void REN_HostFrame();
BOOL REN_MakeObjectAvailable(object_t *);
BOOL REN_MakeTextureAvailable(texture_t *);

#endif /* RENDERER_H_ */
