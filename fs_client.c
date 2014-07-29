#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

void printBuf(char *buf, int size) {
    /* Should match the output from od -x */
    int i;
    for(i = 0; i < size; ) {
	if(i%16 == 0) {
	    printf("%08o ", i);
	}

	int j;
	for(j = 0; j < 16;) {
	    int k;
	    for(k = 0; k < 2; k++) {
		if(i+j+(1-k) < size) {
		    printf("%02x", (unsigned char)(buf[i+j+(1-k)]));
		}
	    }

	    printf(" ");
	    j += k;
	}

	printf("\n");
	i += j;
    }
}

int main(int argc, char *argv[]) {
    char *dirname = NULL;

    if(argc > 1) {
        dirname = argv[1];
        printf("hello, is it me you're looking for\n");
        // printf("%s\n", dirname);
    }
    else {
        dirname = (char *)calloc(strlen(".")+1, sizeof(char));
        strcpy(dirname, ".");
    }

    printf("%s\n", dirname);

    printf("fsMount(): %d\n", fsMount(argv[2], atoi(argv[3]), dirname));
    
    FSDIR *fd = fsOpenDir(dirname);

    if(fd == NULL) {
    	perror("fsOpenDir"); exit(1);
    }

    printf("file successfully opened!\n");

    struct fsDirent *fdent = NULL;
    for(fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
		printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    }

    if(errno != 0) {
		perror("fsReadDir");
    }

    printf("fsCloseDir(): %d\n", fsCloseDir(fd));

    // int ff = fsOpen("/dev/urandom", 0);
    int ff = fsOpen("test/urandom", 0);
    printf("the fd is %d\n", ff);
    if(ff < 0) {
	   perror("fsOpen"); exit(1);
    }
    else printf("fsOpen(): %d\n", ff);

    char fname[15];
    if(fsRead(ff, (void *)fname, 10) < 0) {
	    perror("fsRead"); exit(1);
    }
    printf("the buffer read %s\n", fname);

    int i;
    for(i = 0; i < 10; i++) {
    // printf("%d\n", ((unsigned char)(fname[i]))%26);
    fname[i] = ((unsigned char)(fname[i]))%26 + 'a';
    }
    fname[10] = (char)0;
    printf("Filename to write: %s\n", (char *)fname);

    char buf[50];
    if(fsRead(ff, (void *)buf, 50) < 0) {
	perror("fsRead(2)"); exit(1);
    }

    printBuf(buf, 50);

    printf("fsClose(): %d\n", fsClose(ff));

    ff = fsOpen("test/tuvwxyzabc", 1);
    if(ff < 0) {
	perror("fsOpen(write)"); exit(1);
    }

    if(fsWrite(ff, buf, 50) < 50) {
	fprintf(stderr, "fsWrite() wrote fewer than 256\n");
    }

    if(fsClose(ff) < 0) {
    perror("fsClose"); exit(1);
    }

    printf("fsRemove(%s): %d\n", fname, fsRemove("test/tuvwxyzabc"));

    return 0;
}
