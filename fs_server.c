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
    char ip[256];
    struct OpenedFile *next;
} OpenedFile;

OpenedFile *op_head = NULL;
OpenedFile *op_tail = NULL;

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
    OpenedFolder *curr = user->opendirs;
    while(curr != NULL){
        if(debug)printf("curr path: %s\n", curr->path);
        if(debug)printf("fullpath: %s\n", path);
        if (strcmp(curr->path, path) == 0) {
            if(debug)printf("returning the openedfolder\n");
            return curr;
        }
        if(debug)printf("moving onto the next folder\n");
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

int authorize_file(int user_fd) {
	OpenedFile *curr = op_head;

	while (curr != NULL) {
		if (curr->fd == user_fd) {
			return 1;
		}
		curr = curr->next;
	}

	return 0;
}

void returnSignature(int err, void* ret_val, int ret_size) {
    void *ret_buf = (void *)malloc(sizeof(int)+ret_size);
    memcpy(ret_buf, &err, sizeof(int));
    memcpy(ret_buf+sizeof(int), ret_val, ret_size);
    r.return_size = ret_size + sizeof(int);
    r.return_val = ret_buf;
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
return_type sMount(const int nparams, arg_type* a)
{	
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
return_type sUnmount(const int nparams, arg_type* a)
{

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

    if (mounted == NULL){
        return createReturn(-1);
    }

    if (findOpenFolder(mounted, new_path) != NULL){
        return createReturn(-1);
    }

    DIR *d = NULL;
    d = opendir(new_path);
    
    if(d == NULL){
        return createReturn(-1);
    }

    // if(debug)printf("trying to add the open folder\n");
    if (addOpenFolder(mounted, new_path, d) != 0) {
        closedir(d);
        return createReturn(-1);
    }

    // if(debug)printf("weird stuff\n");
    OpenedFolder *new_open = findOpenFolder(mounted, new_path);
    new_open->storedDir = d;

    if(debug)printf("return from sOpenDir\n");
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


// 	if (authenticate(user_ip) == 0) {return error_val;}

//     char *filepath = a->next->arg_val;

//     MountedUser *current = m_head;
//     while(current != NULL){
//         if(current->ip == user_ip){
//             FSDIR *directory = current->opendirs;
//             FSDIR *dir_prev = NULL;
//             while(directory->path != NULL){
//                 if(directory->path == filepath){
//                     //try closing the dir
//                     int err = closedir(directory->storedDir);
//                     if(err == -1){
//                         return error_val;
//                     }

//                     if(dir_prev == NULL){
//                         FSDIR *old_root = directory;
//                         current->opendirs = directory->next;

//                         // free(old_root);
//                     }else{
//                         dir_prev->next = directory->next;
//                         // free(directory);
//                     }

//                     int zero = 0;
//                     r.return_val = (void *)&zero;
//                     r.return_size = sizeof(int);
//                     return r;
//                 }
//                 dir_prev = directory;
//                 directory = directory->next;
//             }
//         }
//         current = current->next;
//     }
//     return error_val;
}

return_type sReadDir(const int nparams, arg_type* a){
    if(nparams != 3){
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }
    
    char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;
	char *filepath = (char *)a->next->next->arg_val;

    char *rootname = findRootName(filepath);
    MountedUser *mounted = findMount(user_ip, alias);
    OpenedFolder *openfolder = findOpenFolder(mounted, prependRootName(filepath));

    struct dirent *readDirectory = readdir(openfolder->storedDir);

    if(readDirectory == NULL){
        if(debug)printf("in sReadDir: reading the directory is NULL\n");
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }   

    struct fsDirent *fsdir = (struct fsDirent*)malloc(sizeof(struct fsDirent));

    if(readDirectory->d_type == DT_DIR){
        if(debug)printf("type: folder\n");
         fsdir->entType = 1;
    }else if(readDirectory->d_type == DT_REG){
        if(debug)printf("type: file\n");
        fsdir->entType = 0;
    }else{
        if(debug)printf("type: unknown\n");
        fsdir->entType = -1;
    }

    strcpy(fsdir->entName, readDirectory->d_name);
    if(debug)printf("fsdir->entName = %s\n", fsdir->entName);
    if(debug)printf("fsdir->entType = %d\n", fsdir->entType);
    r.return_val = fsdir;
    r.return_size = sizeof(struct fsDirent);
    return r;
}

//params: user_ip -> filepath -> mode
return_type sOpen(const int nparams, arg_type* a){
	if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}
    if(debug)printf("sopen is being called\n");

	char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

	char *filepath = (char *)a->next->arg_val;
	int mode = *(int *)a->next->next->arg_val;

	struct stat buffer;

	int err = stat(filepath, &buffer);
	if (err == -1 && mode == 0) {
        return error_val;
    }
	if (S_ISDIR(buffer.st_mode)) {
        return error_val;
    }

	int fd;
	if (mode == 1) {
		fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, S_IRUSR | S_IWUSR);
	}else {
		fd = open(filepath, O_RDONLY | S_IRUSR);
	}

    OpenedFile *op_current = op_head;
    while(op_current != NULL){
        if(op_current->fd == fd){
            return error_val;
        }
    }

	if(op_head == NULL){
        op_head = (OpenedFile *)malloc(sizeof(OpenedFile));
        op_head->fd = fd;
        strcpy(op_head->ip, user_ip);
        op_tail = op_head;
	}else{
        OpenedFile *op_newFile = (OpenedFile *)malloc(sizeof(OpenedFile));
        op_newFile->fd = fd;
        strcpy(op_newFile->ip, user_ip);
        op_tail->next = op_newFile;
        op_tail = op_tail->next;
	}

	r.return_val = (void *)&fd;
	r.return_size = sizeof(int);
	return r;
}

//nparams: user_ip -> fd
return_type sClose(int nparams, arg_type* a) {
	if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
    int fd = *(int *)a->next->arg_val;

	if (authenticate(user_ip) == 0) {
		return error_val;
	}

    OpenedFile *op_current = op_head;
    OpenedFile *op_prev = op_current;
    while(op_current != NULL){
        if(op_current->fd == fd) {
            if (op_current->ip == user_ip){
                if(op_current == op_head){
                    OpenedFile *op_newHead = op_head->next;
                    // free(op_head);
                    op_head = op_newHead;
                }else{
                    OpenedFile *op_newCurrent = op_current->next;
                    // free(op_current);
                    op_prev->next = op_newCurrent;
                }
                int status = close(fd);
                r.return_val = (void *)&status;
                r.return_size = sizeof(int);
                return r;
            }
            else {
                return error_val;
            }
        }
        if(op_current != op_head){
            op_prev = op_prev->next;
        }
        op_current = op_current->next;
    }

    return error_val;
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
    register_procedure("sReadDir", 3, sReadDir);
    register_procedure("sOpen", 3, sOpen);
    register_procedure("sClose", 2, sClose);
    register_procedure("sRead", 4, sRead);
    register_procedure("sWrite", 4, sWrite);
    register_procedure("sRemove", 2, sRemove);

    strcpy(serverAlias, argv[1]);
    launch_server();
    return 0;
}
