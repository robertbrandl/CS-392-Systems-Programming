#!/bin/bash
#Robert Brandl
#I pledge my honor that I have abided by the Stevens Honor System.
readonly JUNKDIR="$HOME/.junk" #creates a readonly variable to store the address of the .junk directory

usage() { #function which holds the usage or help message
cat <<EOF #heredoc to store the message, uses the basename utility to include the filename inside the message
Usage: $(basename $0) [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
EOF
}

helpf(){ #when h selected, prints the help heredoc message and exits
    usage
    exit 0
}

list(){ #when l selected, goes to the .junk directory and lists the junked files
    mkdir -p $JUNKDIR #ensures that the directory exists
    cd $JUNKDIR #enters the junk directory
    ls -lAF #lists the junked files
    exit 0
}

purge(){ #purges all the files in .junk
    mkdir -p $JUNKDIR #ensures that the directory exists
    cd $JUNKDIR #enters the junk directory
    shopt -s dotglob #allows the program to remove hidden files
    for file in *; do #iterates through the files
        rm -frd $file #removes each file
    done
    exit 0
}
[ $1 ] || usage #checks for when no arguments are present
NUM=0 #variable to act as a boolean to determine which output when multiple arguments are supplied
hCOUNT=0 #tracks number of times h appears in argument list, to check for cases like -h -h -h
lCOUNT=0 #same as above variable
pCOUNT=0 #same as above variable
if [ $2 ] || [ ${#1} -ge 3 ]; then #if a second flag is present
    while getopts ":hlp" options; do #uses getopts to iterate through the arguments
        case "${options}" in 
            h) #when h, l, or p is chosen, NUM is set to 1
                NUM=1
                ((hCOUNT=hCOUNT+1))
                ;;
            l)
                NUM=1
                ((lCOUNT=lCOUNT+1))
                ;;
            p)
                NUM=1
                ((pCOUNT=pCOUNT+1))
                ;;
            *) #when an unknown flag appears, end the program with an unknown option error message and the help heredoc
                echo "Error: Unknown option '-${OPTARG}'."
                usage
                exit 1
                ;;
         esac
     done
     if [ $hCOUNT -eq $# ]; then #checks for repeated h flags like -h -h -h
         helpf
     fi
     if [ $lCOUNT -eq $# ]; then #checks for repeated l flags like -l -l -l
         list
     fi
     if [ $pCOUNT -eq $# ]; then #checks for repeated p flags like -p -p -p
         purge
     fi
     if [ $NUM = 1 ] ; then #if NUM = 1 and no invalid inputs (just h,l,p, or text files, then return too many options and the help heredoc
         echo "Error: Too many options enabled."
         usage
         exit 1
     fi
fi 

while getopts ":hlp" options; do #performs the traditional getopts checking assuming only one flag or just text files (skips through this loop)
    case "${options}" in 
        h) 
            helpf
            ;;
        l) 
            list
            ;;
        p) 
            purge
            ;;
        *) #exits for an invalid argument
            echo "Error: Unknown option '$1'."
            usage
            exit 1
            ;;
    esac
done

mkdir -p $JUNKDIR #ensures that the directory exists
for i in $@; do #loops through all the arguments since they are not flags or invalid input
    if [ ! -f $i ] && [ ! -d $i ]; then #if the file is not a valid file or directory in the current location, print out a warning message and continue
        echo "Warning: '$i' not found."
    else
        mv -i $i $JUNKDIR #move the file or directory to the junk directory
    fi
done
exit 0
