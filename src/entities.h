/**
 * Entity 
 *
 */
#ifndef _ENTITIES_H__
#define _ENTITIES_H__
BOOL ENTITY_Init();
BOOL ENTITY_Update();
BOOL ENTITY_Destroy();

entity_t * ENTITY_NewDummyObject();
entity_t * ENTITY_LoadObject(const char * path);

#endif
