#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "ece454rpc_types.h"
#include "ece454_fs.h"
//#include "simplified_rpc/server_stub.c"
bool debug = true;
//should have linked list for mounted and opened files
typedef struct MountedUser {
  char ip [256];
  char alias[256];
  struct OpenedFolder *opendirs;
  struct MountedUser *next;
} MountedUser;

MountedUser *m_head = NULL;

typedef struct OpenedFile {
    int fd;
    char path[256];
    char ip[256];
    char alias[256];
    struct OpenedFile *next;
} OpenedFile;

OpenedFile *op_head = NULL;

char serverAlias[256];

typedef struct OpenedFolder {
    char path[256];
    DIR *storedDir;
    struct OpenedFolder *next;
} OpenedFolder;

OpenedFolder *of_head = NULL;

return_type r;
return_type error_val;

MountedUser *findMount(char *ip, char *alias) {
    MountedUser *curr = m_head;

    while(curr != NULL){
        if(strcmp(curr->ip, ip) == 0 && strcmp(curr->alias, alias) == 0 ){
            if(debug)printf("the alias is %s\n", alias);
            if(debug)printf("the ip is %s\n", ip);
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

int removeMount(char *ip, char *alias) {
    MountedUser *curr = m_head;
    MountedUser *m_prev = m_head;

    while(curr != NULL){
        if(strcmp(curr->ip, ip) == 0 && strcmp(curr->alias, alias) == 0){
            if(curr == m_head){
                m_head = m_head->next;
            }else{
                m_prev->next = curr->next;
            }
            free(curr);
            return 0;
        }
        if(m_prev != m_head){
            m_prev = m_prev->next;
        }
        curr = curr->next;

    }

    return -1;
}

char *prependRootName(char *filepath){
    char *buff = malloc(sizeof(char)*256);
    sprintf(buff, "%s/%s", serverAlias, filepath);
    return buff;
}

OpenedFolder *findOpenFolder(MountedUser *user, char *path){
    printf("getting here\n");
    OpenedFolder *curr = user->opendirs;
    while(curr != NULL){
        printf("infinite loop\n");
        if (strcmp(curr->path, path) == 0) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

int addOpenFolder(MountedUser *user, char *path, DIR *storedDir){
    OpenedFolder *new_of = malloc(sizeof(OpenedFolder));

    strcpy(new_of->path, path);
    new_of->storedDir = storedDir;

    OpenedFolder *curr = user->opendirs;
    new_of->next=curr;
    user->opendirs = new_of;

    return 0;
}


void printMount() {
    MountedUser *curr = m_head;

    while (curr != NULL) {
        printf("-> %s ", curr->alias);
        curr = curr->next;
    }

    printf("\n");
}


int addMount(char *ip, char *alias) {
    if(debug) printf("Adding Mount IP: %s\n", ip);
    if(debug) printf("Adding Mount alias: %s\n", alias);
    MountedUser *new_mount = malloc(sizeof(MountedUser));
    strcpy(new_mount->ip, ip);
    strcpy(new_mount->alias, alias);
    new_mount->opendirs = NULL;
    new_mount -> next = m_head;
    m_head = new_mount;
    return 0;
}


int authenticate(char *user_ip) {
	   
    if(debug)printf("authencating\n");
    MountedUser *curr = m_head;

    while (curr != NULL) {
        if (strcmp(curr->ip, user_ip) == 0) {
            return 1;
        }
        curr = curr->next;
    }

    if(debug)printf("done authenticating\n");
    return 0;
}

int checkFileInUse(char *fullpath) {
	OpenedFile *curr = op_head;

	while (curr != NULL) {
		if (strcmp(curr->path == fullpath) == 0)
			return 0;
		curr = curr->next;
	}

	return -1;
}

char *findRootName(const char *fullpath) {
    char find = '/';

    const char *ptr = strchr(fullpath, find);
    int index;
    if(ptr) {
       index = ptr - fullpath;
    }
    else {
        if(debug)printf("returning the fullpath from findRootName\n");
        return (void*)fullpath;
    }

    char *localFolderName = malloc(index);
    strncpy(localFolderName, fullpath, index);
    return localFolderName;
}

return_type createReturn(int value) {    
    int val = value;
    void *buff = malloc(sizeof(int));
    memcpy(buff, &val, sizeof(int));

    r.return_size = sizeof(int);
    r.return_val = buff;
    return r;
}

//nparams: user_ip
return_type sMount(const int nparams, arg_type* a) {	
    if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

    char *user_ip = (char *)a->arg_val;
    char *alias  = (char *)a->next->arg_val;

    MountedUser *mounted = findMount(user_ip, alias);
    if(debug)printf("Mounted address: %p\n", mounted);

    if(debug)printMount();
    if (mounted != NULL) {
        if(debug) printf("Mounted is not NULL\n");
        return createReturn(-1);
    }

    addMount(user_ip, alias);

    return createReturn(0);
}

//nparams: user_ip
return_type sUnmount(const int nparams, arg_type* a) {

    if(debug)printf("recevied %d nparams in sUnmount\n", nparams);
	if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;

    MountedUser *found = findMount(user_ip, alias);

    if (found == NULL) {
        if(debug)printf("found is null in sUnmount\n");
        return createReturn(-1);
    }

    if (removeMount(user_ip, alias) != 0) {
        if(debug)printf("problem with removeMount\n");
        return createReturn(-1);
    }
    if(debug)printf("returning a good value from sUnmount\n");
    return createReturn(0);
}

//nparams: user_ip -> filepath
return_type sOpenDir(int nparams, arg_type* a) {
    if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

    char *user_ip = (char *)(a->arg_val);
    char *alias = (char *)(a->next->arg_val);
    char *path = (char *)(a->next->next->arg_val);

    char new_path[256];
    strcpy(new_path, prependRootName(path));
    MountedUser *mounted = findMount(user_ip, alias);

    if(debug)printf("checking if mount is null\n");
    if (mounted == NULL)
        return createReturn(-1);

    if(debug)printf("trying to find the open folder\n");
    if (findOpenFolder(mounted, new_path) != NULL)
        return createReturn(-1);
    

    DIR *d = NULL;
    d = opendir(new_path);
    if(debug)printf("the value of newpath is %s\n", new_path);

    if(debug)printf("checking the D\n");
    if(d == NULL)
        return createReturn(-1);

    // if(debug)printf("trying to add the open folder\n");
    if (addOpenFolder(mounted, new_path, d) != 0) {
        closedir(d);
        return createReturn(-1);
    }

    // if(debug)printf("weird stuff\n");
    OpenedFolder *new_open = findOpenFolder(mounted, new_path);
    new_open->storedDir = d;

    return createReturn(0);
}


int removeOpenFolder(MountedUser *mount, char *fullpath) {
    
    OpenedFolder *curr = mount->opendirs;
    OpenedFolder *prev = mount->opendirs;

    while (curr != NULL) {
        if (strcmp(fullpath, curr->path) == 0) {
            if (prev == curr) {
                mount->opendirs = NULL;
            }
            else {
                prev->next = curr->next;
            }

            free(curr);

            return 0;
        }

        if (prev != curr)
            prev = prev -> next;

        curr = curr->next;
    }

    return -1;
}
//params: filepath
return_type sCloseDir(int nparams, arg_type* a){
    
    if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;
    char *path = (char *)a->next->next->arg_val;

    char *fullpath = malloc(sizeof(char) *256);

    strcpy(fullpath, prependRootName(path));

    if(debug)printf("fullpath is %s\n", fullpath);

    MountedUser *mounted = findMount(user_ip, alias);

    if (mounted == NULL) {
        if(debug)printf("Mounted is NULL\n");
        return createReturn(-1);
    }

    OpenedFolder *openfolder = findOpenFolder(mounted, fullpath);

    
    if (openfolder == NULL){
        if(debug)printf("open folder is not open\n");
        return createReturn(-1);
    }

    if (removeOpenFolder(mounted, fullpath) != 0) {
        if(debug)printf("remove folder not working\n");
        return createReturn(-1);
    }

    return createReturn(0);

}

return_type sReadDir(const int nparams, arg_type* a){
//     if(debug)printf("sReadDir was called with %d arguments\n", nparams);
//     if(nparams != 2){
//         r.return_val = NULL;
//         r.return_size = 0;
//         return r;
//     }
//     if(debug)printf("this is the start of readdir\n");
//     char *user_ip = (char *)a->arg_val;
// 	if (authenticate(user_ip) == 0) {return error_val;}

// 	char *filepath = (char *)a->next->arg_val;

//     char *rootname = findRootName(filepath);
//     if(debug)printf("the rootname is %s\n", rootname);
//     MountedUser *mounted = findMount(rootname, "test");
//     FSDIR* dir = findFSDIR(mounted,filepath);
//     if(debug)printf("found the fsdir\n");
//     struct dirent *readDirectory = readdir(dir->storedDir);
//     if(debug)printf("after read\n");

//     if(readDirectory == NULL){
//         return error_val;
//     }   

//     if(debug)printf("the filename that was read is %s\n", readDirectory->d_name);

//     struct fsDirent *fsdir = (struct fsDirent*)malloc(sizeof(struct fsDirent));

//     if(readDirectory->d_type == DT_DIR){
//         if(debug)printf("the type of the fie is folder\n");
//          fsdir->entType = 1;
//     }else if(readDirectory->d_type == DT_REG){
//         if(debug)printf("type of bug is file\n");
//         fsdir->entType = 0;
//     }else{
//         if(debug)printf("type of file is unknown\n");
//         fsdir->entType = -1;
//     }

//     strcpy(fsdir->entName, readDirectory->d_name);

//     r.return_val = fsdir;
//     r.return_size = sizeof(fsdir);
//     if(debug)printf("this is probably returning the correct thing\n");
//     return r;
}

int addOpenFile(char *user_ip, char *alias, int fd, char *fullpath) {
    OpenedFile *openfile = malloc(sizeof(OpenedFile));
    strcpy(openfile->ip, user_ip);
    strcpy(openfile->alias, alias);
    openfile->fd = fd;
    strcpy(openfile->path, fullpath);
    openfile->next = op_head;

    op_head = openfile;
    return 0;
}

OpenedFile findOpenFile(char *user_ip, char *alias, char *fullpath) {
    OpenedFile *curr = op_head;

    while (curr != NULL) {
        if (strcmp(curr->ip, user_ip) == 0 && strcmp(curr->alias, alias) == 0
            && strcmp(curr->path, fullpath) == 0)
            return curr;

        curr = curr -> next;
    }

    return NULL;
}


//params: user_ip -> filepath -> mode
return_type sOpen(const int nparams, arg_type* a){
	if (nparams != 4) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;
    char *filepath = (char *)a->next->next->arg_val;
    char *fullpath = prependRootName(filepath);
	int mode = *(int *)a->next->next->next->arg_val;

    if (checkFileInUse(fullpath)) == 0)
        return createReturn(-1);

    // Check if user is mounted
    MountedUser *mounted = findMount(user_ip, alias);
    if (mounted == NULL)
        return createReturn(-1);

    // If the file does not exist or is a folder return error
    struct stat buffer;
	int err = stat(filepath, &buffer);
	if (err == -1 && mode == 0)
        return createReturn(-1);

	if (S_ISDIR(buffer.st_mode))
        return createReturn(-1);

	int fd;
	if (mode == 1) {
		fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, S_IRUSR | S_IWUSR);
	}else {
		fd = open(filepath, O_RDONLY | S_IRUSR);
	}

    // Can just add struct because we know it doesn't exist from checkFileInUse
	if (addOpenFile(user_ip, alias, fd, fullpath) != 0)
        close(fd);
        return createReturn(-1);

	return createReturn(fd);
}

int removeOpenFile(char *user_ip, char *alias, int fd) {
    OpenedFile *curr = op_head;
    OpenedFile *prev = op_head;

    while(curr != NULL){
        if(strcmp(curr->ip, user_ip) == 0 && strcmp(curr->alias, alias) == 0 && curr->fd == fd){
            if(curr == op_head){
                op_head = curr->next;
            }else{
                prev->next = curr->next;
            }
            free(curr);
            return 0;
        }

        if(curr != op_head){
            prev = prev->next;
        }

        curr = curr->next;
    }

    return -1;
}

//nparams: user_ip -> fd
return_type sClose(int nparams, arg_type* a) {
	if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

    char *user_ip = (char *)a->arg_val;
	char *alias = (char *)a->next->arg_val;
    int fd = *(int *)a->next->next->arg_val;

    // Check if user is mounted
    MountedUser *mounted = findMount(user_ip, alias);
    if (mounted == NULL)
        return createReturn(-1);

    if (removeOpenFile(user_ip, alias, fd) != 0)
        return createReturn(-1);

    return createReturn(0);
}

//nparams: user_ip -> fd -> buf -> count
return_type sRead(int nparams, arg_type* a) {
    if (nparams != 4) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

    char *user_ip = (char *)a->arg_val;
	int fd = *(int *)a->next->arg_val;
	int bufsize = *(int *)a->next->next->arg_val;
	int count = *(int *)a->next->next->next->arg_val;

    if (authenticate(user_ip) == 0) {
		return error_val;
	}

    int bytesRead = -1;
    char buff[bufsize];
    void *ret_buff = malloc(bufsize + sizeof(int));
    void *ptr = ret_buff;

    OpenedFile *op_current = op_head;
    while(op_current != NULL){
        if (op_current->fd == fd) {
            if (op_current->ip == user_ip) {
                bytesRead = read(fd, buff, count);
                memcpy(ptr, &bytesRead, sizeof(int));
                ptr += sizeof(int);
                memcpy(ptr, buff, sizeof(bufsize));

                r.return_val = ret_buff;
                r.return_size = sizeof(ret_buff);
                return r;
            }
            else {
                return error_val;
            }
        }

        op_current = op_current->next;
    }

    return error_val;
}

return_type sWrite(int nparams, arg_type* a){
    if (nparams != 4) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

    char *user_ip = (char *)a->arg_val;
    if (authenticate(user_ip) == 0) {return error_val;}

	int fd = *(int *)a->next->arg_val;
	int bufsize = *(int *)a->next->next->arg_val;
	int count = *(unsigned int *)a->next->next->next->arg_val;
	int bytesWritten = -1;

    char buff[bufsize];
    void *combined = malloc(bufsize + sizeof(int));
    void *ptr = combined;

    if (authenticate(user_ip) == 0) {
		return error_val;
	}

    OpenedFile *op_current = op_head;
    while(op_current != NULL){
        if(op_current->fd == fd){
            if (op_current->ip == user_ip){
                bytesWritten = write(fd, buff, count);
                memcpy(ptr, &bytesWritten, sizeof(int));
                ptr += sizeof(int);
                memcpy(ptr, &buff, bytesWritten);
                r.return_val = (void *)combined;
                r.return_size = sizeof(combined);
                return r;
            }
            else {
                return error_val;
            }
        }
        op_current = op_current->next;
    }

    return error_val;
}

return_type sRemove(int nparams, arg_type* a){
    if (nparams != 2) {
        r.return_val = NULL;
        r.return_size = 0;
    }

    char *user_ip = a->arg_val;
    char *filepath = a->next->arg_val;

    if (authenticate(user_ip) == 0) {
        return error_val;
    }

    int fd = open(filepath, O_RDONLY);

    OpenedFile *op_current = op_head;
    while (op_current != NULL) {
        if (op_current->fd == fd) {
            return error_val;
        }
        op_current = op_current->next;
    }

    int stat = remove(filepath);

    if (stat == 0) {
        r.return_val = 0;
        r.return_size = sizeof(int);
        return r;
    }

    return error_val;
}

int main(int argc, char *argv[]) {
    register_procedure("sMount", 2, sMount);
    register_procedure("sUnmount", 2, sUnmount);
    register_procedure("sOpenDir", 3, sOpenDir);
    register_procedure("sCloseDir", 3, sCloseDir);
    register_procedure("sReadDir", 2, sReadDir);
    register_procedure("sOpen", 4, sOpen);
    register_procedure("sClose", 3, sClose);
    register_procedure("sRead", 4, sRead);
    register_procedure("sWrite", 4, sWrite);
    register_procedure("sRemove", 2, sRemove);

    strcpy(serverAlias, argv[1]);
    launch_server();
    return 0;
}
