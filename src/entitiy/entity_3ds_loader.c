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
#include "scene_node.h"

#include "entity_priv.h"

typedef struct tds_chunk_t {
    unsigned short chunkId;
    unsigned int chunkLen;
} tds_chunk_t;

typedef struct tds_vertex_t {
    vec3_t position;
    short flags;

    struct tds_vertex_t * next;
} tds_vertex_t;

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

static float TDS_ReadFloat(filehandle_t * file) {
    float f;
    FS_read(file, &f, 4);
    return f;
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

static BOOL TDS_ReadVertex(filehandle_t * file, mesh_t * mesh) {
    int i;

    mesh->num_verticies = TDS_ReadShort(file);
    mesh->vertices = (vertex_t *) malloc(mesh->num_verticies * sizeof(vertex_t));

    for (i = 0; i < mesh->num_verticies; i++) {
        mesh->vertices[i].position[0] = TDS_ReadFloat(file);
        mesh->vertices[i].position[1] = TDS_ReadFloat(file);
        mesh->vertices[i].position[2] = TDS_ReadFloat(file);
    }

    return TRUE;
}

static BOOL TDS_ReadIndex(filehandle_t * file, mesh_t * mesh) {
    int i;

    mesh->num_indices = TDS_ReadShort(file) * 3;
    mesh->indices = (unsigned short *) malloc(mesh->num_indices * sizeof(unsigned short));

    for (i = 0; i < mesh->num_indices; i += 3) {
        mesh->indices[i + 0] = TDS_ReadShort(file);
        mesh->indices[i + 1] = TDS_ReadShort(file);
        mesh->indices[i + 2] = TDS_ReadShort(file);

        (void) TDS_ReadShort(file);
    }

    return TRUE;
}

static BOOL TDS_ReadTextureCoord(filehandle_t * file, mesh_t * mesh) {
    int i;

    if (TDS_ReadShort(file) != mesh->num_verticies) {
        printf("[3DS] Model has an error\n");
        return FALSE;
    }
    for (i = 0; i < mesh->num_verticies; i++) {
        mesh->vertices[i].textureCoord[0] = TDS_ReadFloat(file);
        mesh->vertices[i].textureCoord[1] = TDS_ReadFloat(file);
    }


    return TRUE;
}

static BOOL TDS_ReadMaterialGroup(filehandle_t * file, mesh_t * mesh) {
    char buffer[255];
    short num;
    int i;

    TDS_ReadStringZ(file, buffer);
    printf("[3DS] Mesh has association with '%s'\n", buffer);

    num = TDS_ReadShort(file);
    for (i = 0; i < num; i++) {
        TDS_ReadShort(file);
    }


    return TRUE;
}

BOOL TDS_Load3DS(filehandle_t * file, scene_node_t * entity) {
    tds_chunk_t chunk;
    scene_node_t * currentNode;
    char buffer[255];

    while (!FS_eof(file)) {
        TDS_ReadChunk(file, &chunk);
        printf("[3DS] Chunk ID 0x%04X with length %u\n", chunk.chunkId, chunk.chunkLen);

        switch (chunk.chunkId) {
        case 0x4d4d:
        case 0x3d3d:
        case 0xafff:
        case 0x4100:
            break;

        case 0x0002:
            printf("[3DS] Version: %d\n", TDS_ReadLong(file));
            break;

        case 0x4000:
            TDS_ReadStringZ(file, buffer);
            currentNode = SCENE_NewNodeWithParent(entity, buffer, NODE_TYPE_STATIC_MESH);

            printf("[3DS] Reading %s\n", currentNode->name);
            break;

        case 0xa000:
            TDS_ReadStringZ(file, buffer);
            printf("[3DS] Mat name = %s\n", buffer);
            break;

        case 0x4110:
            TDS_ReadVertex(file, currentNode->object.mesh);
            break;

        case 0x4120:
            TDS_ReadIndex(file, currentNode->object.mesh);
            break;

        case 0x4140:
            TDS_ReadTextureCoord(file, currentNode->object.mesh);
            break;

        case 0x4130:
            TDS_ReadMaterialGroup(file, currentNode->object.mesh);
            break;

        default:
            TDS_SkipChunk(file, &chunk);
            break;
        }
    }

    return TRUE;
}
