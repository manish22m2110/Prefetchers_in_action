#!/bin/bash

# Create an associative array to store geometric means for each degree folder
declare -A geometric_means

# Iterate through each degree folder (degree2, degree3, degree4, degree5)
for degree_folder in degree*; do
  if [ -d "$degree_folder" ]; then
    echo "Entering folder: $degree_folder"
    cd "$degree_folder" || continue
    
    # Create an array to store IPC values for this degree
    ipc_values=()
    
    # Iterate through each trace file
    for trace_file in *.txt; do
      if [ -f "$trace_file" ]; then
        ipc_line=$(grep -oP 'CPU 0 cumulative IPC: \K[0-9]+\.[0-9]+' "$trace_file")
        if [[ -n "$ipc_line" ]]; then
          ipc_value=$(echo "$ipc_line" | awk '{print $1}')
          ipc_values+=("$ipc_value")
          echo "Extracted IPC from $trace_file in $degree_folder: $ipc_value"
        fi
      fi
    done
    
    # Calculate the geometric mean for this degree
    scale=10  # Set the scale for bc to 10 decimal places
    geometric_mean=1
    count=0
    for ipc in "${ipc_values[@]}"; do
      geometric_mean=$(bc -l <<< "scale=$scale; $geometric_mean * $ipc")
      count=$((count + 1))
    done
    geometric_mean=$(bc -l <<< "scale=$scale; e(l($geometric_mean) / $count)")
    
    # Store the geometric mean in the array
    geometric_means["$degree_folder"]=$geometric_mean
    
    cd ..  # Return to the parent directory
  fi
done

# Print the results
echo "Geometric means of IPC values for each degree folder:"
for degree_folder in "${!geometric_means[@]}"; do
  echo "$degree_folder: ${geometric_means[$degree_folder]}"
done
