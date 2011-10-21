/*
 * entity_3ds_loader.c
 *
 *  Created on: 2011-10-19
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "filesystem.h"
#include "lexer.h"
#include "entities.h"
#include "material.h"

#include "entity_priv.h"

typedef struct tds_chunk_t {
    unsigned short chunkId;
    unsigned int chunkLen;
} tds_chunk_t;

static BOOL TDS_ReadChunk(filehandle_t * file, tds_chunk_t * chunk) {
    FS_read(file, &chunk->chunkId, 2);
    FS_read(file, &chunk->chunkLen, 4);

    return YES;
}

static void TDS_ReadStringZ(filehandle_t * file, char * buffer) {
    int i;
    char l_char;
    i = 0;

    do {
        FS_read(file, &l_char, 1);
        buffer[i++] = l_char;
    } while (l_char != '\0' && i < 20);
}

static void TDS_SkipChunk(filehandle_t * file, tds_chunk_t * chunk) {
    FS_seek(file, FS_tell(file) + chunk->chunkLen);
}

BOOL TDS_Load3DS(filehandle_t * file, entity_t * entity) {
    tds_chunk_t chunk;
    char buffer[255];

    while (!FS_eof(file)) {
        TDS_ReadChunk(file, &chunk);

        switch (chunk.chunkId) {
            case 0x4d4d:
            case 0x3d3d:
                break;

            case 0x4000:
                TDS_ReadStringZ(file, &buffer);
                break;

            case 0xa000:
                TDS_ReadStringZ(file, &buffer);
                break;

            default:
                TDS_SkipChunk(file, &chunk);
                break;
        }
    }

    return FALSE;
}
