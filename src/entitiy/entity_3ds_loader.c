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

static short TDS_ReadShort(filehandle_t * file) {
    short s;
    FS_read(file, &s, 2);
    return s;
}

static int TDS_ReadLong(filehandle_t * file) {
    int l;
    FS_read(file, &l, 4);
    return l;
}

static BOOL TDS_ReadChunk(filehandle_t * file, tds_chunk_t * chunk) {
    chunk->chunkId = TDS_ReadShort(file);
    chunk->chunkLen = TDS_ReadLong(file);

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
    FS_seek(file, FS_tell(file) + chunk->chunkLen - 6);
}

BOOL TDS_Load3DS(filehandle_t * file, entity_t * entity) {
    tds_chunk_t chunk;
    char buffer[255];

    while (!FS_eof(file)) {
        TDS_ReadChunk(file, &chunk);
        printf("[3DS] Chunk ID 0x%04X with len %u\n", chunk.chunkId, chunk.chunkLen);

        switch (chunk.chunkId) {
            case 0x4d4d:
            case 0x3d3d:
            case 0xafff:
                break;

            case 0x0002:
                printf("[3DS] Version: %d\n", TDS_ReadLong(file));
                break;

            case 0x4000:
                TDS_ReadStringZ(file, buffer);
                break;

            case 0xa000:
                TDS_ReadStringZ(file, buffer);
                break;

            default:
                TDS_SkipChunk(file, &chunk);
                break;
        }
    }

    return FALSE;
}