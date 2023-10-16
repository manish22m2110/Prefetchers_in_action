# #!/bin/bash

for trace_file in *.txt; do
    # Extract ISSUED and USEFUL values from the trace file
    line=$(grep "cpu0_L2C PREFETCH  REQUESTED" "$trace_file")
    
    result1=$(echo "$line" | awk '{print $6}')
    result2=$(echo "$line" | awk '{print $8}')
    
    # Check if both result1 and result2 are non-empty and result1 is not zero
    if [[ -n "$result1" && -n "$result2" && "$result1" -ne 0 ]]; then
        accuracy=$(echo "scale=2; ($result2 / $result1) * 100" | bc)
        echo "Accuracy for $trace_file: $accuracy%"
    else
        echo "Error: Invalid values for calculation in $trace_file"
    fi
done
