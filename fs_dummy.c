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
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <net/if.h>
#include <ifaddrs.h>
#include "ece454_fs.h"
#include "ece454rpc_types.h"

OpenFile *of_head = NULL;
OpenFile *of_tail = NULL;

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

char *findRootName(const char *fullpath) {
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

mount *findMount(const char *localFolderName) {
    mount *curr = m_head;
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

    return curr;
}

int addMount(const char *ip, const int port, const char *localFolderName) {
    if(m_head == NULL){
        // Initialize the mount type
        m_head = (mount *)malloc(sizeof(mount));
        m_head->ipOrDomName = ip;
        m_head->port = port;
        m_head->foldername = localFolderName;
        // Initialize a first dummy FSDIR
        m_head->opendirs = malloc(sizeof(FSDIR));
        m_head->opendirs->path = "dummy";
        m_head->opendirs->next = NULL;

        m_tail = m_head;
    }
    else{
        mount *m_newMount = (mount *)malloc(sizeof(mount));
        m_newMount->ipOrDomName = ip;
        m_newMount->port = port;
        m_newMount->foldername = localFolderName;

        m_newMount->opendirs = malloc(sizeof(FSDIR));
        m_newMount->opendirs->path = "dummy";
        m_newMount->opendirs->next = NULL;

        m_tail->next = m_newMount;
        m_tail = m_tail->next;
    }

    return 0;
}

int removeMount(const char *localFolderName) {
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

int removeFSDIR(mount *mounted, const char *folderName) {
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

FSDIR *findFSDIR(mount *mounted, const char *folderName) {
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

int findFileDesriptor(const char *path) {
    OpenFile *curr = of_head;

    while (curr != NULL) {
        if (curr->filepath == path) {
            return curr->fd;
        }
        curr = curr->next;
    }

    return -1;
}

int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
    char *interfaceip = obtaininterfaceip("wlan0");
    printf("interfaceip: %s\n", interfaceip);

    return_type ans = make_remote_call(srvIpOrDomName,
        srvPort, "sMount", 2, strlen(localFolderName), (void *)localFolderName, strlen(interfaceip), (void *) interfaceip);

    printf("finished making rpc\n");
    
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

    mount *found = findMount(localFolderName);
    if (found == NULL) {
        return -1;
    }

    char *interfaceip = obtaininterfaceip("wlan0");

    return_type ans = make_remote_call(found->ipOrDomName, found->port, "sUnmount", 1, strlen(interfaceip),
        (void *)interfaceip);

    if(*(int *)ans.return_val == 0){
        return removeMount(localFolderName);
    }else{
        return -1;
    }
}

//Given folderPath
FSDIR* fsOpenDir(const char *folderName) {
    printf("before open dir\n");
    char *interfaceip = obtaininterfaceip("wlan0");   
    char *rootname = findRootName(folderName);
    mount *mounted = findMount(rootname);

    return_type ans = make_remote_call(mounted->ipOrDomName, 
        mounted->port, "sOpenDir", 2, strlen(interfaceip), (void *)interfaceip, strlen(folderName), (void *) folderName);

    printf("after open dir\n");
    if (*(int *)ans.return_val == 0) {
        FSDIR *fsdir = malloc(sizeof(FSDIR));
        fsdir->path = folderName;
        fsdir->next = NULL;
        addFSDIR(mounted, fsdir);
        return fsdir;
    }
    else {
        return NULL;
    }
}

int fsCloseDir(FSDIR *folder) {
    char *interfaceip = obtaininterfaceip("wlan0");   
    char *rootname = findRootName(folder->path);
    mount *mounted = findMount(rootname);
    free(rootname);

    return_type ans = make_remote_call(mounted->ipOrDomName, mounted->port, "sCloseDir", 2, strlen(interfaceip), (void *)interfaceip, strlen(folder->path), (void *)folder->path);

    if(*(int *)ans.return_val == 0){
        return removeFSDIR(mounted, folder->path);
    }else{
        return -1;
    }
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    char *interfaceip = obtaininterfaceip("wlan0");   
    char *rootname = findRootName(folder->path);
    mount *mounted = findMount(rootname);

    return_type ans = make_remote_call(mounted->ipOrDomName, mounted->port, "sReadDir", 2, strlen(interfaceip), (void *)interfaceip, strlen(folder->path), (void *)folder->path);

    if (ans.return_val != NULL) {
        void *ptr = ans.return_val;
        int fileType;
        struct fsDirent *fsdirent = malloc(sizeof(struct fsDirent));
        memcpy(&ans, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(fsdirent->entName, ptr, sizeof(sizeof(char)*256));
        return fsdirent;
    }

    return NULL;
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

    if(*(int *)ans.return_val > 0){
        int fd = *(int *)ans.return_val;
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

    if(*(int *)ans.return_val == 0){
        return 0;
    }else{
        return -1;
    }
}

int fsRead(int fd, void *buf, const unsigned int count) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *rootname = findRootName(of->filepath);
    mount *current_mount = findMount(rootname);

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sRead", 4, strlen(interfaceip), interfaceip,
        sizeof(int), fd, sizeof(int), sizeof(buf), sizeof(int), count);

    void *ptr = ans.return_val;
    int bytesread;
    memcpy(&bytesread, ptr, sizeof(int));
    ptr += sizeof(int);

    if(bytesread > 0){
        memcpy(buf, ptr, bytesread);
        return bytesread;
    }else{
        return -1;
    }
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *rootname = findRootName(of->filepath);
    mount *current_mount = findMount(rootname);
    free(rootname);

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sWrite", 4, strlen(interfaceip), (void *)interfaceip,
        sizeof(int), fd, sizeof(buf), buf, sizeof(int), count);

    if(*(int *)ans.return_val > 0){
        return *(int *)ans.return_val;
    }else{
        return -1;
    }
}

int fsRemove(const char *name) {
    char *interfaceip = obtaininterfaceip("wlan0");
    char *rootname = findRootName(name);
    mount *current_mount = findMount(rootname);
    free(rootname);

    int fd = findFileDesriptor(name);

    if (fd == -1)
        return -1;

    FSDIR *fsdir = findFSDIR(current_mount, name);

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sRemove", 2, strlen(interfaceip), (void *)interfaceip,
        sizeof(int), fd);

    if (*(int *)ans.return_val == 0) {
        return removeOpenFile(fd);
    }

    return -1;
}
