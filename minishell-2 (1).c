/* Robert Brandl
I pledge my honor that I have abided by the Stevens Honor System.*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>//needed for ec
#include <time.h>//needed for stat ec
#define BLUE "\x1b[34;1m"
#define GREEN "\x1b[32m"
#define DEFAULT "\x1b[0m"
#define MAXLINE 4096//max chars per line

volatile sig_atomic_t interrupted = 0; //set to false initially
int find = 0;//set to 1 if file if found in the find function (FOR EC)

void sighandler(){//handles when SIGINT received
     printf("\n");//prints out nextline character
     fflush(stdout);
     interrupted = 1;//sets interrupted to 'false' or 1
}

void func_cd(char* path){//function to perform cd
    struct passwd *pass;//password struct to use getpwuid
    if (strcmp("~", path) == 0){//when ~ is given as the path
        pass = getpwuid(getuid());//creates password struct
        if (pass == NULL){//checks that the passwd can be accessed
            fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        }
        if (chdir(pass->pw_dir) == -1){//uses pass to change directory to the home directory, returns error if it fails
            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", path, strerror(errno));
        }
    }
    else if (path[0] == '~'){//if the path contains ~
        pass = getpwuid(getuid());//same as above, vhanges directory to home directory if possible
        if (chdir(pass->pw_dir) == -1){
            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", path, strerror(errno));
        }
        char a[strlen(path)-2];//array to create a new path that excludes the ~/ characters at the front of the path
        int ind = 0;
        for (int i = 2; i < strlen(path); i++){//loop starts at 2
            a[ind] = path[i];
            ind++;
        }
        if (chdir(a) == -1){//changes directory to the directory a if possible
            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", path, strerror(errno));
        }
    }
    else{//otherwise, get the realpath of the given path if possible and try to change directory
        char buf[PATH_MAX];//buffer for path
		char* sdir = realpath(path, buf);//access the full file path
        if (sdir == NULL || chdir(sdir) == -1){//error check for both realpath and chdir
            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", path, strerror(errno));
        }
    }
}

//EXTRA CREDIT FUNCTION 2: FIND HELPER
void func_find(char* path, char* file){//recursive helper function for find
    char fullpath[PATH_MAX];//creates a variable to store fullpath
    struct dirent *dp;//creates dirent struct
    DIR *dir = opendir(path);//opens the directory
    if (!dir) return;//if it fails, return (no error because function is recursive!!!
    while ((dp = readdir(dir)) != NULL){//reads through directories
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
            strcpy(fullpath, path);
            strcat(fullpath, "/");
            strcat(fullpath, dp->d_name);
            if (strcmp(file, dp->d_name) == 0){//checks if the current file matches the file being looked for
                printf("%s\n", fullpath);
                fflush(stdout);
                find = 1;//sets find equal to 1
            }
            func_find(fullpath, file);//makes recursive call with the new path
        }
        else{//when the path is . or .., do not recurse through, just check for matching file name
            strcpy(fullpath, path);
            strcat(fullpath, "/");
            strcat(fullpath, dp->d_name);
            if (strcmp(file, dp->d_name) == 0){//checks if the current file matches the file being looked for
                printf("%s\n", fullpath);//prints out path
                fflush(stdout);
                find = 1;//sets find equal to 1
            }
        }
    }
    closedir(dir);//closes the directory
}

//EXTRA CREDIT FUNCTION 3: STAT HELPER FUNCTION
void func_stat(char* file, char* directory){//herlper function to generate the information for stat
    struct dirent *dp;//creates dirent struct
    DIR *dir = opendir(directory);//opens the directory
    if (dir == NULL){//checks if directory cant be opened
        fprintf(stderr, "Error: cannot open directory.");
    }
    while ((dp = readdir(dir)) != NULL){//loops through the directory
        struct stat fileinfo;
        struct passwd *pass;//password struct to use getpwuid
        if (strcmp(file, dp->d_name) == 0){//if the file being looked for
            if (stat(dp->d_name, &fileinfo) == 0){//ensures stat is successful
            printf("  File: %s\n", dp->d_name);
            printf("  Size: %ld	", fileinfo.st_size);
            printf("        Blocks: %ld	  ", fileinfo.st_blocks);
            printf(" IO Block: %ld   ", fileinfo.st_blksize);
            fflush(stdout);
            char perm[11];//uses an array to store the permission string
            if (S_ISDIR(fileinfo.st_mode) == 1){
            	printf("directory\n");
            	perm[0] = 'd';
            }
            else if (S_ISCHR(fileinfo.st_mode) == 1){
            	printf("character special file\n");
            	perm[0] = 'c';
            }
            else if (S_ISBLK(fileinfo.st_mode) == 1){
            	printf("block special file\n");
            	perm[0] = 'b';
            }
            else if (S_ISREG(fileinfo.st_mode) == 1){
            	printf("regular file\n");
            	perm[0] = '-';
            }
            else if (S_ISFIFO(fileinfo.st_mode) == 1){
            	printf("pipe\n");
            	perm[0] = 'p';
            }
            else if (S_ISLNK(fileinfo.st_mode) == 1){
            	printf("symbolic link\n");
            	perm[0] = 'l';
            }
            else if (S_ISSOCK(fileinfo.st_mode) == 1){
            	printf("socket\n");
            	perm[0] = 's';
            }
            printf("Device: %jxh/%ldd", (fileinfo.st_dev), (fileinfo.st_dev));
            printf("      Inode: %ld	 ", fileinfo.st_ino);
            printf("  Links: %ld\n", fileinfo.st_nlink);
            fflush(stdout);
            //accesses the numeric permission values using ints
            int usern = 0;
            int groupn = 0;
            int othern = 0;
            if ((fileinfo.st_mode & S_IRUSR) != 0){
				perm[1] = 'r';
				usern += 4;
			}
			else{
				perm[1] = '-';
			}
			if ((fileinfo.st_mode & S_IWUSR) != 0){
				perm[2] = 'w';
				usern += 2;
			}
			else{
				perm[2] = '-';
			}
			if ((fileinfo.st_mode & S_IXUSR) != 0){
				perm[3] = 'x';
				usern += 1;
			}
			else{
				perm[3] = '-';
			}
			if ((fileinfo.st_mode & S_IRGRP) != 0){
				perm[4] = 'r';
				groupn += 4;
			}
			else{
				perm[4] = '-';
			}
			if ((fileinfo.st_mode & S_IWGRP) != 0){
				perm[5] = 'w';
				groupn += 2;
			}
			else{
				perm[5] = '-';
			}
			if ((fileinfo.st_mode & S_IXGRP) != 0){
				perm[6] = 'x';
				groupn += 1;
			}
			else{
				perm[6] = '-';
			}
			if ((fileinfo.st_mode & S_IROTH) != 0){
				perm[7] = 'r';
				othern += 4;
			}
			else{
				perm[7] = '-';
			}
			if ((fileinfo.st_mode & S_IWOTH) != 0){
				perm[8] = 'w';
				othern += 2;
			}
			else{
				perm[8] = '-';
			}
			if ((fileinfo.st_mode & S_IXOTH) != 0){
				perm[9] = 'x';
				othern += 1;
			}
			else{
				perm[9] = '-';
			}
			perm[10] = '\0';
			char permnum[5];//creates an array to output the permission digits
			permnum[0] = '0';
			permnum[1] = usern + '0';
			permnum[2] = groupn + '0';
			permnum[3] = othern + '0';
			permnum[4] = '\0';
            printf("Access: (%s/%s)", permnum, perm);//prints out permission string
            pass = getpwuid(fileinfo.st_uid);//uses a password struct to access the user id
            printf("  Uid: ( %d/%s) ", fileinfo.st_uid, pass->pw_name);
            pass = getpwuid(fileinfo.st_gid);//gets the group id
            printf("  Gid: ( %d/%s)\n", fileinfo.st_gid, pass->pw_name);
            //gets all of the access times
            printf("Access: %s\n", ctime(&fileinfo.st_atime));
            printf("Modify: %s", ctime(&fileinfo.st_mtime));
            printf("Change: %s", ctime(&fileinfo.st_ctime));
            printf(" Birth: -\n");
            fflush(stdout);
            }
            else {//handles when stat fails
                fprintf(stderr, "Error: stat() failed.\n");
            }
        }
    }
    closedir(dir);//closes the directory
}

int main(int argc, char* argv[]){
	struct sigaction action;//creates a sigaction struct to handle SIGINT
	memset(&action, 0, sizeof(struct sigaction));//clears space for the struct
	action.sa_handler = sighandler;//sets the handler to the created function sighandler
	if (sigaction(SIGINT, &action, NULL) == -1){//installs the handler to handle SIGINT, if it fails, return error
    	fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
    	return EXIT_FAILURE;
	}
    while(1){//main loop to run function
        if (interrupted != 1){//doesn't run the iteration if ^C is used in the terminal
		    char currdir[PATH_MAX];//buffer to get current directory
		    if (getcwd(currdir, sizeof(currdir)) == NULL){//get the current directory if possible
		        fprintf(stderr, "Error: Cannot get current working directory. %s. \n", strerror(errno));
		    }
		    printf("%s[%s]%s> ", BLUE, currdir, DEFAULT);//print out current directory in blue
		    fflush(stdout);//ensure output
		    char tmp = '\0';//temp char to read in from terminal
			char arr[MAXLINE];//creates an array that can hold 10 bytes
			int index = 0;//holds the array index
			int num_spaces = 0;//counts the number of spaces in the user input
			memset(arr, 0, MAXLINE);//zero out the buffer
			unsigned int t;//check that read is successful
			while (((t = read(0, &tmp, 1)) != 0)  && tmp != '\n'){//loops through until it can't be read or the newt line char appears
				if (t == -1 && interrupted == 0){//fail to read if not interrupted and read == -1
				     fprintf(stderr, "Failed to read from stdin. %s.\n", strerror(errno));
				}
				if (interrupted == 1){//if interrupted, don't read anymore
				    break;
				}
				if (tmp == ' ') num_spaces++;//increment the number of spaces
				arr[index] = tmp;//sets the array at index to tmp
				index++;//increments index
			}
			if (interrupted != 1){//if interrupted, skip the rest of the iteration
			arr[index] = '\0';//set the last value in the array to the null terminator
			char* split = strtok(arr, " ");//split the array from the first space
			char* args[num_spaces];//create a new array of char* arrays of size num_spaces
			memset(&args, 0, num_spaces);
			int numstrs = 0;//counts the number of strings or arguments
			while(split != NULL){//continues to split the user input until none remains and is separated as arguments
				args[numstrs] = strdup(split);
				numstrs++;
				split = strtok(NULL, " ");
			}
			if (strcmp(args[0], "exit") == 0){//checks for the exit command
				for (int i = 0; i < numstrs; i++){//frees the array of arguments
				    free(args[i]);
				}
				return EXIT_SUCCESS;//ends the loop
			}
			else if (strcmp(args[0], "cd") == 0){//checks for the cd command
				if (numstrs  == 1 || strcmp(args[1], "~") == 0){//when no args provided or the ~ is the path, then call func_cd wuth ~
				    func_cd("~");
				}
				else if (numstrs == 2){//if some other directory passed as the second argument, call func_cd with that directory
				    func_cd(args[1]);
				}
				else{//error if too many arguments
				    fprintf(stderr, "Error: Too many arguments to cd.\n");
				}
				for (int i = 0; i < numstrs; i++){//free the array of arguments
				    free(args[i]);
				}
			}
			else if (strcmp(args[0], "ls") == 0){//EXTRA CREDIT 1: colorized ls
			    if (numstrs == 1){//uses the current directory
			    	struct dirent *dp;//creates a dirent struct
			    	struct stat fileinfo;//creates a stat struct
			        DIR *dir = opendir(currdir);//opens the current directory
			        while ((dp = readdir(dir)) != NULL){//loops through all directories
			             if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")){//checks to make sure that the current file being checked is not . or .. to prevent infinite loops
							continue;
						}
			            if(stat (dp->d_name, &fileinfo) == 0){//ensures that stat can be run on the current file
			                if (S_ISDIR(fileinfo.st_mode) == 1){//if directory, use grren color for output
			                    printf("%s%s%s\n", GREEN, dp->d_name, DEFAULT);
			                }
			                else{//otherwise use regular color
			                	printf("%s\n", dp->d_name);
			                }
			                fflush(stdout);
			            }
					}
					closedir(dir);//closes the directory
			    }
			    else if (numstrs == 2){//handles a given directory
			    	struct dirent *dp;//creates a dirent struct
			    	struct passwd *pass;//creates a passwd struct
			    	pass = getpwuid(getuid());//sets up passwd struct
			    	char* sdir;//array to store directory path
        			char buf[PATH_MAX];//buffer to get realpath
        			int real = 0;//integer to handle three cases: set 0 when the path does not contain '~', set to 1 when the path is only ~, and set to 2 when the path starts with ~ like "~/hw5"
			    	if (strcmp("~", args[1]) == 0){//when ~ is given as the path
        				if (pass == NULL){//checks that the passwd can be accessed
            				fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        				}
        				sdir = pass->pw_dir;//sets the path to home
        				real = 1;//sets real to 1
        			}
        			else if (args[1][0] == '~'){
        			
        			    if (pass == NULL){//checks that the passwd can be accessed
            				fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        				}
        				sdir = pass->pw_dir;//sets path to home
        				for (int i = 1; i < strlen(args[1]); i++){//adds the rest of the path excluding "~/"
        				    char* str = args[1];
        				    char st[2];
        				    st[0] = str[i];
        				    st[1] = '\0';
        				    strcat(sdir, st);
        				}
        				real = 2;//sets real to 2
        			}
        			else{
		            	sdir = realpath(args[1], buf);//access the full file path
		            }
		            if (sdir != NULL){//makes sure directory exists
		                DIR *dir = opendir(sdir);//opens the current directory
				        if (dir != NULL){//checks that directory can be opened!
						    struct stat file;//creates another stat struct
						    int end = 0;//tracks whether the directory is actually a directory, if not, end is set to 1 and function terminates with error
						    if(stat (sdir, &file) == 0){//ensures that stat can be run on the current file
							        if (S_ISDIR(file.st_mode) != 1){
							            fprintf(stderr, "Error: Not a directory.\n");
							            end = 1;//sets end to 1
							        }
							}
							if (end != 1){//as long as end != 1 continue
								while ((dp = readdir(dir)) != NULL){//loops through all directories
			    	                struct stat fileinfo;//creates new stat struct
			    	                memset(&fileinfo, 0, sizeof(struct stat));//memsets to 0
								      if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")){//checks to make sure that the current file being checked is not . or .. to prevent infinite loops
											continue;
										}
										char* pathw;//creates new path, set based on what real is
										if (real == 1) pathw = realpath(pass->pw_dir, buf);
										else if (real == 2){
										    char* p = pass->pw_dir;
											pathw = realpath(p, buf);
										}
										else pathw = realpath(args[1], buf);
										strcat(pathw, "/");//adds the new file to the path string
										strcat(pathw, dp->d_name);
										int fd = open(pathw, O_RDONLY);//opens the file
									if(fd == -1 || fstat (fd, &fileinfo) == 0){//ensures that stat can be run on the current file, and that open worked
									    if (S_ISDIR(fileinfo.st_mode) == 1){
									        printf("%s%s%s\n", GREEN, dp->d_name, DEFAULT);
									    }
									    else{
									    	printf("%s\n", dp->d_name);
									    }
									    fflush(stdout);
									}
									close(fd);//closes the file descriptor
								}
								closedir(dir);//closes the directory
							}
				        }
				        else{//handles when the directory can't be opened
				            fprintf(stderr, "Error: opendir() failed.\n");
				        }
		            }
		            else{//handles when the directory does not exist
		                fprintf(stderr, "Error: Directory doesn't exist.\n");
		            } 
				}
				else{//too many arguments provided to ls
				    fprintf(stderr, "Error: Too many arguments to ls.\n");
				}
			    for (int i = 0; i < numstrs; i++){//frees the array of args
				    free(args[i]);
				}
			}
			else if (strcmp(args[0], "find") == 0){//EXTRA CREDIT 2: FIND
			    if (numstrs == 3){//proper amount of arguments
			        char* path;//array for path
			        struct passwd *pass;//creates passwd struct to access the home directory
			    	pass = getpwuid(getuid());//sets pass
			        if (strcmp("~", args[1]) == 0){//when ~ is given as the path
        				if (pass == NULL){//checks that the passwd can be accessed
            				fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        				}
        				path = pass->pw_dir;//sets path
        			}
			        else if (args[1][0] == '~'){
        			    if (pass == NULL){//checks that the passwd can be accessed
            				fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        				}
        				path = pass->pw_dir;//sets path with ~ equivalent and then adds the rest of the pathway
        				for (int i = 1; i < strlen(args[1]); i++){
        				    char* str = args[1];
        				    char st[2];
        				    st[0] = str[i];
        				    st[1] = '\0';
        				    strcat(path, st);
        				}
        			}
			        else{//otherwise, make the path just args[1] directly
			            path = args[1];
			        }
			        char* sdir;//new path based onr ealpath
			        char buf[PATH_MAX];//buffer
        			if ((sdir = realpath(path, buf)) == NULL){//checks if the path actually exists
        			    fprintf(stderr, "Error: Directory doesn't exist.\n");
        			}
        			else{
        			    struct stat fileinfo;//creates a new stat struct
        			    if(stat (sdir, &fileinfo) == 0){//ensures that stat can be run on the current file
							if (S_ISDIR(fileinfo.st_mode) != 1){//checks if the path actually corresponds to a directory
								fprintf(stderr, "Error: Not a directory.\n");
							}
							else{
							    func_find(sdir, args[2]);//calls helper function
							    if (find != 1){//if global variable set to 0 and no file was found, print error
							        fprintf(stderr, "find: '%s': No such file or directory.\n", args[2]);
							    }
							}
        			    }
        			}
			    }
			    else if (numstrs < 3){
			        fprintf(stderr, "Error: Not enough arguments to find.\n");
			    }
			    else{
				    fprintf(stderr, "Error: Too many arguments to find.\n");
				}
			    for (int i = 0; i < numstrs; i++){//frees the array of arguments
				    free(args[i]);
				}
			}
			else if(strcmp(args[0], "stat") == 0){//EXTRA CREDIT 3: STAT
				if (numstrs == 2){//handle proper case of 2 arguments
			        func_stat(args[1], currdir);//calls helper function with the filename and directory
			    }
			    else if (numstrs < 2){
			        fprintf(stderr, "Error: Not enough arguments to stat.\n");
			    }
			    else{
				    fprintf(stderr, "Error: Too many arguments to stat.\n");
				}
			    for (int i = 0; i < numstrs; i++){//frees the array of args
				    free(args[i]);
				}
			}
			else if (numstrs >= 1){//for any other commands not implemented above
				pid_t pid = fork();//forks to a child
				int status;//holds the status of the child
				args[numstrs] = NULL;//sets the last arg to null, as required by the exec command
				if (pid < 0){//handles if the child cannot be forked
				    fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
				}
				else if (pid == 0){//the child!;
				    if (execvp(args[0], args) == -1){//calls the func specified by the argument if possible
				        fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
				    }
				    for (int i = 0; i < numstrs; i++){//frees the array of args
						free(args[i]);
					}
				    return EXIT_FAILURE;//returns exit failure
				}
				else{//the parent!!
				    pid_t w = wait(&status);//waits for the children to finish
				    if (w < 0 && interrupted == 0){// if it fails, print error
				        fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
				    }
				}
				for (int i = 0; i < numstrs; i++){//frees the array of args
					free(args[i]);
				}
			}
			else{//otherwise, just free the array of arguments
				for (int i = 0; i < numstrs; i++){
					free(args[i]);
				}
			}
    	}
    	}
    	interrupted = 0;//sets interrupted back to 0
    }
}
