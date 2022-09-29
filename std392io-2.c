//Robert Brandl
//I pledge my honor that I have abided by the Stevens Honor System.
#include "std392io.h"

int output (char* filename, char format, void* data){
    int terminal = 0;//tracks whether the output goes to a file or the terminal
    if (strcmp("", filename) == 0){//if the filename is empty, set terminal to 1 to print to the terminal
        terminal = 1;
    }
    if (format != 's' && format != 'd'){//check that the format is only s for string or d for integer, if not, return error
        errno = EIO;
        return -1;
    }
    if (data == NULL){//checking if the base address of the data is NULL
        errno = EIO;
        return -1;
    }
    if (terminal == 0){//output to the given filename
        int fd = open(filename, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);//creates the file or appends as necessary with permissions, creates file descriptor
        if (format == 's'){//outputs strings to the given file (through fd)
            int size = 0;
            void* tmp = data;
            while (*((char*)data) != '\0'){
            	size++;
            	data++;
            }
            if (size > 0){
            	write(fd, tmp, size);
            }
            write(fd, "\n", 1);//handles next line
        }
        else{//handles output of integers through conversion from int to char
            int* ptr = data;
            int d = *ptr;//gets the integer value from the data address
            int num = d;
            int count = 0;
            while (num){//gets the number of digits of the integer
                count++;
                num /= 10;
            }
            if (d < 0){//handles negative numbers
                char tmp[count+1];//creates char array with extra char for the negative symbol
                d = d / (-1);//number is made positive
                tmp[0] = '-';//sets the first char equal to the negative sign
                int i = count;
		        while (d > 0){//processes each digit of the number and convert to a char, which is then stored to the temp array
		            unsigned int rem = d % 10;
		            tmp[i] = rem + '0';
		            d = d / 10;
		            i--;
		        }
		        tmp[count+1] = '\0';//sets the last value in the char array to the null terminator
		        write(fd, tmp, sizeof(tmp));//outputs the string
		        write(fd, "\n", 1);//goes to next line
            }
            else{//handles positive numbers
                char tmp[count];//creates char array of size count
                int i = count - 1;
		        while (d > 0){//converts each digit to a char
		            unsigned int rem = d % 10;
		            tmp[i] = rem + '0';
		            d = d / 10;
		            i--;
		        }
		        tmp[count] = '\0';//adds the null terminator
		        write(fd, tmp, sizeof(tmp));//writes the full number as a string to the file
		        write(fd, "\n", 1);
            }
            
        }
        lseek(fd, 0, SEEK_SET);//resets the location in the file to the beginning so it can be accessed again
    }
    else{//output to the terminal
        if (format == 's'){//outputs the string to the terminal using 1 as the fd
            write(1, data, strlen(data));
            write(1, "\n", 1);
        }
        else {//check if number is negative, add to char
            int* ptr = data;
            int d = *ptr;//accesses the integer from the address data
            int num = d;
            int count = 0;
            while (num){//gets the number of digits of the integer
                count++;
                num /= 10;
            }
            if (d < 0){//handles negative numbers like above
                char tmp[count+1];
                d = d / (-1);
                tmp[0] = '-';//adds the negative sign to the char array
                int i = count;
		        while (d > 0){//converts each int digit to a char, and adds it to the array
		            unsigned int rem = d % 10;
		            tmp[i] = rem + '0';
		            d = d / 10;
		            i--;
		        }
		        tmp[count+1] = '\0';
		        write(1, tmp, sizeof(tmp));//outputs the full number as a string to the terminal
		        write(1, "\n", 1);
            }
            else{//handles positive integers
                char tmp[count];
                int i = count - 1;
		        while (d > 0){
		            unsigned int rem = d % 10;
		            tmp[i] = rem + '0';
		            d = d / 10;
		            i--;
		        }
		        tmp[count] = '\0';
		        write(1, tmp, sizeof(tmp));
		        write(1, "\n", 1);
            }
        }
    }
    return 0;
}

int input(char* filename, char format, void* data){
    int terminal = 0;//integer which stores 0 if input is from a file, 1 if input is from the terminal
    if (strcmp("", filename) == 0){//if no file is given, input is from terminal
        terminal = 1;
    }
    if (terminal == 0){//if input is from file, check that file exists
		char buf[PATH_MAX];
		char* sdir = realpath(filename, buf);//access the full file path
		if (sdir == NULL){//if file is NULL (does not exist), set errno and return -1
		    errno = ENOENT;
		    return -1;
		}
    }
    if (data == NULL){//if the data itself is null, the input is not stored, so no error is needed, the program ends
        return 0;
    }
    if (terminal == 0){//handles storing input from a file to some address at data
        int fd;//stores the fd number
        int opened = 0; //0 means not opened, 1 means opened fd already exists
        int filed; //the file descriptor to use if the file has already been opened
        pid_t pid = getpid();//accesses the current process id
        char cwd[PATH_MAX];//buffer to use to get current working directory
        char* cd = getcwd(cwd, sizeof(cwd));//stores the current working directory
		int num = pid;//temporary variable to store the pid #
		int count = 0;//counter to store number of digits in pid #
		while (num){//gets the number of digits in the pid
		    count++;
		    num /= 10;
		}
		char tmp[count+6];//creates a char array to hold /proc/pid
		int i = count + 5;
		while (pid > 0){//converts pid number to char values and puts it in the tmp array
			unsigned int rem = pid % 10;
			tmp[i] = rem + '0';
			pid = pid / 10;
			i--;
		}
		//sets the other chars in the array
		tmp[count+6] = '\0';
		tmp[0] = '/';
		tmp[1] = 'p';
		tmp[2] = 'r';
		tmp[3] = 'o';
		tmp[4] = 'c';
		tmp[5] = '/';
		strcat(tmp, "/fd/");//concats the tmp array with the rest of the path
		struct dirent *dp;
		int numfds = 0;//stores the total number of fds opened in the current process
		DIR *dir = opendir(tmp);//opens the directory tmp
		if (dir == NULL){//if it doesn't exist, then throw error
		    errno = EIO;
		    return -1;
		}
		while ((dp = readdir(dir)) != NULL){//iterates through the subdirectories and increments the number of file descriptors
		    numfds++;
		}
		closedir(dir);//closes the current directory
		for (int i = 3; i < numfds; i++){//iterates through all fds except 0, 1, 2
		    chdir(tmp);//changes to the proper directory to access the stat for each fd
		    struct stat file_stat;
		    struct stat file_info;
		    int val = fstat(i, &file_stat);//calls fstat on the current file desciptor
		    if (val != -1){
				int inodefd = file_stat.st_ino;//stores the inode number of the file descriptor
				char buf[PATH_MAX];
				chdir(cd);//changes back to the directory of this file
		        char* sdir = realpath(filename, buf);//gets the filename's full path
				if (stat(sdir, &file_info) != -1){//calls stat of the realpath
				
					int inodefile = file_info.st_ino;//gets the inode number of filename
					if (inodefd == inodefile){//compares the inode numbers, if they match, then set opened to 1, save the fd number to filed, and exit loop
						opened = 1;
						filed = i;
						break;
					}
				}
		    }
		}
		chdir(cd);//return back to the proper cwd
		if (opened == 0){//if the file has not been opened, create a new fd
		    char buf[PATH_MAX];
		    char* sdir = realpath(filename, buf);
            fd = open(sdir, O_RDONLY);
		}
		else{//if the file has been opened, use that fd
		    fd = filed;
		}
		if (fd == -1){//if the file descriptor cannot be created, return -1 and set errno
		    errno = ENOENT;
		    return -1;
		}
        int pos = lseek(fd, 0, SEEK_CUR);//gets the current position of the file descriptor
        int size = lseek (fd, 0, SEEK_END);//gets the end position of the file descriptor
        lseek(fd, pos, SEEK_SET);//returns the fd position to where it was
        if (pos == size){//if at the end of the file, return -1, stop program
            return -1;
        }
        if (format == 's'){//handles string input from a file
            char tmp = '\0';
			char* arr = (char*)malloc(128);//creates an array that can hold 128 bytes
			int index = 0;//holds the array index
			int curr_size = 128;//holds the cuurent size
			while (read(fd, &tmp, 1) != 0  && tmp != '\n'){//loops through until it can't be read or the newxt line char appears
				if (index == curr_size){//if array capacity reached
				    char* newarr = malloc(curr_size + 128);//creates a new array of greater size
				    for (size_t j = 0; j < curr_size; j++){//transfers values to new array
				        newarr[j] = arr[j];
				    }
				    free (arr);//frees old array
				    arr = newarr;//sets the old array to the new arrau
				    curr_size += 128;//increases the current capacity
				}
				arr[index] = tmp;//sets the array at index to tmp
				index++;//increments index
			}
			for(int i = 0; i < index; i++){//sets the data to be the char array
			    *(char*) data = arr[i];
			    data++;
			}
			for (int i = index; i < curr_size; i++){//sets the remainder of the buffer size to empty
			    *(char*) data = '\0';
			    data++;
			}
			free (arr);//frees the array
        }
        else if (format == 'd'){//handles integers
            char tmp = '\0';
            *((int*)data) = 0;//sets the data spot to 0
			int* arr = (int*) malloc(128);//creates an array that can hold 128 bytes
			int index = 0;//holds the array index
			int curr_size = 128;//holds the cuurent size
			int neg = 0;//tracks whether the integer is negative
			while (read(fd, &tmp, 1) != 0  && tmp != '\n'){//loops through until it can't be read or the newxt line char appears
				if (index == curr_size){//if array capacity reached
				    int* newarr = (int*)malloc(curr_size + 128);//create a larger array/buffer
				    for (size_t j = 0; j < curr_size; j++){//put the elements from the old array into the new array
				        newarr[j] = arr[j];
				    }
				    free (arr);//free old array
				    arr = newarr;//make arr equal to new array
				    curr_size += 128;//increase capacity
				}
				if (tmp == '-'){//checks if the number is negative, sets neg to 1
				    neg = 1;
				}
				else{//for all chars besides -, add the char to the integer array by converting the char to int by subtracting '0' and incrementing index
				    arr[index] = tmp - '0';
				    index++;
				}
			}
			long num = 0;
			int mult = 1;
			for (int i = index-1; i >= 0; i--){//iterates through int array in reverse to convert the array into one number
			    num = (arr[i])*mult + num;
			    mult = mult * 10;
			}
			if (index > 10 || num > INT_MAX || num < INT_MIN){//sets the data to be negative 1 if the number has more than 10 digits or is greater than the maximum integer or lower than the smallest integer
			    *((int*)data) = (-1);
			}
			else{
				if (neg == 1){//if the number was negative, multiply by -1
					num *= (-1);
				}
				*((int*)data) = (int)num;//set the data address in memory to be the number
			}
			free (arr);//free the array
        }
        else{//when wrong format, read through the data, but can't store it in data because format not valid
            char tmp = '\0';
			char* arr = malloc(128);//creates an array that can hold 128 bytes
			int index = 0;//holds the array index
			int curr_size = 128;//holds the cuurent size
			while (read(fd, &tmp, 1) != 0  && tmp != '\n'){//loops through until it can't be read or the newxt line char appears
				if (index == curr_size){//if array capacity reached
				    char* newarr = malloc(curr_size + 128);//creates a new array of greater size
				    for (size_t j = 0; j < curr_size; j++){//transfers values to new array
				        newarr[j] = arr[j];
				    }
				    free (arr);//frees old array
				    arr = newarr;//sets the old array to the new arrau
				    curr_size += 128;//increases the current capacity
				}
				arr[index] = tmp;//sets the array at index to tmp
				index++;//increments index
			}
			free(arr);
			errno = EIO;
			return -1;
        }
    }
    else{//handling input from the terminal
        if (format == 's'){//for strings
            char c = '\0';
			while (c != '\n'){//read each byte, save to a char variable and add to the data
			    read(0, data, 1);
			    c = *((char*)data);
			    data++;
			}
			data--;
			*((char*)data) = '\0';//sets the final char to the null terminator
			
        }
        else if (format == 'd'){//for integers
            char c = ' ';
            void* tmp = data;//creates a tmp variable to hold data address
            char* d = (char*)data;//converts data to a char* pointer
            *((int*)data) = 0;//sets the data in memory to 0
            int ind = 0;//holds the index
            int neg = 0;//tracks whether the number is negative
			while (c != '\n' && c != '\0'){//iterates through each byte, increments data, and checks whether the integer starts with - for negative
			    read(0,data,1);
			    c = *((char*)data);
			    data++;
			    if (c == '-'){
			        neg = 1;
			    }
			    else{
			        ind++;
			    }
			}
			int arr[ind];//creates an array of size ind
			if (neg == 1){  
			    *d = '\0';
					d++;
				for (int i = 0; i < ind-1; i++){//creates the array of ints by converting each char to an int and replacing the data value with \0
					arr[i] = (*((char*)d))  - '0';
					*d = '\0';
					d++;
				}
			}
			else{
				for (int i=0; i < ind-1; i++){//creates the array of ints by converting each char to an int and replacing the data value with \0
					arr[i] = (*((char*)d))  - '0';
					*d = '\0';
					d++;
				}
			}
			long num = 0;//stores the number in a data type larger than an int
			int mult = 1;
			for (int i = ind-2; i >= 0; i--){//converts the array of ints into a long
			    num = (arr[i])*mult + num;
			    mult = mult * 10;
			}
			if (ind > 10 || num > INT_MAX || num < INT_MIN){//sets the data to be negative 1 if the number has more than 10 digits or is greater than the maximum integer or lower than the smallest integer
			    *((int*)tmp) = (-1);
			}
			else{//otherwise, checks if the number is negative and puts the integer into data
			    if (neg == 1){
			        num *= (-1);
			    }
			    *((int*)tmp) = (int)num;
			}
        }
        else{//when the wrong data format is entered, reads through the data but does not store it!!!
            char c = '\0';
			while (c != '\n'){
			    read(0, &c, 1);
			}
			errno = EIO;
			return -1;
        }
    }
    return 0;
}

int clean(){
    pid_t pid = getpid();//gets the process id
    char cwd[PATH_MAX];//creates a buffer variable to hold the path
    char* cd = getcwd(cwd, sizeof(cwd));//gets the current working directory
    int num = pid;//creates temp variable to hold the pid
    int count = 0;//holds the number of digits of the number
    while (num){//loops to get the number of digits in the process id
        count++;
        num /= 10;
    }
    char tmp[count+6];//creates a new array with the number of digits + 6 for the beginning of the path
    int i = count + 5;
	while (pid > 0){//converts the digits of the number into chars into tmp
	    unsigned int rem = pid % 10;
		tmp[i] = rem + '0';
		pid = pid / 10;
		i--;
	}
	//handles the remainder of the tmp array
	tmp[count+6] = '\0';
	tmp[0] = '/';
	tmp[1] = 'p';
	tmp[2] = 'r';
	tmp[3] = 'o';
	tmp[4] = 'c';
	tmp[5] = '/';
    strcat(tmp, "/fd/");//concats the rest of the path address to tmp
    struct dirent *dp;
    int numfds = 0;//holds the number of file descriptors
    DIR *dir = opendir(tmp);//opens the directory of tmp
    if (dir == NULL){//if the directory does not exist, return error
        errno = EIO;
        return -1;
    }
    while ((dp = readdir(dir)) != NULL){//loops through all directories and increments the number of file descriptors
        numfds++;
    }
    closedir(dir);//closes the directory
    chdir(tmp);//changes directory to where the file descriptors is located
    for (int i = numfds; i >= 3; i--){//closes all of the file descriptors in reverse
        close(i);
    }
    chdir(cd);//returns back tot he directory of std392io.c
    return 0;
}
