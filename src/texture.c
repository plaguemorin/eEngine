/*
 * texture.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "filesystem.h"
#include "texture.h"

typedef union {
    struct {
        unsigned char id;
        unsigned char colour_map_type;
        unsigned char image_type;

        short int colour_map_first_entry;
        short int colour_map_length;
        unsigned char map_entry_size;

        short int horizontal_origin;
        short int vertical_origin;
        short int width;
        short int height;
        unsigned char pixel_depth;
        unsigned char image_descriptor;
    }__attribute__((packed));

    unsigned char header[18];
} tga_header_t;

extern texture_t * loadNativePNG(const char * name);
static texture_t * TEX_LoadTGA(const char * path);

void TEX_Init() {

}

static texture_t * TEX_LoadTGA(const char * path) {
    FILE * fTGA;
    texture_t * texture;
    tga_header_t tgaheader;

    fTGA = fopen(path, "rb");
    if (!fTGA) {
        printf("[TEX] TGA path invalid: %s\n", path);
        return NULL;
    }

    if (fread(&tgaheader.header, sizeof(tga_header_t), 1, fTGA) == 0) {
        printf("[TEX] Unable to read header\n");

        fclose(fTGA);
        return NULL;
    }

    texture = malloc(sizeof(texture_t));
    if (!texture) {
        printf("[TEX] Unable to allocate memory for texture\n");

        fclose(fTGA);
        return NULL;
    }
    memset(texture, 0, sizeof(texture_t));

    /* Determine The TGA Width	(highbyte*256+lowbyte) */
    texture->width = tgaheader.width; //tga.header[1] * 256 + tga.header[0];

    /* Determine The TGA Height	(highbyte*256+lowbyte) */
    texture->height = tgaheader.height; //tga.header[3] * 256 + tga.header[2];

    /* Determine the bits per pixel */
    texture->bpp = tgaheader.pixel_depth;

    /* Make sure everything is valid */
    if ((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp != 32))) {
        printf("[TEX] Texture is invalid\n");

        free(texture);
        texture = NULL;
        fclose(fTGA);

        return NULL;
    }

    /* Load the type */
    texture->type = (texture->bpp == 24) ? TEXTURE_TYPE_BGR : TEXTURE_TYPE_BGRA;

    /* Compute the total amount of memory needed to store data */
    texture->data_length = ((texture->bpp / 8) * texture->width * texture->height);

    /* Allocate */
    texture->data = (unsigned char*) malloc(texture->data_length);
    if (!texture->data) {
        printf("[TEX] Unable to allocate memory for texture data\n");

        free(texture);
        texture = NULL;
        fclose(fTGA);

        return NULL;
    }

    /* If The File Header Matches The Uncompressed Header */
    if (tgaheader.image_type == 2) {
        /* Load An Uncompressed TGA */
        if (fread(texture->data, 1, texture->data_length, fTGA) != texture->data_length) {
            printf("[TEX] Unable to read TGA data\n");

            free(texture->data);
            free(texture);
            texture = NULL;
        }

        /* Byte swapping optimised by Steve Thomas
         for (cswap = 0; cswap < (int) texture->data_length; cswap += (texture->bpp / 8)) {
         texture->data[cswap] ^= texture->data[cswap + 2] ^= texture->data[cswap] ^= texture->data[cswap + 2];
         }
         */
    }
    /* If The File Header Matches The Compressed Header */
    else if (tgaheader.image_type == 10) {
        /*
         * TODO: Load A Compressed TGA
         */
    } else {
        printf("[TEX] Unknown TGA header\n");

        free(texture->data);
        free(texture);
        texture = NULL;
    }

    fclose(fTGA);
    return texture;
}

texture_t * TEX_LoadTexture(const char * path) {
    texture_t * text;
    /* TODO: Use the file system subsystem */
    /* TODO: Replace with a command style pattern */

    text = loadNativePNG(path);

    if (!text) {
        text = TEX_LoadTGA(path);
        if (!text) {
            printf("[TEX] Error\n");
        }
    }

    if (text) {
        text->renderer_data = NULL;
    }
    return text;
}

