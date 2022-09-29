/*Robert Brandl
I pledge my honor that I have abided by the Stevens Honor System.*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
void getpermissionstring (char filename[], char * perm){//creates the permission string for the current file using stat and its macros
    struct stat fileinfo;
    stat(filename, &fileinfo);
    if ((fileinfo.st_mode & S_IRUSR) != 0){
        perm[0] = 'r';
    }
    else{
        perm[0] = '-';
    }
    
    if ((fileinfo.st_mode & S_IWUSR) != 0){
        perm[1] = 'w';
    }
    else{
        perm[1] = '-';
    }
    if ((fileinfo.st_mode & S_IXUSR) != 0){
        perm[2] = 'x';
    }
    else{
        perm[2] = '-';
    }
    if ((fileinfo.st_mode & S_IRGRP) != 0){
        perm[3] = 'r';
    }
    else{
        perm[3] = '-';
    }
    if ((fileinfo.st_mode & S_IWGRP) != 0){
        perm[4] = 'w';
    }
    else{
        perm[4] = '-';
    }
    if ((fileinfo.st_mode & S_IXGRP) != 0){
        perm[5] = 'x';
    }
    else{
        perm[5] = '-';
    }
    if ((fileinfo.st_mode & S_IROTH) != 0){
        perm[6] = 'r';
    }
    else{
        perm[6] = '-';
    }
    if ((fileinfo.st_mode & S_IWOTH) != 0){
        perm[7] = 'w';
    }
    else{
        perm[7] = '-';
    }
    if ((fileinfo.st_mode & S_IXOTH) != 0){
        perm[8] = 'x';
    }
    else{
        perm[8] = '-';
    }
}
void checkdir(char* directory, char* permstring){
    DIR* dp;//creates directory pointer
    if ((dp = opendir(directory)) != NULL){//checks that directory can be opened each time
	    struct dirent* dirp;
	    int len = strlen(directory);//gets the length of the directory path
	    char tempdir[len];
	    for (int i = 0; i <= len; i++){
		tempdir[i] = directory[i];//stores the directory into a temp array
	    }
	    char* matchs = permstring;//creates a temp variable to store the permission string
	    while((dirp =readdir(dp)) != NULL ){//loop through all files in current directory
	        if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")){//checks to make sure that the current file being checked is not . or .. to prevent infinite loops
	            continue;
	        }
		strcat(tempdir, "/");
		strcat(tempdir, dirp->d_name); //adds the current file to the pathway
		struct stat fileinfo;
		char perm[9];//array to hold permission string of current file
		char* dirsend = tempdir;//char* variable to send to helper function to get permission string
		//printf("%s\n", dirsend);
		if(stat (dirsend, &fileinfo) == 0){//ensures that stat can be run on the current file
			getpermissionstring(dirsend, perm);//stores the permission string for the current file in perm
			int match = 1;//variable to track if the permission string of the file matches the permission string being searched for
			if (strcmp(matchs, perm) != 0){//if the strings don't match, set match to 0
			    match = 0;
			}
			if (match == 1 && S_ISREG(fileinfo.st_mode) == 1){//if strings match, print the current pathway
			    printf("%s\n", dirsend);
			}
			if (S_ISDIR(fileinfo.st_mode) == 1){//if the current file is a directory, recurse onto the new directory
			    DIR* checker;//created to ensure no memory leaks with recursive call
			    if ((checker = opendir(dirsend)) == NULL){
			        closedir(dp);
			    }
			    else{
			    	closedir(checker);
			    }
			    checkdir(dirsend, matchs);
			}
			strcpy(tempdir, directory);//resets the temp variable to hold the original pathway for the next file
		}
	    }
       closedir(dp);//closes the directory when finished
    }
    else{//if the given directory cannot be opened, then end the program completely
        closedir(dp);//closes the directory when finished
        fprintf(stderr, "Error: Cannot open directory '%s'. Permission denied.\n", directory); 
        exit(EXIT_FAILURE);
    }
}
int main(int argc, char* argv[]){
    int valid = 0;//creates an int to store whether the input is valif, where 0 means valid, 1 invalid
    if (strlen(argv[2]) != 9){//checks if the permission string is the proper length, if not, sets valid to 1
        valid = 1;
    }
    for (int i = 0; i < 9; i++){//iterates through the permission string of length 9 and checks if each character is either rwx-, if not, sets valid to 1
        if (argv[2][i] != 'r' && argv[2][i] != 'w' && argv[2][i] != 'x' && argv[2][i] != '-'){
            valid = 1;
        }
    }
    /*
    Set of three if statements which check that the corresponding permissions are in the proper order, i.e. only r or - can be in positions 0,3,6 and only w or - in positions 1,4,7 and only x or - in positions 2,5,8
    */
    if ((argv[2][0] != 'r' && argv[2][0] != '-') || (argv[2][3] != 'r' && argv[2][3] != '-') || (argv[2][6] != 'r' && argv[2][6] != '-')){
            valid = 1;
    }
    if ((argv[2][1] != 'w' && argv[2][1] != '-') || (argv[2][4] != 'w' && argv[2][4] != '-') || (argv[2][7] != 'w' && argv[2][7] != '-')){
            valid = 1;
    }
    if ((argv[2][2] != 'x' && argv[2][2] != '-') || (argv[2][5] != 'x' && argv[2][5] != '-') || (argv[2][8] != 'x' && argv[2][8] != '-')){
            valid = 1;
    }
    //in all cases where permission string is invalid, stderr is used to print invalid message and program exits with failure
    if (valid == 1){
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", argv[2]); 
        exit(EXIT_FAILURE);
    }
    char permstring[9];
    for (int i = 0; i < 9; i++){
        permstring[i] = argv[2][i];//stores the string of permissions
    }
    char buf[strlen(argv[1])];//create a char array for the address
    char *sdir = realpath(argv[1], buf);//access the path of the directory
    char* ps = permstring;//creates a char* to hold the permission string
    checkdir(sdir,ps);//calls recursive helper to check for files with these permissions
    exit(EXIT_SUCCESS);//ends program successfully
}
