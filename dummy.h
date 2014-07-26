#include "ece454_fs.h"
typedef struct FSDIR {
    struct fsDirent files[100];
    int currentFile;
    char *path;
    struct FSDIR *next;
} FSDIR;