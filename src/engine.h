/**
 * Engine Core Header
 * 
 * engine.h
 */
 
#ifndef ENGINE_H_
#define ENGINE_H_

#include "global.h"

struct lua_State;
struct renderer_t;
union td_properties_t;
struct td_texture_t;
struct td_material_t;
struct td_object_t;
struct td_light_t;

struct world_t;

/**
 * Renderer
 */
typedef struct all_renderer_t {
	/* Name of the rendering engine (Fixed, GLES, Software, etc) */
	char * name;
	
	/* Renderer properties wanted */
	union td_properties_t	props;
	
	BOOL (*init)(int windowWidth, int windowHeight);
	
	BOOL (*register_texture)(struct td_texture_t *);
	BOOL (*unregister_texture(struct td_texture_t *);
	
	BOOL (*register_material)(struct td_material_t *);
	BOOL (*unregister_material)(struct td_material_t *);
	
	BOOL (*register_object)(struct td_object_t *);
	BOOL (*unregister_object)(struct td_object_t *);
	
	BOOL (*register_light)(struct td_light_t *);
	BOOL (*unregister_light)(struct td_light_t *);
} renderer_t;

/**
 * All the engine's structure
 */
typedef struct all_engine_t {
	struct lua_State		* 		lua;
	struct renderer_t		*		renderer;
	struct world_t			*		world;
} engine_t;

#endif
