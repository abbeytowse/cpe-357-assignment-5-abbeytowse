#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H
#define _GNU_SOURCE
#include <stddef.h>
#include <unistd.h> 

struct dict { 
    pid_t child; 
    int line_num; 
}; 

struct array_list {
    int length; 
    int capacity; 
    char **array; 
};

void *checked_malloc(size_t size);

struct array_list array_list_new(); 

struct array_list array_list_add_to_end(struct array_list al, char *arg); 
 
void free_array_list(struct array_list al); 

#endif
