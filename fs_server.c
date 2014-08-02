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

int checkFileInUse(char *fullpath) {
	OpenedFile *curr = op_head;

	while (curr != NULL) {
        if (strcmp(curr->path, fullpath) == 0){
			return 1;
        }
		curr = curr->next;
	}

	return 0;
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

return_type createReturn(int error_num, int value) {    
    int val = value;
    int err = error_num;
    void *buff = malloc(sizeof(int) * 2);
    void *ptr = buff;
    memcpy(ptr, &err, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, &val, sizeof(int));

    r.return_size = sizeof(int) * 2;
    r.return_val = buff;
    return r;
}

//nparams: user_ip
return_type sMount(const int nparams, arg_type* a) {	
 //    if (nparams != 2) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

    char *user_ip = (char *)a->arg_val;
    char *alias  = (char *)a->next->arg_val;

    MountedUser *mounted = findMount(user_ip, alias);
    if(debug)printf("Mounted address: %p\n", mounted);

    if(debug)printMount();
    if (mounted != NULL) {
        if(debug) printf("Mounted is not NULL\n");
        return createReturn(13, -1);
    }

    addMount(user_ip, alias);

    return createReturn(0, 0);
}

//nparams: user_ip
return_type sUnmount(const int nparams, arg_type* a) {

    if(debug)printf("recevied %d nparams in sUnmount\n", nparams);
	// if (nparams != 2) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

	char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;

    MountedUser *found = findMount(user_ip, alias);

    if (found == NULL) {
        if(debug)printf("found is null in sUnmount\n");
        return createReturn(1, -1);
    }

    if (removeMount(user_ip, alias) != 0) {
        if(debug)printf("problem with removeMount\n");
        return createReturn(1, -1);
    }
    if(debug)printf("returning a good value from sUnmount\n");
    return createReturn(0, 0);
}

//nparams: user_ip -> filepath
return_type sOpenDir(int nparams, arg_type* a) {
 //    if (nparams != 3) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

    char *user_ip = (char *)(a->arg_val);
    char *alias = (char *)(a->next->arg_val);
    char *path = (char *)(a->next->next->arg_val);

    char new_path[256];
    strcpy(new_path, prependRootName(path));
    MountedUser *mounted = findMount(user_ip, alias);

    if (mounted == NULL){
        printf("exit 5\n");
        return createReturn(1, -1);
    }

    if (findOpenFolder(mounted, new_path) != NULL){
        printf("exit 4\n");
        return createReturn(2, -1);
    }

    DIR *d = NULL;
    d = opendir(new_path);
    
    if(d == NULL){
        printf("exit 1\n");
        return createReturn(errno, -1);
    }

    // if(debug)printf("trying to add the open folder\n");
    if (addOpenFolder(mounted, new_path, d) != 0) {
        closedir(d);
        printf("exit 2\n");
        return createReturn(1, -1);
    }

    // if(debug)printf("weird stuff\n");
    OpenedFolder *new_open = findOpenFolder(mounted, new_path);
    new_open->storedDir = d;

    if(debug)printf("return from sOpenDir\n");
    printf("exit 3\n");
    return createReturn(0, 0);
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
    
 //    if (nparams != 3) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

	char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;
    char *path = (char *)a->next->next->arg_val;

    char *fullpath = malloc(sizeof(char) *256);

    strcpy(fullpath, prependRootName(path));

    if(debug)printf("fullpath is %s\n", fullpath);

    MountedUser *mounted = findMount(user_ip, alias);

    if (mounted == NULL) {
        if(debug)printf("Mounted is NULL\n");
        return createReturn(2, -1);
    }

    OpenedFolder *openfolder = findOpenFolder(mounted, fullpath);

    
    if (openfolder == NULL){
        if(debug)printf("open folder is not open\n");
        return createReturn(2, -1);
    }

    if (removeOpenFolder(mounted, fullpath) != 0) {
        if(debug)printf("remove folder not working\n");
        return createReturn(2, -1);
    }

    return createReturn(0, 0);

}

return_type sReadDir(const int nparams, arg_type* a){
    // if(nparams != 3){
    //     r.return_val = NULL;
    //     r.return_size = 0;
    //     return r;
    // }
    
    char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;
	char *filepath = (char *)a->next->next->arg_val;

    char *rootname = findRootName(filepath);
    MountedUser *mounted = findMount(user_ip, alias);

    if(mounted == NULL){
        return createReturn(1, -1);
    }

    OpenedFolder *openfolder = findOpenFolder(mounted, prependRootName(filepath));

    struct dirent *readDirectory = readdir(openfolder->storedDir);


    if(readDirectory == NULL){
        if(debug)printf("in sReadDir: reading the directory is NULL\n");
        return createReturn(1, -1);
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
    
    void *returnBuff = malloc(sizeof(struct fsDirent)+sizeof(int));
    void *ptr = returnBuff;

    int val = 0;
    void *buff = malloc(sizeof(int));
    memcpy(buff, &val, sizeof(int));
    
    memcpy(ptr, buff, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, fsdir, sizeof(struct fsDirent));
    r.return_val = returnBuff;
    r.return_size = sizeof(struct fsDirent) + sizeof(int);

    return r;
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

OpenedFile *findOpenFile(char *user_ip, char *alias, char *fullpath) {
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
	// if (nparams != 4) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

	char *user_ip = (char *)a->arg_val;
    char *alias = (char *)a->next->arg_val;
    char *filepath = (char *)a->next->next->arg_val;
    char *fullpath = prependRootName(filepath);
	int mode = *(int *)a->next->next->next->arg_val;
    if(debug)printf("fullpath on server: %s\n", fullpath);

    if (checkFileInUse(fullpath) == 1){
        if(debug)printf("file in use");
        return createReturn(26, -1);
    }

    // Check if user is mounted
    MountedUser *mounted = findMount(user_ip, alias);
    if (mounted == NULL){
        return createReturn(1, -1);
    }

    // If the file does not exist or is a folder return error
    struct stat buffer;
	int err = stat(fullpath, &buffer);
    if(debug)printf("errval: %d\n", err);
	if (err == -1 && mode == 0){
        if(debug)printf("cannot open in read mode\n");
        return createReturn(err, 0);
    }

	if (S_ISDIR(buffer.st_mode)){
        if(debug)printf("wtf its a folder\n");
        return createReturn(21, 0);
    }

	int fd;
	if (mode == 1) {
		fd = open(fullpath, O_CREAT | O_RDWR | O_APPEND | O_TRUNC, S_IRUSR | S_IWUSR);
	}else {
		fd = open(fullpath, O_RDONLY | S_IRUSR);
	}

    if(debug)printf("the fd is %d\n", fd);
    // Can just add struct because we know it doesn't exist from checkFileInUse
	if (addOpenFile(user_ip, alias, fd, fullpath) != 0){
        if(debug)printf("closing the file descriptor %d because of error\n", fd);
        close(fd);
        return createReturn(1, -1);
    }
	return createReturn(0, fd);
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
	// if (nparams != 3) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

    char *user_ip = (char *)a->arg_val;
	char *alias = (char *)a->next->arg_val;
    int fd = *(int *)a->next->next->arg_val;

    // Check if user is mounted
    MountedUser *mounted = findMount(user_ip, alias);
    if (mounted == NULL){
        return createReturn(1, -1);
    }

    if (removeOpenFile(user_ip, alias, fd) != 0){
        return createReturn(1, -1);
    }

    return createReturn(0, 0);
}

//nparams: user_ip -> fd -> buf -> count -> alias
return_type sRead(int nparams, arg_type* a) {
 //    if (nparams != 4) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

    char *user_ip = (char *)a->arg_val;
	int fd = *(int *)a->next->arg_val;
	int count = *(int *)a->next->next->arg_val;
    char *alias = (char *)a->next->next->next->arg_val;

    MountedUser *mounted = findMount(user_ip, alias);
    if (mounted == NULL){
        return createReturn(1, -1);
    }

    int bytesRead = -1;

    char *buff = malloc(count);
    memset(buff, 0, count);
    bytesRead = read(fd, buff, count);
    if(debug)printf("buffer contained: %s\n", buff);
    if(debug)printf("bytesread is %d, count is %d\n", bytesRead, count);
    
    if(debug)printf("sending back the buffer %s\n", buff);

    if(bytesRead > 0){
        void *returnBuff = malloc(bytesRead+sizeof(int));

        int val = 0;
        void *temp = malloc(sizeof(int));
        memcpy(temp, &val, sizeof(int));

        void *ptr = returnBuff;
        memcpy(ptr, temp, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, buff, bytesRead);
        r.return_val = returnBuff;
        r.return_size = bytesRead + sizeof(int);
        return r;
    }else{
        r.return_val = &errno;
        r.return_size = sizeof(int);
        return r;
    }
}

return_type sWrite(int nparams, arg_type* a){
    if(debug)printf("got into sWrite\n");
 //    if (nparams != 5) {
	// 	r.return_val = NULL;
	// 	r.return_size = 0;
	// 	return r;
	// }

    char *user_ip = (char *)a->arg_val;
	int fd = *(int *)a->next->arg_val;
	void *buff = a->next->next->arg_val;
	int count = *(unsigned int *)a->next->next->next->arg_val;
    char *alias = a->next->next->next->next->arg_val;

    MountedUser *mounted = findMount(user_ip, alias);
    if (mounted == NULL){
        return createReturn(1, -1);
    }
	
    int bytesWritten = -1;
    bytesWritten = write(fd, buff, count);

    if(bytesWritten > 0){
        return createReturn(0, bytesWritten);
    }else{
        return createReturn(errno, -1);
    }
}

return_type sRemove(int nparams, arg_type* a){
    // if (nparams != 2) {
    //     r.return_val = NULL;
    //     r.return_size = 0;
    // }

    char *user_ip = a->arg_val;
    char *filepath = a->next->arg_val; 
    char *fullpath = prependRootName(filepath);   

    int stat = remove(fullpath);

    if (stat == 0) {
        return createReturn(0, 0);
    }else{
        return createReturn(errno, -1);
    }
}

int main(int argc, char *argv[]) {
    register_procedure("sMount", 2, sMount);
    register_procedure("sUnmount", 2, sUnmount);
    register_procedure("sOpenDir", 3, sOpenDir);
    register_procedure("sCloseDir", 3, sCloseDir);
    register_procedure("sReadDir", 3, sReadDir);
    register_procedure("sOpen", 4, sOpen);
    register_procedure("sClose", 3, sClose);
    register_procedure("sRead", 4, sRead);
    register_procedure("sWrite", 5, sWrite);
    register_procedure("sRemove", 2, sRemove);

    strcpy(serverAlias, argv[1]);
    launch_server();
    return 0;
}
