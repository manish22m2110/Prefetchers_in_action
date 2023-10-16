for file in *.txt; do
    # Get the filename without the directory path
    filename=$(basename "$file")

    # Use grep to find the line containing "CPU 0 cumulative IPC:"
    ipc_line=$(grep "CPU 0 cumulative IPC:" "$file")

    # Use awk to extract the IPC value from the line
    ipc_value=$(echo "$ipc_line" | awk '{print $5}')

    # Print the filename and IPC value
    echo "$ipc_value"
done