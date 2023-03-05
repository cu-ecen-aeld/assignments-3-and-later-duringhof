#!/bin/sh
if [ $# -eq 2 ]; then
	filesdir=$1
	searchstr=$2
else
	echo "Error: Invalid Number of Arguments"
	echo "Total number of arguments should be 2."
	echo "The order of the arguments should be:"
	echo "	1) File Directory Path."
	echo "	2) String to be searched in the specified directory path."
	exit 1
fi
if [ -d "$filesdir" ]; then
	file_count=$(ls $filesdir | wc -l)
	match_count=$(grep -r $searchstr $filesdir | wc -l)
	echo "The number of files are $file_count and the number of matching lines are $match_count"
	exit 0
else
	echo "Error: Argument 1 is not a directory"
        exit 1
fi
