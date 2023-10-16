#!/bin/bash

# Create or overwrite the output file
output_file="L2CMissPerKiloInstructions.txt"
> "$output_file"


# Iterate through each trace file
for trace_file in *.txt; do
  if [ -f "$trace_file" ]; then
    # Extract L2C miss count and total access from the log file
    l2c_total_access=$(grep -oP 'cpu0_L2C TOTAL\s+ACCESS:\s+\K\d+' "$trace_file")
    l2c_miss=$(grep -oP 'cpu0_L2C TOTAL\s+ACCESS:\s+\d+\s+HIT:\s+\d+\s+MISS:\s+\K\d+' "$trace_file")

    # Calculate L2C MPKI and save to the output file
    if [[ -n "$l2c_miss" && -n "$l2c_total_access" ]]; then
      l2c_mpk_inst=$(bc -l <<< "scale=4; ($l2c_miss / $l2c_total_access) * 1000")
      echo "$trace_file: $l2c_mpk_inst" >> "../$output_file"
    fi
  fi
done

echo "L2C Miss Per Kilo Instructions calculations completed. Results saved in $output_file"
