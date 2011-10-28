/*
 * scene_node.h
 *
 *  Created on: 2011-10-27
 *      Author: plaguemorin
 */

#ifndef SCENE_NODE_H_
#define SCENE_NODE_H_

scene_node_t * SCENE_NewNode(char * name, scene_node_type_t);
scene_node_t * SCENE_NewNodeWithParent(scene_node_t *, char * name, scene_node_type_t);
scene_node_t * SCENE_DeepCopy(scene_node_t *);

void SCENE_AttachNodeAsChildOf(scene_node_t * parent, scene_node_t * child);
void SCENE_FreeNode(scene_node_t *);

#endif /* SCENE_NODE_H_ */
