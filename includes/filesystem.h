/*
 * filesystem.h
 *
 *  Created on: 2011-10-03
 *      Author: plaguemorin
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

typedef struct filehandle_t {
    /* Full file name (with path) */
    char * fullPath;

    /* Pointer to the extention of the file */
    char * fileExtention;

    /* Following is used when the file is loaded into memory */
    int bLoaded; /* Was file loaded into memory? */
    unsigned long filesize; /* Size of file data in bytes */

    unsigned char *ptrStart; /* pointer to start of file data block */
    unsigned char *ptrCurrent; /* pointer to current position in file data block */
    unsigned char *ptrEnd; /* pointer to end of file data block */

    void *filedata; /* file data loaded into memory */

    void * privateHandle;
} filehandle_t;

void FS_Init();

filehandle_t * FS_OpenFileRead(const char *);
filehandle_t * FS_OpenFileWrite(const char *);

void FS_Close(filehandle_t *);

void FS_StripExtension(const char *in, char *out);
char *FS_FileExtension(const char *in);
void FS_DirectoryPath(char *in, char *out);

char* FS_GetExtensionAddress(char* string);

int FS_read(filehandle_t *, void *buffer, unsigned int nbyte);
int FS_seek(filehandle_t *, long int offset);
unsigned long FS_tell(filehandle_t*);
BOOL FS_eof(filehandle_t *);

#endif /* FILESYSTEM_H_ */
