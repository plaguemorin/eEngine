/**
 *
 * Base engine file
 * 
 * engine.c 
 */
 
#include "engine.h"
#include "script.h"

engine_t engine;


engine_t * engine_init() {
	SCRIPTING_Init(&engine);
   
    return &engine;
}

int main(int a, char **b) {
	engine_init();
	
		
	return 0;
}
