/**
 * Scripting Helper
 */

#ifndef SCRIPTING_H__
#define SCRIPTING_H__

/* Include the Lua API header files. */
#include <lua.h>

BOOL SCRIPTING_Init(engine_t *);
BOOL SCRIPTING_Destory(engine_t *);
BOOL SCRIPTING_AfterLoaded(engine_t *);

#endif
