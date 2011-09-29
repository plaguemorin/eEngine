/**
 * Engine Core Header
 * 
 * engine.h
 */
 
#ifndef ENGINE_H_
#define ENGINE_H_

#include "global.h"
#include "3d_math.h"

struct lua_State;
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
	
	void (*start_3D)(camera_t * camera);
	void (*render_object_instance)(world_object_instance_t *);
	void (*end_3D)();

	void (*start_2D)(int, int);
	void (*printString)(int, int, font_t *, const char *);
	void (*end_2D)();

	BOOL (*register_texture)(struct td_texture_t *);
	BOOL (*unregister_texture)(struct td_texture_t *);
	
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
	renderer_t				*		renderer;
	world_t					*		world;
	camera_t						camera;

	unsigned int					renderWidth;
	unsigned int					renderHeight;

	float							lastRenderTime;
	font_t                  *       defaultFont;
} engine_t;

extern engine_t * engine;

BOOL engine_init(int w, int h);

void engine_update();
void engine_drawframe();

#define malloc(x) debug_malloc(x, __FILE__, __LINE__)
void * debug_malloc(size_t s, const char * file, int line);

#endif
