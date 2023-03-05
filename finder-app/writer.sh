#!/bin/sh
if [ $# -eq 2 ]; then
	writefile=$1
	writestr=$2
else
	echo "Error: Invalid Number of Arguments"
	echo "Total number of arguments should be 2."
	echo "The order of the arguments should be:"
	echo "	1) Full path to a file including file name."
	echo "	2) String to be written in the specified file."
	exit 1
fi
touch2() { mkdir -p "$(dirname "$1")" && touch "$1" ; }
touch2 $writefile
if [ $? -eq 0 ]; then
	echo $writestr > $writefile
	exit 0
else
	echo "Error: File could not be created"
        exit 1
fi
