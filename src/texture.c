/*
 * texture.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "3d_math.h"

#include "texture.h"

extern texture_t * loadNativePNG(const char * name);

void TEX_Init() {

}

texture_t * TEX_LoadTexture(const char * path) {
	texture_t  * text;

	printf("[TEX] Loading %s\n", path);
	text = loadNativePNG(path);
	if (text) {
		printf("[TEX] Done\n");
	} else {
		printf("[TEX] Error\n");
	}

	return text;
}
