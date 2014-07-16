#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ece454rpc_types.h"
#include "stack.c"

//should have linked list for mounted and opened files
typedef struct UsedFile{
	int fd;
	bool locked;
	struct UsedFile *next;
} UsedFile;

typedef struct MountedUser {
  char *ip;
  struct MountedUser *next;
} MountedUser;

typedef struct OpenedFile {
    int fd;
    char *ip;
    struct OpenedFile *next;
} OpenedFile;

typedef struct File {
    char *filename;
    unsigned char type;
    struct File *next;
} File;

typedef struct FSDIR {
    File *files;
    File *currentFile;
    DIR *parent;
} FSDIR;

UsedFile *uf_head = NULL;
UsedFile *latest_file = NULL;
OpenedFile *op_head = NULL;
OpenedFile *op_current = NULL;
OpenedFile *op_tail = NULL;
MountedUser *root = NULL;
MountedUser *tail = NULL;

return_type r;
return_type error_val;
error_val.return_val = (void *) -1;
error_val.return_size = sizeof(int);

int main(int argc, char*argv[]) {
	/*uf_head = (UsedFile *)malloc(sizeof(UsedFile));
	uf_head->fd = 0;
	uf_head->locked = false;
	latest_file = uf_head;

    op_head = (MountedUser *)malloc(sizeof(OpenedFile));
    op_head->fd = 0;
    op_head->ip = NULL;
    op_head->next = NULL;
    op_tail = op_head;
    op_current = op_head;

	root = (MountedUser *)malloc(sizeof(MountedUser));
	root->ip = "";
	root->next = NULL;
	tail = root;*/
}


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
	UsedFile *curr = uf_head;

	while (curr != NULL) {
		if (curr->fd == user_fd) {
			if (curr->locked == true) { return 0; }
			return 1;
		}
		curr = curr->next;
	}

	return -1;
}

//nparams: user_ip
return_type mount(const int nparams, arg_type* a)
{

	// Server:
    // Check to see if the folder exists. If it doesn't, create.
    // if exists or created return 1 else return 0

	if (nparams != 1) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;

    if(root == NULL){
        root = (MountedUser *)malloc(sizeof(MountedUser));
        root->ip = user_ip;
        tail = root;
    }else{
        MountedUser *new_MountedUser = (MountedUser *)malloc(sizeof(MountedUser));
        new_MountedUser->ip = user_ip;
        new_MountedUser->next = NULL;
        tail->next = new_MountedUser;
        tail = new_MountedUser;
    }

    r.return_size = sizeof(int);
	r.return_val = (void *) 0;

    return r;
}

//nparams: user_ip
return_type unmount(const int nparams, arg_type* a)
{

	// Server:
    // Check to see if the folder exists. If it doesn't, create.
    // if exists or created return 1 else return 0

	if (nparams != 1) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;

	MountedUser *prev = root;
	MountedUser *curr = root -> next;

	r.return_size = sizeof(int);
	r.return_val = (void *) -1;

	while (curr != NULL) {
		if (curr->ip == user_ip) {
			prev->next = curr->next;
			free(curr);
			r.return_val = 0;
			return r;
		}

		prev = prev->next;
		curr = curr->next;
	}

    return r;
}

//nparams: user_ip -> filepath
return_type openDir(int nparams, arg_type* a) {
	// Use stat to check if folder already exists
	// if exists
		// Use "ls" to get folder names and then add to vector for FSDIR
		// Create FSDIR from the found folder
		// return FSDIR
	// else:
	//		return error_num
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
    File *f_head = NULL;
    File *f_tail = NULL;
    File *f_newFile = NULL;
    FSDIR *fsdir = (FSDIR *)malloc(sizeof(FSDIR));

	err = stat(filepath, &buffer);
	if (err == -1) {
        r.return_val = (void *)NULL;
        r.return_size = 0;
        return r;
    }

    d = opendir(filepath);
    //TODO: need to figure out better algorithm to find folder
    if (d){
        while ((dir = readdir(d)) != NULL){
            if(f_head == NULL){
                f_head = (File *)malloc(sizeof(File));
                f_head->filename = dir->d_name;
                f_tail = f_head;

                //TODO: check the filepath
                err = stat(filepath+dir->d_name, &buffer);
                if(err == -1){
                    f_head->type = -1;
                }
                if (S_ISDIR(buffer.st_mode)) {
                    f_head->type = 0
                }else{
                    f_head->type = 1;
                }
            }else{
                f_newFile = (File *)malloc(sizeof(File));
                f_newFile->filename = dir->d_name;
                f_tail->next = f_newFile;
                f_tail = f_newFile;
            }
        }
        closedir(d);
    }

    fsdir->files = f_head;
    fsdir->currentFile = f_head;
    fsdir->parent = d;
    r.return_val = (void *)fsdir;
    r.return_size = sizeof(fsdir);
    return r;
}

//params: FSDIR*
return_type closeDir(int nparams, arg_type* a){
    if (nparams != 1) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

    FSDIR *fsdir = (FSDIR *)a->arg_val;

    int err = closedir(d);
    if(err == -1){
        return error_val;
    }

    //free the pointer or its value?
    free(fsdir);

    r.return_val = (void *)0;
    r.return_size = sizeof(int);
    return r;
}

//nparams: FSDIR*
return_type readDir(int nparams, arg_type* a){
    if (nparams != 1) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

    FSDIR *fsdir = (FSDIR *)a->arg_val;

    struct fsDirent *fsdirent = (struct fsDirent *)malloc(sizeof(struct fsDirent));

    if(fsdir->currentFile != NULL){
        fsdirent->entName = fsdir->currentFile->filename;
        fsdirent->entType = fsdir->currentFile->type;

        fsdir->currentFile = fsdir->currentFile->next;

        r.return_val = (void *)fsdirent;
        r.return_size = sizeof(fsdirent);
        return r;
    }else{
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }
}

//params: user_ip -> filepath -> mode
return_type open(const int nparams, arg_type* a){
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
        //set lock here so others cannot use the file
		/*UsedFile *openedFile;
		if(uf_head->fd != 0){
            openedFile->fd = fd;
            openedFile->locked = true;
            latest_file.next = openedFile;
		}else{
            uf_head->fd = fd;
            uf_head->locked = true;
            latest_file = uf_head;
		}*/
	}else {
		fd = open(filepath, O_RDONLY | S_IRUSR);
	}

	if(op_head->fd == 0){
        op_head->fd = fd;
        op_head->ip = user_ip;
	}else{
        OpenedFile op_newFile = (OpenedFile *)sizeof(OpenedFile);
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
return_type close(int nparams, arg_type* a) {
	if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *user_ip = (char *)a->arg_val;
    int fd = (int)a->next->arg_val;

	if (authenticate(user_ip) == 0) {
		r.return_val = (void *)-1;
		r.return_size = sizeof(int);
		return r
	}

    OpenedFile op_prev;
    op_current = op_head;
    op_prev = op_current;
    while(op_current != NULL){
        if(op_current->fd == fd && op_current->ip == user_ip){
            if(op_current == op_head){
                OpenedFile op_newHead = op_head->next;
                free(op_head);
                op_head = op_newHead;
            }else{
                OpenedFile op_newCurrent = op_current->next;
                free(op_current);
                op_prev->next = op_newCurrent;
                op_current = op_newCurrent;
            }
            r.return_val = (void *)close(fd);
            r.return_size = sizeof(int);
            return r;
        }
        if(op_current != op_head){
            op_prev = op_prev->next;
        }
        op_current = op_current->next;
    }

    return error_val;
}

bool isLocked(int fd) {
	UsedFile *current;
    current = uf_head;
    while(current != NULL){
        if (current->fd == fd){
        	if (current->locked == true) {
        		return true;
        	}
        	return false;

        }
        current = current->next;
    }

    return false;
}


//nparams: fd -> buf -> count
return_type read(int nparams, arg_type* a) {
    if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	//TODO: make sure it works concurrently
	// each reads starts from the next, so what happens when we have to reads
	// from two different clients?
	int fd = (int)a->arg_val;
	void *buf = (void *)a->next->arg_val;
	int count = (int)a->next->next->arg_val;

    int bytesRead = -1;

    op_current = op_head;
    while(op_current != NULL){
        if(op_current->fd == fd && op_current->fd == fd){
            bytesRead = read(fd, buf, count);

            r.return_val = (void *)buf;
            r.return_size = sizeof(buf);
            return r;
        }
        op_current = op_current->next;
    }

    return error_val;
    // TODO: If a user gets locked, the process should eventually finish the read
    // How do we take care of this? Poll periodically.

    /*bool locked = isLocked(fd);
    if(!locked){
        if (read(fd, buffer, count) == -1) {
        	return error_val;
        }

        r.return_val = buffer;
        r.return_size = sizeof(buffer);
    } else{
        return error_val;
    }

    return r;*/
}

return_type write(int nparams, arg_type* a){
    if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	int fd = (int)a->arg_val;
	char *buffer = (void *)a->next->arg_val;
	int count = (unsigned int)a->next->next->arg_val;
	int bytesWritten = -1;

    op_current = op_head;
    while(op_current != NULL){
        if(op_current->fd == fd && op_current->fd == fd){
            bytesWritten = write(fd, buffer, count);
            break;
        }
        op_current = op_current->next;
    }
    r.return_val = (void *)bytesWritten;
    r.return_size = sizeof(int);
    return r;
    /*if(!isLocked(fd)){
        int bytesWritten = write(fd, buffer, count);
        if(bytesWritten > 0){
            r.return_val = (char *)bytesWritten;
            r.return_size = sizeof(int);
        }else{
            return error_val;
        }
    }else{
        return error_val;
    }*/
}

return_type remove(int nparams, arg_type* a){
    //loop through the code and find the file with the filename
    //delete the file if it is not locked
}

