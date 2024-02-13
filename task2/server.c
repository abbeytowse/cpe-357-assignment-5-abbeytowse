#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define PORT 2828

void child_exit(int sig) { 
    int status; 
    pid_t fin;
 
    printf("HERE\n"); 
    fin = waitpid(0, &status, WNOHANG); 
    while (fin < 1) { 
       fin = waitpid(0, &status, WNOHANG);     
    }  
}

void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r");
   char *line = NULL;
   size_t size;
   ssize_t num;

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   while ((num = getline(&line, &size, network)) != EOF)
   {
      write(nfd, line, num); 
   }

   free(line);
   fclose(network);
}

void run_service(int fd)
{
   pid_t child;  

   while (1)
   {
      int nfd = accept_connection(fd);
      struct sigaction sigact; 
      sigact.sa_flags = 0;
      sigemptyset(&sigact.sa_mask);    
      sigact.sa_handler = child_exit;
      sigaction(SIGCHLD, &sigact, NULL); 
      if (nfd != -1)
      {
         printf("Connection established\n");

	 if((child = fork()) < 0) { 
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

int main(void)
{
   int fd = create_service(PORT);

   if (fd != -1)
   {
      printf("listening on port: %d\n", PORT);

      run_service(fd);

      close(fd);
   }

   return 0;
}
