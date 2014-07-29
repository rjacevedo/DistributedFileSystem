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

bool debug = true;

OpenFile *of_head = NULL;
ClientMount *m_head = NULL;

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
        // index = fullpath - ptr;
    }
    else {
        return (void*)fullpath;
    }

    if(debug)printf("value of index is %d\n", index);
    char *localFolderName = calloc(index, sizeof(char));
    strncpy(localFolderName, fullpath, index);
    if(debug)printf("in findRootName: returned %s\n", localFolderName);
    return localFolderName;
}

ClientMount *findMount(const char *alias) {
    ClientMount *curr = m_head;
    while(curr != NULL) {
        if (strcmp(curr->foldername, alias)==0)
            return curr;

        curr = curr->next;
    }

    return NULL;
}

int addMount(const char *ip, const int port, const char *localFolderName) {
    // Initialize the ClientMount type
    ClientMount *new_mount = (ClientMount *)malloc(sizeof(ClientMount));

    strcpy(new_mount->ipOrDomName, ip);
    new_mount->port = port;
    strcpy(new_mount->foldername, localFolderName);

    new_mount -> next = m_head;
    m_head = new_mount;

    return 0;
}

int removeMount(const char *localFolderName) {
    ClientMount *curr = m_head;
    ClientMount *prev = m_head;

    int removed = -1;
    while(curr != NULL) {
        if (strcmp(curr->foldername, localFolderName) == 0) {
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

int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
    char *interfaceip = obtaininterfaceip("wlan0");
    printf("interfaceip: %s\n", interfaceip);

    return_type ans = make_remote_call(srvIpOrDomName,
        srvPort, "sMount", 2, strlen(interfaceip)+1, (void *) interfaceip, strlen(localFolderName)+1, (void *)localFolderName);

    printf("finished making rpc\n");

    if(*(int *)ans.return_val == 0){
        free(ans.return_val);
        return addMount(srvIpOrDomName, srvPort, localFolderName);
    }
    
    return -1;
}

int fsUnmount(const char *localFolderName) {
    // tell the server that the client will be unmounted
    // remove the list of active clients on the server
    // Remove the hash table relation

    char *interfaceip = obtaininterfaceip("wlan0");
    ClientMount *found = findMount(localFolderName);
    if(debug)printf("fsUnmount found : %s\n", found->ipOrDomName);
    if(debug)printf("fsUnmount found : %d\n", found->port);

    return_type ans = make_remote_call(found->ipOrDomName, found->port, "sUnmount", 2, strlen(interfaceip)+1,
        (void *)interfaceip, strlen(localFolderName)+1, (void *)localFolderName);

    if(debug)printf("fsUnmount ans : %p\n", ans.return_val);

    if(*(int *)ans.return_val == 0){
        free(ans.return_val);
        return removeMount(localFolderName);
    }
    
    return -1;
}

char *createFilepath(char *fullpath, char *alias) {
    int length = strlen(alias);
    char *ptr = (char *)fullpath;
    char *buff = calloc(256, sizeof(char));

    if(strcmp(fullpath, alias) == 0){
        char *test = calloc(1, sizeof(char));
        strcpy(test, ".");
        return test;
    }else{
        strcpy(buff, fullpath + length + 1);
    }
    return buff;
}

//Given folderPath
FSDIR* fsOpenDir(const char *folderName) {
    char *interfaceip = obtaininterfaceip("wlan0");
  
    char *alias = findRootName(folderName);
    char *folderpath = createFilepath((char *)folderName, alias);

    ClientMount *mounted = findMount(alias);
    
    return_type ans = make_remote_call(mounted->ipOrDomName, 
        mounted->port, "sOpenDir", 3, strlen(interfaceip)+ 1, (void *)interfaceip, strlen(alias)+1, (void *)alias, strlen(folderpath)+ 1, (void *) folderpath);


    if (*(int *)ans.return_val == 0) {
        if(debug)printf("in fsOpenDir: returning FSDIR\n");
        FSDIR *newfsdir = malloc(sizeof(FSDIR));
        strcpy(newfsdir->path, folderName);
        free(ans.return_val);
        return newfsdir;
    }
    if(debug)printf("in fsOpenDir: returning NULL\n");
    free(ans.return_val);
    return NULL;
}

int fsCloseDir(FSDIR *folder) {
    char *interfaceip = obtaininterfaceip("wlan0");   

    char *alias = findRootName(folder->path);

    ClientMount *mounted = findMount(alias);

    if (mounted == NULL){
        if(debug)printf("mounted is NULL\n");
        return -1;
    }


    char *folderpath = createFilepath((char *)folder->path, alias);
    printf("folderpath: %s\n", folderpath);

    // if(debug)printf("%p, %p, %p, %p", mounted, interfaceip, alias, folder->path);
    return_type ans = make_remote_call(mounted->ipOrDomName, mounted->port, "sCloseDir", 3, strlen(interfaceip)+1, (void *)interfaceip, strlen(alias)+1, (void *)alias,
        strlen(folderpath)+1, (void *)folderpath);

    // free(alias);

    if(*(int *)ans.return_val == 0){
        int val = *(int *)ans.return_val;
        free(ans.return_val);
        return val;
    }

    free(ans.return_val);
    return -1;
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    char *interfaceip = obtaininterfaceip("wlan0");   
    char *alias = findRootName(folder->path);
    char *folderpath = createFilepath(folder->path, alias);
    ClientMount *mounted = findMount(alias);

    if (mounted == NULL) {
        if(debug)printf("in fsReadDir: mounted is NULL\n");
        return NULL;
    }

    return_type ans = make_remote_call(mounted->ipOrDomName, mounted->port, "sReadDir", 3, strlen(interfaceip)+1, (void *)interfaceip, strlen(alias)+1, (void *)alias,
        strlen(folderpath)+1, (void *)folderpath);

    if (ans.return_val != NULL) {
        struct fsDirent *fsdirent = malloc(sizeof(struct fsDirent));
        memcpy(fsdirent, ans.return_val, ans.return_size);
        free(ans.return_val);
        return fsdirent;
    }
    if(debug)printf("fsReadDir returned NULL\n");
    free(ans.return_val);
    return NULL;
}

int addOpenFile(int fd, char *alias) {
    OpenFile *openfile = malloc(sizeof(OpenFile));
    openfile->fd = fd;
    strcpy(openfile->alias, alias);

    openfile->next = of_head;
    of_head = openfile;
}

OpenFile *findOpenFile(int fd) {
    OpenFile *curr = of_head;

    while (curr != NULL) {
        if (curr->fd == fd)
            return curr;
        curr = curr->next;
    }

    return NULL;
}

int fsOpen(const char *fname, int mode) {
    // ip, filepath, mode
    // rpc call fsOpen(fname, mode);
    char *interfaceip = obtaininterfaceip("wlan0");
    char *alias = findRootName(fname);
    char *filepath = createFilepath((char *)fname, alias);
    ClientMount *mounted = findMount(alias);
    if (mounted == NULL) {
        if(debug)printf("current ClientMount is null\n");
        return -1;
    }

    // free(alias);
    while (1) {
        return_type ans = make_remote_call(mounted->ipOrDomName, mounted->port, "sOpen", 
            4, strlen(interfaceip)+1, interfaceip, strlen(alias)+1, (void *)alias, 
            strlen(filepath)+1, (void *)filepath, sizeof(int), &mode);

        if(debug)printf("return val is %d\n", *(int *)ans.return_val);
        if(*(int *)ans.return_val > 0) {
            addOpenFile(*(int *)ans.return_val, alias);
            int val = *(int *)ans.return_val;
            free(ans.return_val);
            if(debug)printf("this is a test, the return_val is %d\n", val);
            return val;
        }else if(*(int *)ans.return_val == 0){
            if(debug)printf("cannot open file!\n");
            free(ans.return_val);
            return -1;
        }

        sleep(1);
    }
}

int fsClose(int fd) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);

    if(of == NULL){
        if(debug)printf("file is not open\n");
        return -1;
    }

    ClientMount *current_mount = findMount(of->alias);

    if(current_mount == NULL){
        if(debug)printf("user is not mounted\n");
        return -1;
    }

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sClose", 3, strlen(interfaceip)+1, interfaceip,
        strlen(of->alias)+1, (void *)of->alias, sizeof(int), &fd);

    if(*(int *)ans.return_val == 0){
        return 0;
    }
    
    return -1;
}

int fsRead(int fd, void *buf, const unsigned int count) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *alias = findRootName(of->alias);
    ClientMount *current_mount = findMount(alias);

    if(current_mount == NULL){
        if(debug)printf("mount in fsRead is null\n");
        return -1;
    }

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sRead", 4, strlen(interfaceip)+1, interfaceip,
        sizeof(int), &fd, sizeof(int), &count, strlen(alias)+1, (void *) alias);

    if(ans.return_val != NULL){
        memset(buf, 0, count);
        memcpy(buf, ans.return_val, ans.return_size);
        free(ans.return_val);
        return ans.return_size;
    }else{
        if(debug)printf("could not read\n");
        free(ans.return_val);
        return -1;
    }
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    char *interfaceip = obtaininterfaceip("wlan0");
    OpenFile *of = findOpenFile(fd);
    char *alias = findRootName(of->alias);
    ClientMount *current_mount = findMount(alias);

    if(current_mount == NULL){
        return -1;
    }

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sWrite", 5, strlen(interfaceip)+1, (void *)interfaceip,
        sizeof(int), &fd, count, buf, sizeof(int), &count, strlen(alias)+1, alias);
    if(*(int *)ans.return_val > 0){
        int val = *(int *)ans.return_val;
        free(ans.return_val);
        if(debug)printf("bytes written in fsWrite is %d\n", val);
        return val;
    }else{
        free(ans.return_val);
        return -1;    
    }
}

int fsRemove(const char *name) {
    char *interfaceip = obtaininterfaceip("wlan0");
    char *alias = findRootName(name);
    char *filepath = createFilepath((char *)name, alias);
    ClientMount *current_mount = findMount(alias);
    ClientMount *mounted = findMount(alias);
    if (mounted == NULL) {
        if(debug)printf("mounted in fsRemove is null\n");
        return -1;
    }

    return_type ans = make_remote_call(current_mount->ipOrDomName, current_mount->port, "sRemove", 2, strlen(interfaceip)+1, (void *)interfaceip,
        strlen(filepath)+1, filepath);

    if(debug)printf("return val in fsRemove of %d\n", *(int *)ans.return_val);
    if (*(int *)ans.return_val == 0) {
        free(ans.return_val);
        return 0;
    }else{
        return -1;
    }
}
