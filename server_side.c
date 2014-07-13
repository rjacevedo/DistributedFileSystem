#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ece454rpc_types.h"

using namespace std;

typedef struct ServerFile{
	int fd;
	bool locked;
	ServerFile *next;
} ServerFile;

ServerFile *sf_head;
ServerFile *latest_file;
head = (ServerFile *)malloc(sizeof(ServerFile));
head->fd = NULL;
head->locked = false;

typedef struct Node {
  char *ip;
  Node *next;
} Node;

Node *root;
Node *tail;
root = malloc(sizeof(Node));
root->ip = "";
root->next = NULL;
tail = root;

return_type r;

int authenticate(char *user_ip) {
	Node *curr = root;

	while (curr != NULL) {
		if (curr->ip == user_ip) {
			return 1;
		}
		curr = curr->next;
	}

	return 0;
}

int authorize_file(int user_fd) {
	Node *curr = sf_head;

	while (curr != NULL) {
		if (curr->fd == user_fd) {
			if (curr->locked == true) { return 0 }
			return 1;
		}
		curr = curr->next;
	}

	return -1;
}

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

	char *user_ip = (char *)a->arg_val

	Node *new_node = malloc(sizeof(Node));
	new_node->ip = user_ip;
	new_node->next = NULL:
	tail->next = new_node;
	tail = new_node;

	r.return_val = 1;
	r.return_size = sizeof(int);

    return r;
}

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

	char *user_ip = (char *)a->arg_val

	Node *prev = head;
	Node *curr = head -> next;

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

return_type fsopen(const int nparams, arg_type* a)
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
	}
	else {
		fd = open(filepath, O_RDONLY);
	}

	r.return_val = fd;
	r.return_size = sizeof(int);
	
	return r;
}

return_type fsclose(int nparams, arg_type* a) {
	if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
	}

	int fd = a->arg_val;
	char *user_ip = (char *)a->next->arg_val;

	if (authenticate(user_ip) == 0) {
		r.return_val = -1
		r.return_size = sizeof(int)
		return r
;	}
	
	r.return_val = close(fd);
	r.return_size(sizeof(int));
	return r;
}

return_type fsread(int nparams, arg_type* a) {}

return_type opendir(foldername) {
	// Start searching for foldername from root using bredth-first-search
	// if exists
		// Use "ls" to get folder names and then add to vector for FSDIR
		// Create FSDIR from the found folder
		// return FSDIR
	// else:
	//		return error_num
}
