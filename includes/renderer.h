/*
 * renderer.h
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#ifndef RENDERER_H_
#define RENDERER_H_

BOOL RENDERER_Init(int w, int h);
BOOL RENDERER_Destroy();

void REN_Update(float);
void REN_HostFrame();
BOOL REN_MakeNodeAvailable(scene_node_t *);
BOOL REN_MakeObjectAvailable(mesh_t *);
BOOL REN_MakeTextureAvailable(texture_t *);

#endif /* RENDERER_H_ */
