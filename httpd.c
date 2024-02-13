#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "array_list.h" 

#define NUM_ARGS 2 
#define IDX_PORT 1
 

void *checked_malloc_c(size_t size) {
    char *p;

    p = (char*)malloc(sizeof(char) * size); 
    if (p == NULL) { 
        perror("malloc"); 
	exit(1); 
    } 

    return p; 
}

int is_num(char argv[]) {
    if (isdigit(argv[0])) { 
        return 1; 
    } 

    return 0; 
}

void validate_args(int argc, char *argv[]) { 

    if (NUM_ARGS != argc) { 
        fprintf(stderr, "ERROR: incorrect number of arguments\n"); 
	exit(1); 
    }

    if (!is_num(argv[IDX_PORT])) { 
        fprintf(stderr, "ERROR: command line argument is not an integer\n"); 
	exit(1); 
    } else { 
        int num = atoi(argv[IDX_PORT]); 

	if ((num < 1024) || (num > 65535)) { 
            fprintf(stderr, "ERROR: port number out of range\n"); 
	    exit(1); 
	} 
    }  

    return; 
} 

void remove_trailing_newline(char *str) { 
    
    if (str == NULL) { 
        return; 
    } 

    int len = strlen(str); 
    if (str[len-1] == '\n') { 
        str[len-1] = '\0'; 
    } 
}

int count(char *str) {
    int len = strlen(str); 
    int n = 0; 

    for (int i = 0; i < len; i++) { 
        if (str[i] == 47) { 
            n++; 
        } 
    } 

    return n;  
} 

void child_exit(int sig) { 
    int status; 
    pid_t fin;
 
    fin = waitpid(0, &status, WNOHANG); 
    while (fin > 0) { 
       fin = waitpid(0, &status, WNOHANG);     
    }  
}

void write_header(struct dirent *dp, int nfd, int found, int cgi_request, int pid) {
    struct stat s;    
    char result[25]; 
    int size = 0; 
    char *line1 = NULL; 
    char *line2 = NULL; 
    char *line3 = NULL; 
    char *line4 = "\r\n";

    if (found == 1) {
	if (cgi_request == 1) {
            char *base = "_file.txt";
	    char *folder = "cgi-like/";
            char str_pid[strlen(base) + 10]; 
	    sprintf(str_pid, "%i", pid);  
	    strcat(str_pid, base); 

            char str_folder[strlen(folder) + strlen(str_pid)]; 	    
	    sprintf(str_folder, "%s", folder); 
	    strcat(str_folder, str_pid);  	
            stat(str_folder, &s); 
	} else { 	
	    stat(dp->d_name, &s); 
	}

	size = s.st_size;
        line1 = "HTTP/1.0 200 OK\r\n"; 
        line2 = "Content-Type: text/html\r\n";
        line3 = "Content-Length: "; 
        sprintf(result, "%i\r\n", size); 
    } else { // if the file was not found
	char *content = "HTML ERROR\n";
        
	size = strlen(content) + 1; // add one additional element to the size for the null character 
        if (found == 0) { 	
            line1 = "HTTP/1.0 404 Not Found\r\n"; 
	} else if (found == 3) { 
            line1 = "HTTP/1.0 403 Permission Denied\r\n"; 
	} else if (found == 4) { 
            line1 = "HTTP/1.0 500 Internal Error\r\n";
        } 
	else { 	
            line1 = "HTTP/1.0 400 Bad Request\r\n";
	}

	line2 = "Content-Type: text/html\r\n"; 
	line3 = "Content-Length: "; 
	sprintf(result, "%i\r\n", size);
    } 

    // +1 adds one additional element to the count of the write to account for the null character
    write(nfd, line1, strlen(line1) + 1); 
    write(nfd, line2, strlen(line2) + 1); 
    write(nfd, line3, strlen(line3) + 1); 
    write(nfd, result, strlen(result) + 1); 
    write(nfd, line4, strlen(line4) + 1);

    printf("%s%s%s%s%s", line1, line2, line3, result, line4); 
} 

void write_contents(struct dirent *dp, int nfd, int found, int cgi_request, int pid) { 
    long size;
    char *buffer; 
    FILE *contents;  
  
    if (found == 1) { 

	if (cgi_request == 1) {
            char *base = "_file.txt";
	    char *folder = "cgi-like/";
            char str_pid[strlen(base) + 10];
	    sprintf(str_pid, "%i", pid);  
	    strcat(str_pid, base);   
            char str_folder[strlen(folder) + strlen(str_pid)]; 	    

	    sprintf(str_folder, "%s", folder); 
	    strcat(str_folder, str_pid); 
            contents = fopen(str_folder, "r"); 
        } else { 
            contents = fopen(dp->d_name, "r"); 
        } 
    
        if (contents == NULL) {	
            fprintf(stderr, "ERROR: failed to open file\n"); 
	    return; 
        } 

        fseek(contents, 0L, SEEK_END); 
        size = ftell(contents); 
        fseek(contents, 0L, SEEK_SET); 
        buffer = checked_malloc_c(size); 
        if (buffer == NULL) { 
            fprintf(stderr, "ERROR: malloc failed\n"); 
	    exit(1); 
        }

        fread(buffer, sizeof(char), size, contents);  	
        write(nfd, buffer, strlen(buffer) + 1); // +1 to make sure the count is accounting for the null character 
	printf("%s", buffer); 
        fclose(contents);
        free(buffer); 
    } else { 
	char *message = "HTML ERROR\n";

	write(nfd, message, strlen(message) + 1); // +1 to make sure the count is accounting for the null character 
        printf("%s", message); 	
    } 
} 

int cgi_like_support(char *file) {
    pid_t child; 
    char file_copy[strlen(file) + 1]; // +1 to add one additional element for the null character 
    int found = 0;
    char *function = NULL; 
    char *directory = NULL; 
    char *start = "./";  
    int size = 0; 
    pid_t pid = 0; 
    struct array_list args_list = array_list_new(); 

    strcpy(file_copy, file);
    directory = strtok(file_copy, "/");  
    function = strtok(NULL, "?");
    size = strlen(start) + strlen(function) + 1; // +1 to add additional element for the null characterer 
    char *result = checked_malloc_c(size); 
    strcpy(result, start); 
    strcat(result, function);
    char result1[strlen(result) + 1]; // +1 to add one additional element for the null character  
    strcpy(result1, result);
    args_list = array_list_add_to_end(args_list, result1); 
    if ((child = fork()) < 0) { 
        fprintf(stderr, "ERROR: failed to fork for cgi-like support\n");
	found = 4;
        return found; 
    } else if (child == 0) {
	char *args = NULL; 
        DIR *dir;  
	int fd;
	pid = getpid();  
        char *base = "_file.txt";
        char str_pid[strlen(base) + 10]; 

	sprintf(str_pid, "%i", pid);  
	strcat(str_pid, base);
        if ((dir = opendir(directory)) == NULL) { 
            fprintf(stderr, "ERROR: failed to open cgi-like directory"); 
	    found = 4;
	    return found; 
	} 	

	if (chdir(directory) != 0) {
            fprintf(stderr, "ERROR: failed to change directories\n"); 
	    exit(1); 
	}
        
  	if ((fd = open(str_pid, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0) { 
            fprintf(stderr, "ERROR: failed to open file"); 
	    exit(1); 
        }  

        dup2(fd, 1); // change the output to stdout
	close(fd); 
	if (strstr(file, "?")) { 	
	    args = strtok(NULL, "");  
	    if (strstr(args, "&")) { // more than 1 arg
		int added = 0; 
		int i = 0; 
		char copy_args[strlen(args) + 1]; // +1 to add one additional element for the null characte
		char *arg = NULL; 
	        char *c = strchr(args, '&'); 	

		while (c != NULL) { // this calculates the number of args I have 	
                    i++; 
		    c = strchr(c+1, '&'); 
	        } 

		strcpy(copy_args, args); 
		arg = strtok(copy_args, "&"); 
		args_list = array_list_add_to_end(args_list, arg); 
		added ++; 
		while (added < i) {
	           arg = strtok(NULL, "&"); 
		   args_list = array_list_add_to_end(args_list, arg); 
		   added++;  
	        } 

		arg = strtok(NULL, ""); 
		args_list = array_list_add_to_end(args_list, arg); 
		args_list = array_list_add_to_end(args_list, NULL); 
	        execv(args_list.array[0], args_list.array); 	
		fprintf(stderr, "ERROR: exec failed\n"); 
		exit(1); 
            } else { // only 1 arg
		execl(result, result, args, NULL); 
		fprintf(stderr, "ERROR: exec failed\n"); 
		exit(1); 
	    } 
	} else { // no args 
	    execl(result, result, NULL); 
	    fprintf(stderr, "ERROR: exec failed\n"); 
	    exit(1); 
        } 

    } else {
	wait(0); 
        free(result); 
	free_array_list(args_list); 
    }

    found = 1;
    return child; 
} 

void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r");
   char *line = NULL;
   size_t size;
   ssize_t num;
   char home[1000];

   if (getcwd(home, sizeof(home)) == NULL) { 
       fprintf(stderr, "ERROR: failed to get current working directory\n"); 
       exit(1); 
   }

   if (network == NULL) {
      perror("fdopen");
      close(nfd);
      return;
   }

   while ((num = getline(&line, &size, network)) >= 0) {
      char string[strlen(line) + 1]; // +1 to add one additional element for the null character 
      char *type = NULL;  
      char *file = NULL;   
      //char *version = NULL;
      DIR *dir; 
      struct dirent *dp;
      struct stat s; 
      int found = 0; 
      pid_t pid = 0; 
      int bad_request = 0;
      int cgi_request = 0;  
       
      strcpy(string, line); 
      type = strtok(string, " "); 
      char type_s[strlen(type) + 1]; // +1 to add one additional element for the null character  
      strcpy(type_s, type);
       // +1 to add one additional element for the null character 
       if ((strcmp(type, "GET") == 0) || (strcmp(type, "HEAD") == 0)) {  
          file = strtok(NULL, " ");
	  int first = file[0];  
	  if (first == 47) { // 47 is the ascii for '/' 
	      file++; // remove the '/' at the beginning 
	  }

	  if (strstr(file, "cgi-like")) {
	      char cgi_copy[strlen(file) + 1]; // +1 to add one additional element for the null character 
	      char *cgi = NULL; 

	      strcpy(cgi_copy, file);  
	      cgi = strtok(cgi_copy, "/");
	      if (!strcmp(cgi, "cgi-like")) { 
		  cgi_request = 1; 
	          found = cgi_like_support(file);
		  if (found > 4) { 
                      pid = found; 
		      found = 1; 
		  } 
	      } else {
	          bad_request = 1;  
	      } 	      
	  } 

	  if ((bad_request == 0) && (cgi_request == 0)) { 
	      char file2[strlen(file) + 1]; 

	      if (strstr(file, "/")) {
		  char *part = NULL;
		  int counter = 0;   
		  char name[strlen(file) + 1]; 
		  char d[strlen(file) + 1]; 
		  int n = 0;  
                  
	          strcpy(name, file);
		  n = count(name);
		  part = strtok(name, "/"); 
		  n--;  
		  strcpy(d, part); 
		  while (n > counter) {
	              part = strtok(NULL, "/");  
		      strcat(d, "/");    
		      strcat(d, part);
		      counter++;  
	          }

		  file = strtok(NULL, "/"); 
		  strcpy(file2, file); 
		  remove_trailing_newline(d); 	  
                  if ((dir = opendir(d)) == NULL) {
                      fprintf(stderr, "ERROR: unable to open subdirectory\n Exiting program...\n"); 
		      exit(1); 
	          } 

		  chdir(d); 
	      } else {
                  if ((dir = opendir(".")) == NULL) {  
                      fprintf(stderr, "ERROR: unable to open current working directory\n Exiting program...\n"); 
	              exit(1); 
                  }

		  strcpy(file2, file); 
              }  
             
              while ((dp = readdir(dir)) != NULL) {  
                  if (strcmp(dp->d_name, file2) == 0) { 
                      found = 1; 
	             
		      stat(dp->d_name, &s); 
	              if ((s.st_mode & S_IRUSR) == 0) { 
                          found = 3; 
	              } 

                      break; 
                  } 
              }

          }     
      } 

      if ((strcmp(type, "GET") == 0) && (bad_request == 0)) {
	  write_header(dp, nfd, found, cgi_request, pid); 
	  write_contents(dp, nfd, found, cgi_request, pid);   
      } else if ((strcmp(type, "HEAD") == 0) && (bad_request == 0)) { 
          write_header(dp, nfd, found, cgi_request, pid); 
      } else { // this is the bad request
          found = 2;
          write_header(dp, nfd, found, cgi_request, pid);
          if (strcmp(type, "GET") == 0) { 
              write_contents(dp, nfd, found, cgi_request, pid); 
          } 
      }

      chdir(home); 
   }

   free(line);
   fclose(network);
}

void run_service(int fd) {
   pid_t child;  

   while (1) {
       
      int nfd = accept_connection(fd); 
      struct sigaction sigact;

      sigact.sa_flags = 0;
      sigemptyset(&sigact.sa_mask);    
      sigact.sa_handler = child_exit;
      sigaction(SIGCHLD, &sigact, NULL); 
      if (nfd != -1) {
         printf("Connection established\n");

	 if((child = fork()) < 0) {
	     fprintf(stderr, "ERROR: failed to fork child process\n"); 
         } else if (child == 0) { 
             handle_request(nfd);
	     printf("Connection closed\n");
	     exit(1); 
	 } else {
	     close(nfd);   
	 }
	 
      }  
   } 
}

int main(int argc, char *argv[]) {

   validate_args(argc, argv); 
   int port = atoi(argv[IDX_PORT]); 
   int fd = create_service(port);

   if (fd != -1)
   {
      printf("listening on port: %d\n", port);

      run_service(fd);

      close(fd);
   }

   return 0;
}


