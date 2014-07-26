/* 
 * Mahesh V. Tripunitara
 * University of Waterloo
 * You specify what goes in this file. I just have a "dummy"
 * specification of the FSDIR type.
 */

#ifndef _ECE_FS_OTHER_INCLUDES_
#define _ECE_FS_OTHER_INCLUDES_
#include <sys/types.h>
#include <dirent.h>

typedef struct FSDIR {
	char path[256];
	struct FSDIR *next;
} FSDIR;

typedef struct OpenFile {
    int fd;
    char filepath[256];
    struct OpenFile *next;
} OpenFile;

typedef struct mount {
    char ipOrDomName[256];
    unsigned int port;
    char foldername[256];
    struct mount *next;
    struct FSDIR *opendirs;
} mount;


#endif
