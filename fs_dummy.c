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
#include "ece454rpc_types.h"
#include <string.h>
#include "server_side.c"
#include <errno.h>

struct fsDirent dent;
//create hash table to map localfoldername to domain/ip + port number

typedef struct FSDIR {
    // Vector array for subfolders
    // Pointer to somwehere in the array (int)
    File *files;
    File *currentFile;
    DIR *parent;
} FSDIR;

typedef struct mount {
    // Local foldername
    // servername
    // port
    char *ipOrDomName;
    unsigned int port;
    char *folderName;
    struct mount *next;
} mount;

mount *m_head = NULL;
mount *m_tail = NULL;
mount *m_current = NULL;
mount *m_previous = NULL;

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
    arg_type *args = malloc(sizeof(arg_type));
    args->arg_val = (void *)localFolderName;
    args->arg_size = sizeof(localFolderName);

    return_type r = mount(1, args);

    if((int *)r.return_val == 0){
        if(m_head == NULL){
            m_head = (mount *)malloc(sizeof(mount));
            m_head->ipOrDomName = srvIpOrDomName;
            m_head->port = srvPort;
            m_head->folderName = localFolderName;

            m_tail = m_head;
            m_current = m_head;
        }else{
            mount m_newMount = (mount *)malloc(sizeof(mount));
            m_newMount->ipOrDomName = srvIpOrDomName;
            m_newMount->port = srvPort;
            m_newMount->folderName = localFolderName;

            m_tail->next = m_newMount;
            m_tail = m_tail->next;
        }
        return 0;
    }else{
        return -1;
    }
}

int fsUnmount(const char *localFolderName) {
    // tell the server that the client will be unmounted
    // remove the list of active clients on the server
    // Remove the hash table relation
    arg_type *args = malloc(sizeof(arg_type));
    args->arg_val = (void *)localFolderName;
    args->arg_size = sizeof(localFolderName);

    return_type r = unmount(1, args);

    if((int *)r.return_val == 0){
        m_current = m_head;
        while(m_current != NULL){
            if(m_current->folderName == localFolderName && m_current == m_head){
                m_head = m_head->next;
            }else{
                m_previous->next = m_current->next;
                free(m_current);
            }
        }
        return 0;
    }else{
        return -1;
    }
}

FSDIR* fsOpenDir(const char *folderName) {
    // RPC call getdir(folderName)
    // return FSDIR
    arg_type *args = (arg_type *)malloc(sizeof(arg_type));
    //write algo to get ip adress
    args->arg_val = (void *)ip_address;
    args->arg_size = sizeof(ip_address);

    args->next = (arg_type *)malloc(sizeof(arg_type));
    args->next->arg_val = (void *)folderName;
    args->next->arg_size = sizeof(folderName);

    return_type r = open(2, args);

    if(r.return_val != NULL){
        FSDIR *fsdir = (FSDIR *)r.return_val;
        return fsdir;
    }else{
        return NULL;
    }
}

int fsCloseDir(FSDIR *folder) {
    // Delete mapping from hash table
    // Return true if success else false
    arg_type *args = (arg_type *)malloc(sizeof(arg_type));
    args->arg_val = (void *)folder;
    args->arg_size = sizeof(folder);

    return_type r = closeDir(1, args);

    if(r.return_val == 0){
        return 0;
    }else{
        return -1;
    }
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    arg_type *args = (arg_type *)malloc(sizeof(arg_type));
    args->arg_val = folder;
    args->arg_size = sizeof(folder);
    /*const int initErrno = errno;
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
    return &dent;*/
}

int fsOpen(const char *fname, int mode) {
    // rpc call fsOpen(fname, mode);
    arg_type *args = (arg_type *)malloc(sizeof(arg_type));
    args->arg_val = (void *)ip_address;
    args->arg_size = sizeof(ip_address);

    args->next = (arg_type *)malloc(sizeof(arg_type));
    args->next->arg_val = (void *)fname;
    args->next->arg_size = sizeof(fname);

    args->next->next = (arg_type *)malloc(sizeof(arg_type));
    args->next->next->arg_val = (void *)mode;
    args->next->next->arg_size = sizeof(int);

    return_type r = open(3, args);

    if((int)r.return_val > 0){
        return (int)r.return_val;
    }else{
        return -1;
    }

    /*int flags = -1;

    if(mode == 0) {
	flags = O_RDONLY;
    }
    else if(mode == 1) {
	flags = O_WRONLY | O_CREAT;
    }

    return(open(fname, flags, S_IRWXU));*/
}

int fsClose(int fd) {
    arg_type *args = (arg_type *)malloc(sizeof(arg_type));
    args->arg_val = (void *)ip_address;
    args->arg_size = sizeof(ip_address);

    args->next = (arg_type*)malloc(sizeof(arg_type));
    args->next->arg_val = (void *)fd;
    args->next->arg_size = sizeof(int);

    return_type r = close(2, args);

    if((int)r.return_val == 0){
        return 0;
    }else{
        return -1;
    }

    //return(close(fd));
}

int fsRead(int fd, void *buf, const unsigned int count) {
    arg_type *args = (arg_type *)malloc(sizeof(arg_type));
    args->arg_val = (void *)fd;
    args->arg_size = sizeof(int);

    args->next = (arg_type *)malloc(sizeof(arg_type));
    args->next->arg_val = (void *)buf;
    args->next->arg_size = sizeof(buf);

    args->next->next = (arg_type *)malloc(sizeof(arg_type));
    args->next->next->arg_val = (void *)count;
    args->next->next->arg_size = sizeof(unsigned int);

    return_type r = read(3, args);

    if((int)r.return_val > 0){
        return (int)r.return_val;
    }else{
        return -1;
    }

    //return(read(fd, buf, (size_t)count));
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    arg_type *args = (arg_type *)malloc(sizeof(arg_type));
    args->arg_val = (void *)fd;
    args->arg_size = sizeof(int);

    args->next = (arg_type *)malloc(sizeof(arg_type));
    args->next->arg_val = (void *)buf;
    args->next->arg_size = sizeof(buf);

    args->next->next = (arg_type *)malloc(sizeof(arg_type));
    args->next->next->arg_val = (void *)count;
    args->next->next->arg_size = sizeof(int);

    return_type r = write(3, args);

    if((int)r.return_val > 0){
        return (int)r.return_val;
    }else{
        return -1;
    }
    //return(write(fd, buf, (size_t)count));
}

int fsRemove(const char *name) {
    return(remove(name));
}
