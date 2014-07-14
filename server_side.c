#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ece454rpc_types.h"


typedef struct UsedFile{
	int fd;
	bool locked;
	struct UsedFile *next;
} UsedFile;

typedef struct MountedUser {
  char *ip;
  struct MountedUser *next;
} MountedUser;

return_type r;

UsedFile *uf_head;
UsedFile *latest_file;
MountedUser *root;
MountedUser *tail;

int main(int argc, char*argv[]) {
	uf_head = malloc(sizeof(UsedFile));
	uf_head->fd = 0;
	uf_head->locked = false;
	latest_file = uf_head;

	root = malloc(sizeof(MountedUser));
	root->ip = "";
	root->next = NULL;
	tail = root;
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
return_type fsMount(const int nparams, arg_type* a)
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

	MountedUser *new_MountedUser = malloc(sizeof(MountedUser));
	new_MountedUser->ip = user_ip;
	new_MountedUser->next = NULL;
	tail->next = new_MountedUser;
	tail = new_MountedUser;

    r.return_size = sizeof(int);
	r.return_val = (void *) 1;

    return r;
}

//nparams: user_ip
return_type fsUnmount(const int nparams, arg_type* a)
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

//nparams: user_ip -> filename
return_type fsOpenDir(int nparams, arg_type* a) {
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
	}

	return_type error_val;
	error_val.return_val = (void *) -1;
	error_val.return_size = sizeof(int);

	char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

    char *filename = (char *)a->next->arg_val;

	DIR *d = NULL;
    struct dirent *dir;
    char filepath[500];

    d = opendir(".");
    if (d){
        while ((dir = readdir(d)) != NULL){
            //pop onto the stack
            if(dir->d_name == filename){

            }
        }
        closedir(d);
    }

	struct stat buffer;

	int err = stat(filepath, &buffer);
	if (err == -1) {return error_val;}
	if (S_ISDIR(buffer.st_mode)) {

	}
}

return_type fsCloseDir(int nparams, arg_type* a){

}

return_type fsReadDir(int nparams, arg_type* a){

}

//params: user_ip -> filepath -> mode
return_type fsOpen(const int nparams, arg_type* a)
{
	if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	return_type error_val;
	error_val.return_val = (void *) -1;
	error_val.return_size = sizeof(int);

	char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

	char *filepath = (char *)a->next->arg_val;
	int mode = (int)a->next->next->arg_val;

	struct stat buffer;

	int err = stat(filepath, &buffer);
	if (err == -1 && mode == 0) {return error_val;}
	if (S_ISDIR(buffer.st_mode)) {return error_val;}

	int fd;
	if (mode == 1) {
		fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, S_IRUSR | S_IWUSR);
        //set lock here so others cannot use the file
		UsedFile *openedFile;
		if(uf_head->fd != 0){
            openedFile->fd = fd;
            openedFile->locked = true;
            latest_file.next = openedFile;
		}else{
            uf_head->fd = fd;
            uf_head->locked = true;
            latest_file = uf_head;
		}
	}
	else {
		fd = open(filepath, O_RDONLY);
	}

	r.return_val = fd;
	r.return_size = sizeof(int);

	return r;
}

//nparams: user_ip -> fd
return_type fsClose(int nparams, arg_type* a) {
	if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
	}

	char *user_ip = (char *)a->arg_val;
    int fd = a->next->arg_val;

	if (authenticate(user_ip) == 0) {
		r.return_val = -1;
		r.return_size = sizeof(int);
		return r
	}

	r.return_val = close(fd);
	r.return_size(sizeof(int));
	return r;
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


//nparams: fd -> count
return_type fsRead(int nparams, arg_type* a) {
    if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
	}

	//TODO: make sure it works concurrently
	// each reads starts from the next, so what happens when we have to reads
	// from two different clients?
	int fd = a->arg_val;
	int count = a->next->arg_val;

    char buffer[count];
    
    return_type err;
    err.return_val = -1;
    err.return_size = sizeof(int);

    // TODO: If a user gets locked, the process should eventually finish the read
    // How do we take care of this? Poll periodically.

    bool locked = isLocked(fd);
    if(!locked){
        if (read(fd, buffer, count) == -1) {
        	return err;
        }

        r.return_val = buffer;
        r.return_size = sizeof(buffer);
    } else{
        return err;
    }

    return r;
}

return_type fsWrite(int nparams, arg_type* a){
    if (nparams != 3) {
		r.return_val = NULL;
		r.return_size = 0;
	}

	int fd = a->arg_val;
	char *buffer = a->next->arg_val;
	int count = a->next->next->arg_val;

    if(!isLocked(fd)){
        int bytesWritten = write(fd, buffer, count);
        if(bytesWritten > 0){
            r.return_val = (char *)bytesWritten;
            r.return_size = sizeof(int);
        }else{
            return error_val;
        }
    }else{
        return error_val;
    }
}

return_type fsRemove(int nparams, arg_type* a){

}

