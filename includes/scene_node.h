/*
 * scene_node.h
 *
 *  Created on: 2011-10-27
 *      Author: plaguemorin
 */

#ifndef SCENE_NODE_H_
#define SCENE_NODE_H_

scene_node_t * SCENE_NewNode(const char * name, scene_node_type_t);
scene_node_t * SCENE_NewNodeWithParent(scene_node_t *, const char * name, scene_node_type_t);
void SCENE_InitializeNode(scene_node_t * new_scene_node, const char * name, scene_node_type_t type);
scene_node_t * SCENE_DeepCopy(scene_node_t *);

void SCENE_AttachNodeAsChildOf(scene_node_t * parent, scene_node_t * child);
void SCENE_FreeNode(scene_node_t *);

void SCENE_DumpNode(scene_node_t * node, unsigned int level);

#endif /* SCENE_NODE_H_ */
