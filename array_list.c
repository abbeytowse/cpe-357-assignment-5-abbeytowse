#include "array_list.h" 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <dirent.h> 
#include <unistd.h>
#define PATH_SIZE 256 
#define CAPACITY 5

void *checked_malloc(size_t size) {
	int *p; 

	p = malloc(sizeof(char *) * size); 
	if (p == NULL) {
		perror("malloc"); 
		exit(1); 
	} 

	return p;
}

struct array_list array_list_new() {
	struct array_list al; 

	al.length = 0; 
	al.capacity = CAPACITY;
	al.array = checked_malloc(CAPACITY); 
	return al;     
}

struct array_list array_list_add_to_end(struct array_list al, char *arg) {
	if (al.capacity == al.length) { 
		al.array = realloc(al.array, sizeof(char*) * (al.length+CAPACITY)); 
		al.capacity += CAPACITY; 
	}

	al.array[al.length] = arg;   
	al.length += 1;  
	return al; 
}

void free_array_list(struct array_list al) {

	free(al.array); 
} 
