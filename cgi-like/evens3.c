#define _GNU_SOURCE
#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h> 
#include <sys/resource.h> 
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h> 
#define NUM_ARGS 3 

/*bool is_num (char argv[]) { 
   
    if (!isdigit(argv[0])) {
        return false; 
    }
  
    return true; 
}

int validate_args(int argc, char *argv[]) {

    if (argc == NUM_ARGS) { 
	if (is_num(argv[1])) { 
	    return 0; 
	} else { 
	    fprintf(stderr, "Invalid command line arg given\n");
	    exit(1);  
	}
    } 

    fprintf(stderr, "No command line arg given\n"); 
    exit(1);  
}*/


int main (int argc, char *argv[]) {
    int n = atoi(argv[1]);
    int j = atoi(argv[2]);  
    int h = atoi(argv[3]); 

    // read in integer N from the command-line 
    //validate_args(argc, argv); 
    //n = atoi(argv[1]);   
    for (int i = 0; i <= n; i++) { 
	// parent prints the even numbers from 1 to N - wait for child to terminate  	
        if (i%2 == 0) {
            printf("%d\n", i); 
	 } 
     }
     for (int i = 0; i <= j; i++) { 
         if (i%2 == 0) { 
             printf("%d\n", i); 
         }
     }
      for (int i = 0; i <= h; i++) { 
         if (i%2 == 0) { 
             printf("%d\n", i); 
         }
     }    
    
    return 0; 
}
    
