/* 
 * Mahesh V. Tripunitara
 * University of Waterloo
 * A dummy implementation of the functions for the remote file
 * system. This file just implements those functions as thin
 * wrappers around invocations to the local filesystem. In this
 * way, this dummy implementation helps clarify the semantics
 * of those functions. Look up the man pages for the calls
 * such as opendir() and read() that are made here.
 */
#include "ece454_fs.h"
#include <string.h>

struct fsDirent dent;
//create hash table to map localfoldername to domain/ip + port number

typedef struct FSDIR {
    // Vector array for subfolders
    // Pointer to somwehere in the array (int)
} FSDIR;

typedef struct mount {
    // Local foldername
    // servername
    // port
} mount;

int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
    struct stat sbuf;
    // TCP connection to server
    // mount(clientaddr, clientport)
    
    // client:
    // rpc to "mount"
    // if success:
    //  create mount struct
    //  return 0
    // else 
    //  return -1
    //
    //return 0 if success, -1 with error
}

int fsUnmount(const char *localFolderName) {
    // tell the server that the client will be unmounted
    // remove the list of active clients on the server
    // Remove the hash table relation
    return 0;
}

FSDIR* fsOpenDir(const char *folderName) {
    // RPC call getdir(folderName)
    // return FSDIR
    return(opendir(folderName));
}

int fsCloseDir(FSDIR *folder) {
    // Delete mapping from hash table
    // Return true if success else false

    return(closedir(folder));
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    struct dirent *d = readdir(folder);

    if(d == NULL) {
	if(errno == initErrno) errno = 0;
	return NULL;
    }

    if(d->d_type == DT_DIR) {
	dent.entType = 1;
    }
    else if(d->d_type == DT_REG) {
	dent.entType = 0;
    }
    else {
	dent.entType = -1;
    }

    memcpy(&(dent.entName), &(d->d_name), 256);
    return &dent;
}

int fsOpen(const char *fname, int mode) {
    // rpc call fsOpen(fname, mode);
    int flags = -1;

    if(mode == 0) {
	flags = O_RDONLY;
    }
    else if(mode == 1) {
	flags = O_WRONLY | O_CREAT;
    }

    return(open(fname, flags, S_IRWXU));
}

int fsClose(int fd) {
    return(close(fd));
}

int fsRead(int fd, void *buf, const unsigned int count) {
    return(read(fd, buf, (size_t)count));
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    return(write(fd, buf, (size_t)count)); 
}

int fsRemove(const char *name) {
    return(remove(name));
}
