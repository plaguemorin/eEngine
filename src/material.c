/*
 * material.c
 *
 *  Created on: 2011-10-04
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "engine.h"
#include "renderer.h"
#include "texture.h"

void MAT_Init() {

}

material_t * MAT_LoadMaterial(const char * name) {
	material_t * mat;
	printf("[MAT] Loading %s\n", name);

	mat = (material_t *) malloc(sizeof(material_t));
	if (!mat) {
		printf("[MAT] Failed to allocate memory for material\n");
		return NULL;
	}
	memset(mat, 0, sizeof(material_t));

	mat->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
	if (mat->name) {
		strcpy(mat->name, name);
	}

	mat->texture_diffuse = TEX_LoadTexture(name);

	if (!mat->texture_diffuse) {
		free(mat->name);
		free(mat);
		mat = NULL;
	}

	return mat;
}
