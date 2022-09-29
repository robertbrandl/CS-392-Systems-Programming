/* Robert Brandl
I pledge my honor that I have abided by the Stevens Honor System. */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#define READ_END 0
#define WRITE_END 1
int main(int argc, char const *argv[]) {
    if (argc != 5){//checks for a valid number of arguments, prints based on pdf output
        printf("Usage: ./spfind -d <directory> -p <permissions string>\n");
        exit(EXIT_SUCCESS);
    }
    pid_t ret[2];//creates a small array to hold return values of fork()
    int c1_c2[2];//file descriptors for pipe between the first child (pfind) and the second child (sort)
    int c2_p[2];//file descriptors for pipe between the second child (sort) and the parent (outputs results)
    if (pipe(c1_c2) == -1){//creates a pipe between the first child and second child, otherwise throws error
        fprintf(stderr, "Error: cannot create pipe.\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(c2_p) == -1){//creates a pipe between the second child and the parent, otherwise throws error
        fprintf(stderr, "Error: cannot create pipe.\n");
        exit(EXIT_FAILURE);
    }
    ret[0] = fork();//calls fork to create the first child
    if (ret[0] == 0) {//in the child process if created successfully
    	if (dup2(c1_c2[WRITE_END], 1) == -1){//duplicates the read-end of the c1_c2 pipe and closes stdout for redirection
    		fprintf(stderr, "Error: cannot duplicate file descriptor.\n");
        	exit(EXIT_FAILURE);
    	}
        if (close(c1_c2[READ_END]) == -1){//closes the read end of c1_c2 if possible
        	fprintf(stderr, "Error: cannot close pipe.\n");
        	exit(EXIT_FAILURE);
        }
        if (close(c2_p[READ_END]) == -1){//closes the read end of c2_p if possible
        	fprintf(stderr, "Error: cannot close pipe.\n");
        	exit(EXIT_FAILURE);
        }
        if (close(c2_p[WRITE_END]) == -1){//closes the write end of c2_p if possible
        	fprintf(stderr, "Error: cannot close pipe.\n");
        	exit(EXIT_FAILURE);
        }
        if (execlp("./pfind", "./pfind", argv[1], argv[2], argv[3], argv[4], NULL) == -1){//calls exec to run the pfind execuatable using the provided command line arguments
            fprintf(stderr, "Error: pfind failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (ret[0] == -1){//if fork() fails, print error and end program
        fprintf(stderr, "Error: fork() failed.\n");
        exit(EXIT_FAILURE);
    }
    int status1;//holds the status for the first child
    if (waitpid(ret[0], &status1, 0) == -1){//waits until the first child finishes before forking again
    	fprintf(stderr, "Error: waitpid() failed.\n");
        exit(EXIT_FAILURE);
    }
    ret[1] = fork();//calls fork() to create the second child
    if (ret[1] == 0) {//in the second child process
        if (dup2(c1_c2[READ_END], 0) == -1){
        	fprintf(stderr, "Error: cannot duplicate file descriptor.\n");
        	exit(EXIT_FAILURE);
        }
    	if (dup2(c2_p[WRITE_END], 1) == -1){//closes the write-end of the c2_p pipe if possible
    		fprintf(stderr, "Error: cannot duplicate file descriptor.\n");
        	exit(EXIT_FAILURE);
        }
        if (close(c1_c2[WRITE_END]) == -1){//closes the write-end of the c1_c2 pipe if possible
        	fprintf(stderr, "Error: cannot close pipe.\n");
        	exit(EXIT_FAILURE);
        }
    	if (close(c2_p[READ_END]) == -1){//closes the read-end of the c1_c2 pipe if possible
    		fprintf(stderr, "Error: cannot close pipe.\n");
        	exit(EXIT_FAILURE);
    	}
        if (execlp("sort", "sort", NULL) == -1){//calls exec to run the sort command
            fprintf(stderr, "Error: sort failed.\n");
        }
    }
    else if (ret[1] == -1){//if second child cannot be created, print error and end program
        fprintf(stderr, "Error: fork() failed.\n");
        exit(EXIT_FAILURE);
    }
    if (close(c1_c2[READ_END]) == -1){
    	fprintf(stderr, "Error: cannot close pipe.\n");
        exit(EXIT_FAILURE);
    }
    if (close(c1_c2[WRITE_END]) == -1){
    	fprintf(stderr, "Error: cannot close pipe.\n");
        exit(EXIT_FAILURE);
    }
    if (close(c2_p[WRITE_END]) == -1){
    	fprintf(stderr, "Error: cannot close pipe.\n");
        exit(EXIT_FAILURE);
    }
    int status2;//holds the status for the second child
    if (waitpid(ret[1], &status2, 0) == -1){//waits for the second child to finish
    	fprintf(stderr, "Error: waitpid() failed.\n");
        exit(EXIT_FAILURE);
    }
    char message;//holds the current character
    int nbytes;//holds the return value of read
    int matches = 0;//tracks the number of matches
    while ((nbytes = read(c2_p[READ_END], &message, 1)) > 0){//loops through the output from sort and read each byte
           if (write(1, &message, 1) == -1){//writes each bytes as long as read is successful
               fprintf(stderr, "Error: write() failed.\n");
               exit(EXIT_FAILURE);
           }
           if (message =='\n'){//updates the number of matches for each newline
               matches++;
           }
    }
    if (WEXITSTATUS(status1) == 0 && WEXITSTATUS(status2) == 0 && argc == 5){//checks that both children completed successfully before printing out the number of matches
    	printf("Total matches: %d\n", matches);//prints out the number of matches
    }
    return 0;//exits successfully
}
