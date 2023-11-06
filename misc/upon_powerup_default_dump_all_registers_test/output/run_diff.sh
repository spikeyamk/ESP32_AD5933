#!/bin/bash

# Specify the directory where your files are located
directory="."

# Loop through each file and compare it with every other file
for file1 in "$directory"/*.txt; do
    for file2 in "$directory"/*.txt; do
        # Ensure we're not comparing the same file
        if [ "$file1" != "$file2" ]; then
            # Extract the file names without the directory path
            filename1=$(basename "$file1")
            filename2=$(basename "$file2")
            
            # Run the diff command
            diff "$file1" "$file2"
	fi
    done
done

