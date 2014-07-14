#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "ece454rpc_types.h"

using namespace std;

typedef struct UsedFile{
	int fd;
	bool locked;
	UsedFile *next;
} UsedFile;

UsedFile *uf_head;
UsedFile *latest_file;
uf_head = (UsedFile *)malloc(sizeof(UsedFile));
uf_head->fd = NULL;
uf_head->locked = false;
latest_file = uf_head;

typedef struct MountedUser {
  char *ip;
  MountedUser *next;
} MountedUser;

MountedUser *root;
MountedUser *tail;
root = malloc(sizeof(MountedUser));
root->ip = "";
root->next = NULL;
tail = root;

return_type r;

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
	MountedUser *curr = root;

	while (curr != NULL) {
		if (curr->fd == user_fd) {
			if (curr->locked == true) { return 0 }
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

	char *user_ip = (char *)a->arg_val

	MountedUser *new_MountedUser = malloc(sizeof(MountedUser));
	new_MountedUser->ip = user_ip;
	new_MountedUser->next = NULL:
	tail->next = new_MountedUser;
	tail = new_MountedUser;

    r.return_size = sizeof(int);
	r.return_val = 1;

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

	char *user_ip = (char *)a->arg_val

	MountedUser *prev = head;
	MountedUser *curr = head -> next;

	r.return_size = sizeof(int);
	r.return_val = -1;

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
	error_val.return_val = -1;
	error_val.return_size = sizeof(int);

	char *user_ip = (char *)a->arg_val;
	if (authenticate(user_ip) == 0) {return error_val;}

    char *filename = (char *)a->next->arg_val;

	DIR *d = NULL;
    struct dirent *dir;
    char *filepath[];

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
	error_val.return_val = -1;
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
		fd = open(filepath, O_CREAT | O_WR_ONLY | O_APPEND | O_TRUNC, S_IRUSR | S_IWUSR);
        //set lock here so others cannot use the file
		UsedFile *openedFile;
		if(uf_head->fd != NULL){
            openedFile->fd = fd;
            openedFile->locked true;
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
		r.return_val = -1
		r.return_size = sizeof(int)
		return r
;	}

	r.return_val = close(fd);
	r.return_size(sizeof(int));
	return r;
}

//nparams: fd -> count
return_type fsRead(int nparams, arg_type* a) {
    if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
	}

	int fd = a->arg_val;
	int count = a->next->arg_val;
    char *buffer;
    bool locked = false;

    UsedFile current;
    current = uf_head;
    while(current->next != NULL){
        if(current->fd == fd){
            locked = true;
            break;
        }
    }

    if(!locked){
        read(fd, buffer, count);
        r.return_val = buffer;
        r.return_size = sizeof(buffer);
    }else{
        r.return_val = -1;
        r.return_size = sizeof(int);
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
