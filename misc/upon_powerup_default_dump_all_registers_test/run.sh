#!/bin/bash

# Input text
input_text=$(cat tmp.txt | sed 's/^.*\[0x//' | sed 's/^/0x/' | sed 's/\]//')

# Split the text into individual lines
IFS=$'\n' lines=($input_text)

# Output directory
output_dir="output"

# Create the output directory if it doesn't exist
mkdir -p $output_dir

# Counter for file names
file_count=1

# Loop through the lines and create files with 19 lines each
for ((i=0; i<${#lines[@]}; i+=19)); do
    file_name="$output_dir/file$file_count.txt"
    file_count=$((file_count + 1))
    
    for ((j=i; j<i+19 && j<${#lines[@]}; j++)); do
        echo "${lines[j]}" >> $file_name
    done
done

