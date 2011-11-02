/**
 * Entity 
 *
 */
#ifndef _ENTITIES_H__
#define _ENTITIES_H__

BOOL ENTITY_Init();
BOOL ENTITY_Update();
BOOL ENTITY_Destroy();

BOOL ENTITY_LoadObject(const char * path, scene_node_t * top);

#endif
