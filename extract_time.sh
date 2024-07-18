#!/bin/bash

preprocess_line() {
    local line="$1"
    # Remove trailing ^M characters
    line=$(echo "$line" | tr -d '\r')
    # Remove leading and trailing whitespaces
    line=$(echo "$line" | sed 's/^[ \t]*//;s/[ \t]*$//')
    # Replace multiple whitespaces with a single whitespace
    line=$(echo "$line" | tr -s ' ')
    echo "$line"
}

process_line() {
    local line
    line=$(preprocess_line "$1")
    IFS=' ' read -r -a parts <<< "$line"

    # Extract times using a more precise regular expression
    all_times=$(echo "$line" | sed -n 's/.* \([0-9]\+\(\.[0-9]\+\)\?e[+-]\?[0-9]\+\) \+\([0-9]\+\(\.[0-9]\+\)\?e[+-]\?[0-9]\+\) \+\([0-9]\+\(\.[0-9]\+\)\?e[+-]\?[0-9]\+\).*/\1,\3,\5/p')

    if [ -z "$all_times" ]; then
        return
    fi
    IFS=',' read -r total_time min_time max_time <<< "$all_times"
    kernel_name="${parts[0]}"
    if ! [[ "${parts[1]}" =~ ^[+-]?[0-9]+\.?[0-9]*$ ]]; then
        kernel_name+=" ${parts[1]}"
    fi
    result="$kernel_name,$total_time,$min_time,$max_time"
    echo "$result"
}

extract_time() {
    local input_text="$1"
    local is_in_time_section=0
    local time_info=()

    while IFS= read -r line; do
        if [ $is_in_time_section -eq 0 ]; then
            if [[ "$line" == _* ]]; then
                is_in_time_section=1
            fi
        else
            if [[ "$line" == "Timer label"* ]] || [[ "$line" == "_"* ]] || [ -z "$line" ]; then
                continue
            fi
            kernel_time=$(process_line "$line")
            if [ -n "$kernel_time" ]; then
                time_info+=("$kernel_time")
            fi
        fi
    done <<< "$input_text"

    # Output to CSV
    echo "kernel,total_time,min_time,max_time"
    for time in "${time_info[@]}"; do
        echo "$time"
    done
}

# Check if input and output directory arguments are provided
if [ "$#" -ne 4 ] || [ "$1" != "-i" ] || [ "$3" != "-o" ]; then
    echo "Usage: $0 -i <input_directory> -o <output_directory>"
    exit 1
fi

input_dir="$2"
output_dir="$4"

# Check if input directory exists
if [ ! -d "$input_dir" ]; then
    echo "Input directory not found: $input_dir"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "$output_dir"

# Loop through files matching the pattern in the input directory
for file in "$input_dir"/stencil_l_*_*.log; do
    if [ -f "$file" ]; then
        # Extract base filename without extension
        base_filename=$(basename "${file%.log}")
        # Read input file content
        input_text=$(cat "$file")
        # Extract total times and output to corresponding CSV file in the output directory
        extract_time "$input_text" > "$output_dir/${base_filename}.csv"
    fi
done

echo "Processing completed."
