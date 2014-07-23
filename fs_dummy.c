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

typedef struct OpenFile {
    int fd;
    char *filepath;
    struct OpenFile *next;
} OpenFile;

mount *of_head = NULL;
mount *of_tail = NULL;

typedef struct FSDIR {
    struct fsDirent files[100]
    int currentFile;
    char *path;
    struct FSDIR *next;
} FSDIR;

typedef struct mount {
    char *ipOrDomName;
    unsigned int port;
    char *folderName;
    struct mount *next;
    struct FSDIR *opendirs;
} mount;

mount *m_head = NULL;
mount *m_tail = NULL;

// Function used to return the ip address string of a given interface
// Sampled code from: http://stackoverflow.com/questions/2146191/
// obtaining-local-ip-address-using-getaddrinfo-c-function
char* obtaininterfaceip(char *interface_name) {
    char *ip;
    struct sockaddr blah;
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char buf[64];

    if(getifaddrs(&myaddrs) != 0)
    {
        printf("getifaddrs not working");
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }

            case AF_INET6:
            {
                struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                in_addr = &s6->sin6_addr;
                break;
            }

            default:
                continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf)))
        {
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        }
        else
        {
            // printf("omg its here %s\n", buf);
            if (strcmp(interface_name, ifa->ifa_name) == 0) {
                ip = malloc(sizeof(buf));
                memcpy(ip, &buf, sizeof(buf));
                break;
            }
        }
    }

    freeifaddrs(myaddrs);
    return ip;
}

char *findRootName(char *fullpath) {
    char find = '/';

    const char *ptr = strchr(fullpath, find);
    int index;
    if(ptr) {
       index = ptr - fullpath;
    }
    else {
        return fullpath;
    }
    
    char *localFolderName = malloc(index);
    strncpy(localFolderName, fullpath, index);
    return localFolderName;
}

mount findMount(char *localFolderName) {
    mount curr = m_head;
    int found = 0;
    while(curr != NULL) {
        if (curr->foldername == localFolderName) {
            found = 1;
            break;
        }

        curr = curr->next;
    }

    if (!found) {
        return NULL;
    }

    return *curr;
}

int addMount(char *ip, int port, char *localFolderName) {
    if(m_head == NULL){
        // Initialize the mount type
        m_head = (mount *)malloc(sizeof(mount));
        m_head->ipOrDomName = ip;
        m_head->port = srvPort;
        m_head->folderName = localFolderName;
        
        // Initialize a first dummy FSDIR
        m_head->opendirs = malloc(sizeof(FSDIR));
        m_head->opendirs->currentFile = 0;
        m_head->opendirs->path = "dummy";
        m_head->opendirs->next = NULL;

        m_tail = m_head;
    }
    else{
        mount m_newMount = (mount *)malloc(sizeof(mount));
        m_newMount->ipOrDomName = ip;
        m_newMount->port = srvPort;
        m_newMount->folderName = localFolderName;

        m_newMount->opendirs = malloc(sizeof(FSDIR));
        m_newMount->opendirs->currentFile = 0;
        m_newMount->opendirs->path = "dummy";
        m_newMount->opendirs->next = NULL;

        m_tail->next = m_newMount;
        m_tail = m_tail->next;
    }

    return 0;
}

int removeMount(char *localFolderName) {
    mount *curr = m_head;
    mount *prev = m_head;

    int removed = -1;
    while(curr != NULL) {
        if (curr->foldername == localFolderName) {
            if(curr == m_head){
                m_head = m_head -> next;
            } else{
                prev->next = curr->next;
            }
            free(curr);
            removed = 0;
        }

        if (prev != curr) {
            prev = prev->next;
        }

        curr = curr->next;
    }

    return removed;
}

int addFSDIR(mount *mounted, FSDIR *newfsdir) {
    FSDIR *curr = mounted->opendirs;

    while (curr -> next != NULL) {
        curr = curr ->next;
    }

    curr->next = newfsdir;

    return 0;
}

int removeFSDIR(mount *mounted, folderName) {
    FSDIR *prev = mounted->opendirs;
    FSDIR *curr = prev->next;

    while (curr != NULL) {
        if (curr->path == folderName) {
            prev->next = curr->next;
            free(curr);
            return 0;
        }
    }

    return -1;
}

FSDIR *findFSDIR(mount *mounted, folderName) {
    FSDIR *curr = mounted->opendirs;

    while (curr != NULL) {
        if (curr->path == folderName) {
            return curr;
        }
    }

    return NULL;
}

int addOpenFile(int fd, char *path) {
    OpenFile *curr = of_tail;
    OpenFile *newfile = malloc(sizeof(OpenFile));
    newfile->fd = fd;
    newfile->filepath = path;
    newfile->next = NULL;

    if (of_tail == NULL) {
        of_head = newfile;
        of_tail = newfile;
    }
    else {
        of_tail->next = newfile;
    }

    return 0;
}

int removeOpenFile(int fd) {
    OpenFile *curr = of_head;
    OpenFile *prev = of_tail;

    while (curr != NULL) {
        if (curr->fd == fd) {
            if (curr == of_head) {
                of_head = of_head->next;
            }
            else {
                prev->next = curr->next;
            }

            free(curr);
            return 0;
        }
    }

    return -1;
}

OpenFile *findOpenFile(int fd) {
    OpenFile *curr = of_head;

    while (curr != NULL) {
        if (curr->fd == fd) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
    char *interfaceip = obtaininterfaceip("wlan0")    

    return_type ans = make_remote_call(srvIpOrDomName, 
        srvPort, "sMount", 2, strlen(localFolderName), (void *)localFolderName, strlen(interfaceip), (void *) interfaceip);

    if(*(int *)ans.return_val == 0){
        return addMount(srvIpOrDomName, srvPort, localFolderName);
    }else{
        return -1;
    }
}

int fsUnmount(const char *localFolderName) {
    // tell the server that the client will be unmounted
    // remove the list of active clients on the server
    // Remove the hash table relation

    mount found = findMount(localFolderName);
    if (found == NULL) {
        return -1;
    }

    char *interfaceip = obtaininterfaceip("wlan0");

    return_type ans = make_remote_call(curr->ipOrDomName, curr->port, "sUnmount", 1, strlen(interfaceip),
        (void *)interfaceip);

    if(*(int *)ans.return_val == 0){
        return removeMount(localFolderName);
    }else{
        return -1;
    }
}

//Given folderPath
FSDIR* fsOpenDir(const char *folderName) {
    char *rootname = findRootName(folderName);
    mount *mounted = findMount(rootname);
    free(rootname);

    if (mounted == NULL) {
        return NULL;
    }

    return_type ans = make_remote_call(mounted->ipOrDomName, mounted->port, "sOpenDir", 1, strlen(folderName), folderName)

    char buf[1500];
    char *ptr;
    int file_count;

    FSDIR *fsdir = malloc(sizeof(FSDIR));
    fsdir->path = folderName;

    if(ans.return_val != NULL){
        ptr = buf;
        buf = ans.return_val

        memcpy(&file_count, ptr, sizeof(int));
        ptr += sizeof(int);

        int param_length;
        char *filename;
        int filetype;
        char entype[0];

        int count = 0;
        for (int i = 0; i < file_count; i++) {
            // Unmarshall the data [length, filename, file_type]
            memcpy(&param_length, ptr, sizeof(int));
            ptr += sizeof(int);
            memcpy(filename, ptr, param_length);
            ptr += param_length;
            memcpy(&filetype, ptr, sizeof(int));
            sprintf(entype, "%d", filetype)

            fsDirent *file = malloc(sizeof(fsDirent));
            strcpy(file->entName, filename);
            file->entType = entype;

            fsdir->files[count] = filename;
            fsdir->next = NULL;
            count += 1;
        }

        addFSDIR(mounted, fsdir);

        return fsdir;
    }else{
        return NULL;
    }
}

int fsCloseDir(FSDIR *folder) {
    char *rootname = findRootName(folder);
    mount *mounted = findMount(rootname);
    free(rootname);

    return_type ans = make_remote_call(mounted->ipOrDomName, mounted->port, "sCloseDir", 1, strlen(folder->path), folder->path);

    if(*(int *)r.return_val == 0){
        return removeFSDIR(mounted, folder->path);
    }else{
        return -1;
    }
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    fsDirent *fsdir = folder->files[folder->currentFile];
    folder->currentFile += 1;
    return fsdir;
}


int fsOpen(const char *fname, int mode) {
    // ip, filepath, mode
    // rpc call fsOpen(fname, mode);
    char *interfaceip = obtaininterfaceip("wlan0");
    char *localFolderName = findRootName(fname);
    mount *current_mount = findMount(localFolderName);
    free(localFolderName);

    if (current_mount == NULL) {
        return -1;
    }

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sOpen", 3, strlen(interfaceip), interfaceip,
        strlen(fname), fname, sizeof(int), &mode);

    if(*(int *)r.return_val > 0){
        int fd = *(int *)r.return_val;
        addOpenFile(fd, localFolderName);
        return fd;
    }else{
        return -1;
    }
}

int fsClose(int fd) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *rootname = findRootName(of->filepath);
    mount *current_mount = findMount(rootname);
    free(rootname);

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sClose", 2, strlen(interfaceip), interfaceip,
        sizeof(int), fd);

    if(*(int *)r.return_val == 0){`
        return 0;
    }else{
        return -1;
    }
}

int fsRead(int fd, void *buf, const unsigned int count) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *rootname = findRootName(of->path);
    mount *current_mount = findMount(rootname);

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sRead", 3, strlen(interfaceip), interfaceip,
        sizeof(int), fd, sizeof(int), count);
    
    buf = (void *)ans;
    if((int)r.return_val > 0){
        return (int)r.return_val;
    }else{
        return -1;
    }

}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *rootname = findRootName(of->path);
    mount *current_mount = findMount(rootname);
    free(rootname);

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sWrite", 4, strlen(interfaceip), interfaceip,
        sizeof(int), fd, sizeof(buf), buf, sizeof(int), count);

    if(*(int *)ans.return_val > 0){
        return *(int *)r.return_val;
    }else{
        return -1;
    }
}

int fsRemove(const char *name) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *rootname = findRootName(of->path);
    mount *current_mount = findMount(rootname);
    free(rootname);

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sRemove", 2, strlen(interfaceip), interfaceip);
    
    if (*(int *)ans.return_val == 0) {
        return removeOpenFile(fd);
    }

    return -1;
}
