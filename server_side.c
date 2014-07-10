
typedef struct Node {
  char *ip;
  Node *next;
} Node;

Node *root;
Node *tail;
root = (Node *)malloc(sizeof(Node));
root->ip = "";
root->next = NULL;
tail = root;

return_type add(const int nparams, arg_type* a)
{
    if(nparams != 2) {
	/* Error! */
	r.return_val = NULL;
	r.return_size = 0;
	return r;
    }

    if(a->arg_size != sizeof(int) ||
       a->next->arg_size != sizeof(int)) {
	/* Error! */
	r.return_val = NULL;
	r.return_size = 0;
	return r;
    }

    int i = *(int *)(a->arg_val);
    int j = *(int *)(a->next->arg_val);

    int *ret_int = (int *)malloc(sizeof(int));

    *ret_int = i+j;
    r.return_val = (void *)(ret_int);
    r.return_size = sizeof(int);

    return r;
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

	r.return_size = sizeof(int);
	r.return_val = 1;

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

		prev = head->next;
		curr = curr->next;
	}

    return r;
}


// array activeClients = new array();

return_type opendir(foldername) {
	// Start searching for foldername from root using bredth-first-search
	// if exists
		// Use "ls" to get folder names and then add to vector for FSDIR
		// Create FSDIR from the found folder
		// return FSDIR
	// else:
	//		return error_num
}

int mount(clientaddr, clientport){
	// activeClients.push(clientaddr + clientport);
}\