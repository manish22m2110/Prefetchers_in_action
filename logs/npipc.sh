#!/bin/bash

# Directory containing the script and "No_prefetch" folder
base_dir="$(dirname "$0")"

# Create an array to store IPC values and trace file names
ipc_values=()
trace_files=()

# Iterate through the trace files in the "No_prefetch" folder
no_prefetch_dir="${base_dir}/No_prefetch"
for trace_file in "${no_prefetch_dir}"/*.txt; do
  if [ -f "$trace_file" ]; then
    ipc_line=$(grep -oP 'CPU 0 cumulative IPC: \K[0-9]+\.[0-9]+' "$trace_file")
    if [[ -n "$ipc_line" ]]; then
      ipc_value=$(echo "$ipc_line" | awk '{print $1}')
      ipc_values+=("$ipc_value")
      trace_name=$(basename "$trace_file")
      trace_files+=("$trace_name")
      echo "Extracted IPC from $trace_name: $ipc_value"
    fi
  fi
done

# Check if any IPC values were extracted
if [ ${#ipc_values[@]} -eq 0 ]; then
  echo "No IPC values found in any file."
  exit 1
fi

# Write IPC values to the output file with formatted names
output_file="${base_dir}/NoPrefetchIPC.txt"
echo "No Prefetching IPC values Obtained." > "$output_file"
for ((i=0; i<${#trace_files[@]}; i++)); do
  echo "${trace_files[$i]}: ${ipc_values[$i]}" >> "$output_file"
done

echo "IPC values for No Prefetch have been written to $output_file."
