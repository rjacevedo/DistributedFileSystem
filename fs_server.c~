#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
//#include "simplified_rpc/server_stub.c"
#include "simplified_rpc/ece454rpc_types.h"

//should have linked list for mounted and opened files
typedef struct MountedUser {
  char *ip;
  char *alias;
  struct FSDIR *opendirs;
  struct MountedUser *next;
} MountedUser;

typedef struct FSDIR {
    char* clientString;
    DIR *storedDir;
    struct FSDIR *next;
} FSDIR;

typedef struct OpenedFile {
    int fd;
    char *ip;
    struct OpenedFile *next;
} OpenedFile;

OpenedFile *op_head = NULL;
OpenedFile *op_tail = NULL;
MountedUser *root = NULL;
MountedUser *tail = NULL;

return_type r;
return_type error_val = {(void *) -1, sizeof(int)};

int authenticate(char *user_ip) {
	MountedUser *curr = root;

	while (curr != NULL) {
		if (curr->ip == user_ip) {
			return 1;
		}
		curr = curr->next;
	}

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

    if(root == NULL){
        root = (MountedUser *)malloc(sizeof(MountedUser));
        root->ip = user_ip;
        root->alias = alias;
        tail = root;
    }else{
        MountedUser *new_MountedUser = (MountedUser *)malloc(sizeof(MountedUser));
        new_MountedUser->ip = user_ip;
        new_MountedUser->alias = alias;
        new_MountedUser->next = NULL;
        tail->next = new_MountedUser;
        tail = new_MountedUser;
    }

    r.return_size = sizeof(int);
	r.return_val = (void *) 0;

    return r;
}

//nparams: user_ip
return_type sUnmount(const int nparams, arg_type* a)
{
	if (nparams != 1) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;

	MountedUser *curr = root;
    MountedUser *prev = curr;

	r.return_size = sizeof(int);
	r.return_val = (void *) -1;

	while (curr != NULL) {
        if (curr->ip == user_ip) {
            if(curr == root){
                free(curr);
                root = NULL;
            }else{
                prev->next = curr->next;
                free(curr);
            }
            r.return_val = 0;
            return r;
        }

        if(curr != root){
            prev = prev->next;
        }
		curr = curr->next;
	}

    return error_val;
}

//nparams: user_ip -> filepath
return_type sOpenDir(int nparams, arg_type* a) {
    if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

    char *filepath = (char *)a->next->arg_val;

	DIR *d = NULL;
    struct dirent *dir;
    struct stat buffer;
    int err;

	err = stat(filepath, &buffer);
	if (err == -1) {
        r.return_val = (void *)NULL;
        r.return_size = 0;
        return r;
    }

    void *buf = malloc(1500);
    void *ptr = buf;
    ptr += sizeof(int);
    int length;
    int count = 0;
    d = opendir(filepath);
    if (d){
        while ((dir = readdir(d)) != NULL){
            length = strlen(dir->d_name);
            memcpy(ptr, &length, sizeof(int));
            ptr += sizeof(int);
            memcpy(ptr, dir->d_name, length);
            ptr += length;

            int concat_len = strlen(dir->d_name) + strlen(filepath);
            char concat_buf[concat_len];
            strcat(concat_buf, filepath);
            strcat(concat_buf, dir->d_name);
            err = stat(concat_buf, &buffer);
            int zero = 0;
            if(err == -1){
                memcpy(ptr, (void *)-1, sizeof(int));
            }else if(S_ISDIR(buffer.st_mode)){
                memcpy(ptr, (void *)1, sizeof(int));
            }else{
                memcpy(ptr, &zero, sizeof(int));
            }
            ptr += sizeof(int);

            count += 1;
        }
        memcpy(buf, &count, sizeof(int));

        //Confirm later
        MountedUser *current = root;
        while(current->ip != NULL){
            if(current->ip == user_ip){
                FSDIR *directory = current->opendirs;
                FSDIR *dir_tail = NULL;
                while(directory->storedDir != NULL){
                    dir_tail = directory;
                    if(directory->storedDir == d){
                        return error_val;
                    }
                    directory = directory->next;
                }
                FSDIR *dir_newTail = (FSDIR *)malloc(sizeof(FSDIR));
                dir_newTail->clientString = filepath;
                dir_newTail->storedDir = d;

                dir_tail->next = dir_newTail;
            }
            current = current->next;
        }
        closedir(d);
    }

    r.return_val = (void *)buf;
    r.return_size = sizeof(buf);
    return r;
}

//params: filepath
return_type sCloseDir(int nparams, arg_type* a){
    if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

    char *filepath = a->arg_val;

    MountedUser *current = root;
    while(current != NULL){
        if(current->ip == user_ip){
            FSDIR *directory = current->opendirs;
            FSDIR *dir_prev = NULL;
            while(directory->clientString != NULL){
                if(directory->clientString == filepath){
                    //try closing the dir
                    int err = closedir(directory->storedDir);
                    if(err == -1){
                        return error_val;
                    }

                    if(dir_prev == NULL){
                        FSDIR *old_root = directory;
                        current->opendirs = directory->next;

                        free(old_root);
                    }else{
                        dir_prev->next = directory->next;
                        free(directory);
                    }

                    int zero = 0;
                    r.return_val = (void *)&zero;
                    r.return_size = sizeof(int);
                    return r;
                }
                dir_prev = directory;
                directory = directory->next;
            }
        }
        current = current->next;
    }
    return error_val;
}

return_type sReadDir(const int nparams, arg_type* a){
    if(nparams != 2){
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

	char *filepath = (char *)a->next->arg_val;

    MountedUser *current = root;
    while(current != NULL){
        if(current->ip == user_ip){
            FSDIR *directory = current->opendirs;
            while(directory != NULL){
                if(directory->clientString == filepath){
                    struct dirent *readDirectory = readdir(directory->storedDir);

                    void *return_buffer = malloc(256);
                    void *return_ptr = return_buffer;
                    struct stat buffer;
                    int err = stat(filepath, &buffer);
                    if(err == -1){
                        return error_val;
                    }

                    memcpy(return_ptr, (void *)sizeof(int), sizeof(int));
                    return_ptr += sizeof(int);
                    int zero = 0;
                    if(S_ISREG(buffer.st_mode)){
                        memcpy(return_ptr, (void *)&zero, sizeof(int));
                        return_ptr += sizeof(int);
                    }else if(S_ISDIR(buffer.st_mode)){
                        memcpy(return_ptr, (void *)1, sizeof(int));
                        return_ptr += sizeof(int);
                    }else{
                        memcpy(return_ptr, (void *)-1, sizeof(int));
                        return_ptr += sizeof(int);
                    }

                    memcpy(return_ptr, (void *)strlen(readDirectory->d_name), sizeof(int));
                    return_ptr += sizeof(int);
                    memcpy(return_ptr, (void *)readDirectory->d_name, strlen(readDirectory->d_name));
                    return_ptr += strlen(readDirectory->d_name);

                    r.return_val = return_buffer;
                    r.return_size = sizeof(return_buffer);
                    return r;
                }
                directory = directory->next;
            }
        }
        current = current->next;
    }

    return error_val;
}

//params: user_ip -> filepath -> mode
return_type sOpen(const int nparams, arg_type* a){
	if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

	char *filepath = (char *)a->next->arg_val;
	int mode = (int)a->next->next->arg_val;

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
        op_head->ip = user_ip;
        op_tail = op_head;
	}else{
        OpenedFile *op_newFile = (OpenedFile *)malloc(sizeof(OpenedFile));
        op_newFile->fd = fd;
        op_newFile->ip = user_ip;
        op_tail->next = op_newFile;
        op_tail = op_tail->next;
	}

	r.return_val = (void *)fd;
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
    int fd = (int)a->next->arg_val;

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
                    free(op_head);
                    op_head = op_newHead;
                }else{
                    OpenedFile *op_newCurrent = op_current->next;
                    free(op_current);
                    op_prev->next = op_newCurrent;
                }
                r.return_val = (void *)close(fd);
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
	int fd = (int)a->next->arg_val;
	int bufsize = (int)a->next->next->arg_val;
	int count = (int)a->next->next->next->arg_val;

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

	int fd = (int)a->next->arg_val;
	int bufsize = (int)a->next->next->arg_val;
	int count = (unsigned int)a->next->next->next->arg_val;
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

int main() {
    register_procedure("sMount", 2, sMount);
    register_procedure("sUnmount", 1, sUnmount);
    register_procedure("sOpenDir", 2, sOpenDir);
    register_procedure("sCloseDir", 2, sCloseDir);
    register_procedure("sOpen", 3, sOpen);
    register_procedure("sClose", 2, sClose);
    register_procedure("sRead", 4, sRead);
    register_procedure("sWrite", 4, sWrite);
    register_procedure("sRemove", 2, sRemove);

    launch_server();
    return 0;
}
