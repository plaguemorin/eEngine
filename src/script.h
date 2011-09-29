/**
 * Scripting Helper
 */

#ifndef SCRIPTING_H__
#define SCRIPTING_H__

/* Include the Lua API header files. */
#include <lua.h>

BOOL SCRIPTING_Init();
BOOL SCRIPTING_Destory();
BOOL SCRIPTING_AfterLoaded();

#endif
