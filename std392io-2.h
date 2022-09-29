//Robert Brandl
//I pledge my honor that I have abided by the Stevens Honor System.
#include <fcntl.h>//open
#include <unistd.h>//close, chdir, getcwd, getpid, read
#include <stdlib.h>//malloc, free
#include <errno.h>//handling errrors and setting errno
#include <string.h>//using strlen and string functions
#include <dirent.h>//iterate over directories
#include <sys/stat.h>//for stat and fstat
#include <limits.h>//for INT_MAX and INT_MIN

int output(char*, char, void*);
int input(char*, char, void*);
int clean();
