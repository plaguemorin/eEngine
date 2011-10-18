/*
 * font.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "renderer.h"
#include "font.h"
#include "texture.h"

BOOL FONT_Init() {

    return FONT_LoadDefaultFont("data/fonts/DEFAULT.png");
}

BOOL FONT_LoadDefaultFont(const char * name) {
    texture_t * tex;

    engine->defaultFont = malloc(sizeof(font_t));

    tex = TEX_LoadTexture(name);
    if (tex) {
        if (REN_MakeTextureAvailable(tex) == YES) {
            engine->defaultFont->texture = tex;
            engine->defaultFont->size = 2;
            engine->defaultFont->hFrac = (float) (engine->defaultFont->nMaxHeight / (float) tex->height);
            engine->defaultFont->wFrac = (float) (engine->defaultFont->nMaxWidth / (float) tex->width);

            memset(engine->defaultFont->nCharWidth, 0xD, sizeof(engine->defaultFont->nCharWidth) / sizeof(engine->defaultFont->nCharWidth[0]));

            printf("[FNT] Default font was loaded\n");
        } else {
            printf("[FNT] Unable to make texture avail\n");
            return NO;
        }
    } else {
        printf("[FNT] Unable to load texture\n");
        return NO;
    }

    return YES;
}
