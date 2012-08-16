// Example code from "a tutorial on 'dynamic' arrays in C"
// http://fydo.net

// Special thanks for suggestions and help from:
//   bbulkow of http://bbulkow.blogspot.com
//   tinkertim of http://echoreply.us

// Output:
// Steve's number is 42!
// Bill's number is 33!
// George's number is 15!
// fydo's number is 74!
//
// 6 allocated, 4 used

// This code is public domain. Do whatever you like with it! :D
// I've tested it using GCC, no promises for anything else. Sorry.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *name;
} DYNAMIC;

DYNAMIC 	*the_array = NULL;
int 	num_elements = 0; // To keep track of the number of elements used
int		num_allocated = 0; // This is essentially how large the array is

int add_to_dynstring (DYNAMIC item) {
	if(num_elements == num_allocated) {
		// Feel free to change the initial number of refs and the rate at which refs are allocated.
		if (num_allocated == 0)
			num_allocated = 1; // Start off with 3 refs
		else
			num_allocated ++; // Double the number of refs allocated
		
		// Make the reallocation transactional by using a temporary variable first
		void *_tmp = realloc(the_array, (num_allocated * sizeof(DYNAMIC)));
		
		// If the reallocation didn't go so well, inform the user and bail out
		if (!_tmp)
		{ 
			fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
			return(-1); 
		}
		
		// Things are looking good so far, so let's set the 
		the_array = (DYNAMIC*)_tmp;	
	}
	
	the_array[num_elements] = item; 
	num_elements++;
	
	return num_elements;
}

int main() {
	int i;
    char *name = "Riccardo";
    char *name2 = "Ivana";

	DYNAMIC dynstring;
	dynstring.name = malloc((strlen(name) + 1) * sizeof(char));
	strncpy(dynstring.name, name, strlen(name) + 1);
    add_to_dynstring(dynstring);

	dynstring.name = malloc((strlen(name2) + 1) * sizeof(char));
	strncpy(dynstring.name, name2, strlen(name2) + 1);
    add_to_dynstring(dynstring);
	
	// Regurgitate!
	for (i = 0; i < 2; i++) {
		printf("%s\n", the_array[i].name);
	}
	
	printf("\n%d allocated, %d used\n", num_allocated, num_elements);
	
	// Deallocate!
	for (i = 0; i < 2; i++) {
		
	}

	free(the_array);	
	
	// All done.
	return 0;
}
