#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
typedef struct Node {
  char *ip;
  struct Node *next;
} Node;

// int fsopen(char *filepath, int mode) {
// 	struct stat buffer;

// 	int err = stat(filepath, &buffer);

// 	if (err == -1 && mode == 0) {return -1;}
// 	if (S_ISDIR(buffer.st_mode)) {return -1;} 

// 	printf("blahhhh");

// 	int fd;
// 	if (mode == 1) {
// 		fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, S_IRUSR | S_IWUSR);
// 	}
// 	else {
// 		fd = open(filepath, O_RDONLY);
// 	}

// 	printf("%d\n", fd);

// 	return fd;
// }

char *findRootName(char *fullpath) {
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

int main(int argc, char *argv[]) {
	char *hello = findRootName("blah/helloooo/i_am_a_folder/test.txt");
	free(hello);
	// printf("%s\n", findRootName("blah/helloooo/i_am_a_folder/test.txt"));
	// printf("%s\n", findRootName("blah/"));
	// printf("%s\n", findRootName("blah"));
	// printf("%s\n", findRootName("blah//"));

	return 1;
}

// int main(int argc, char *argv[]) {
// 	char filepath[] = "/home/jonathan/Dropbox/4A/ECE 454 - Distributed Systems/Assignments/Assignment4/code/test.txt";
// 	int fd;
// 	fd = fsopen(filepath, 0);
// 	if (fd != -1) {
// 		printf("success\n");
// 	}
// 	else{
// 		printf("failure\n");
// 	}

// 	int nread;
// 	int count = 8;

// 	char buf[10]; 

// 	nread = read(fd, buf, count);
// 	printf("%s\n", buf);


// 	nread = read(fd, buf, count);
// 	printf("%s\n", buf);


// 	return 1;
// }

// int main(int argc, char *argv[]) {
// 	Node *root;
// 	Node *tail;
// 	root = malloc(sizeof(Node));
// 	root->ip = "";
// 	root->next = NULL;
// 	tail = root;

// 	char *ip = "22.00.11.11";

// 	Node *new_node = malloc(sizeof(Node));
// 	new_node->ip = ip;
// 	new_node->next = NULL;
	
// 	tail->next = new_node;
// 	tail = new_node;

// 	char *ip2 = "44.22.33.11";

// 	Node *node2 = malloc(sizeof(Node));
// 	node2->ip = ip2;
// 	node2->next = NULL;
	
// 	tail->next = new_node;
// 	tail->next = node2;
// 	tail = node2;

// 	Node *curr = root;

// 	// while (curr != NULL) {
// 	// 	printf("%s\n", curr->ip);

// 	// 	if (curr->ip == ip2) {
// 	// 		printf("works\n");
// 	// 	}

// 	// 	curr = curr->next;
// 	// }

// 	Node *prev = root;
// 	curr = root -> next;

// 	while (curr != NULL) {
// 		if (curr->ip == "22.00.11.11") {
// 			prev->next = curr->next;
// 			free(curr);
// 		}

// 		prev = prev->next;
// 		curr = curr->next;
// 	}


// 	curr = root;	
// 	while (curr != NULL) {
// 		printf("%s\n", curr->ip);

// 		// if (curr->ip == ip2) {
// 		// 	printf("works\n");
// 		// }

// 		curr = curr->next;
// 	}

// 	return 1;
// }