/*
 * filesystem.c
 *
 *  Created on: 2011-10-03
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global.h"
#include "filesystem.h"

static char fs_gamedir[35335];

void FS_Init() {
    char * gameDir;
    // Figure out basedir
    gameDir = getcwd(NULL, 0);

    strncpy(fs_gamedir, gameDir, sizeof(fs_gamedir));
    free(gameDir);

    printf("[FS] Game dir: %s\n", fs_gamedir);
}

filehandle_t * FS_OpenFileRead(const char * filePath) {
    int fullPathSize;
    filehandle_t * fs;
    int pos;
    int end;

    // Allocate the handle
    fs = malloc(sizeof(filehandle_t));
    if (!fs) {
        printf("[FS] Unable to allocate memory for file handle\n");
        return NULL;
    }

    // Allocate the path
    fullPathSize = strlen(fs_gamedir) + strlen(filePath) + 2;
    fs->fullPath = malloc(sizeof(char) * (fullPathSize + 1));
    if (!fs->fullPath) {
        free(fs);
        printf("[FS] Unable to allocate memory for file path\n");
        return NULL;
    }

    snprintf(fs->fullPath, fullPathSize, "%s/%s", fs_gamedir, filePath);
    printf("[FS] Loading %s\n", fs->fullPath);

    // Open the file
    fs->privateHandle = fopen(fs->fullPath, "rb");
    if (!fs->privateHandle) {
        printf("[FS] Unable to load file %s !\n", fs->fullPath);
        free(fs->fullPath);
        free(fs);
        return NULL;
    }

    // Populate the positions
    pos = ftell(fs->privateHandle);
    fseek(fs->privateHandle, 0, SEEK_END);
    end = ftell(fs->privateHandle);
    fseek(fs->privateHandle, pos, SEEK_SET);
    fs->filesize = end;

    // Read data
    fs->filedata = calloc(fs->filesize + 1, sizeof(char));
    if (fs->filedata == NULL) {
        printf("[FS] Unable allocate memory for content of %s (%lu bytes) !\n", fs->fullPath, fs->filesize);
        fclose(fs->privateHandle);
        free(fs->fullPath);
        free(fs);
        return NULL;
    }
    // Update the pointers
    fs->ptrStart = fs->ptrCurrent = fs->filedata;

    fread(fs->filedata, sizeof(char), fs->filesize, fs->privateHandle);

    fs->ptrEnd = fs->ptrStart + fs->filesize;
    fs->bLoaded = 1;

    fs->fileExtention = (char *) (strrchr(fs->fullPath, '.') + 1);

    fclose(fs->privateHandle);
    fs->privateHandle = NULL;

    return fs;
}

void FS_Close(filehandle_t * fhandle) {
    if (fhandle->fullPath) {
        free(fhandle->fullPath);
        fhandle->fullPath = NULL;
        fhandle->fileExtention = NULL;
    }

    if (fhandle->filedata) {
        free(fhandle->filedata);
        fhandle->filedata = NULL;
    }

    free(fhandle);
}

void FS_StripExtension(const char *in, char *out) {
    while (*in && *in != '.') {
        *out++ = *in++;
    }

    *out = '\0'; // NUL-terminate string.
}

char *FS_FileExtension(const char *in) {
    static char exten[8];
    char* j;
    char* i;

    i = (char*) in + strlen(in);
    j = (char*) exten + 7;

    exten[7] = '\0';

    while (*i != '.') {
        j--;
        i--;
        *j = *i;
        //in--;
    }
    j++;

    //exten[7] = '\0'; // NUL-terminate string.

    return j;
}

void FS_DirectoryPath(char *in, char *out) {
    char *s;

    s = in + strlen(in);
    out += strlen(in);

    while (s != in && *s != '/' && *s != '\\') {
        s--;
        out--;
    }

    while (s != in - 1)
        *out-- = *s--;
}

int FS_read(filehandle_t * file, void *buffer, unsigned int nbyte) {
    unsigned int readRead = MIN(file->ptrEnd - file->ptrCurrent, nbyte);

    if (readRead > 0) {
        memcpy(buffer, file->ptrCurrent, readRead);
        file->ptrCurrent += readRead;
    }

    return readRead;
}

int FS_seek(filehandle_t * file, long int offset) {
    unsigned int realOffset = MIN(offset, file->filesize);
    file->ptrCurrent = file->ptrStart + realOffset;
    return realOffset;
}

unsigned long FS_tell(filehandle_t * file) {
    return (file->ptrCurrent - file->ptrStart);
}

BOOL FS_eof(filehandle_t * file) {
    return (file->ptrCurrent >= file->ptrEnd) ? TRUE : FALSE;
}

char* FS_GetExtensionAddress(char* string) {
    char* extension;

    extension = &string[strlen(string) - 1];

    while (*extension != '.' && extension != string)
        extension--;

    return (extension + 1);
}
