import matplotlib.pyplot as plt

# Define the prefetcher speedup degrees and IPC values
trace_files = ["Trace1", "Trace2", "Trace3", "Trace4", "Trace5"]
base_case_ipc = [0.320921, 0.352702, 0.212169, 0.137012, 0.288761]
ipc_values = [0.320938,0.352701,0.212202,0.137036,0.288718]

# Initialize an empty list to store speedup values
speedup_values = []

# Calculate the speedup for each degree of prefetcher
for i in range(len(trace_files)):
    speedup = base_case_ipc[i] / ipc_values[i]
    speedup_values.append(speedup)

# Create a bar plot
plt.bar(trace_files, speedup_values, align='center', alpha=0.7)
plt.xlabel('Trace Files')
plt.ylabel('Speedup')
plt.title('Trace File vs. stream_Prefetcher Speedup')
plt.xticks(rotation=45)  # Rotate x-axis labels for readability
plt.ylim(min(speedup_values) - 0.002, max(speedup_values) + 0.002)

# Save the plot as an image file (e.g., PNG)
plt.savefig('Stream_prefetcher.png')

# Display the plot (optional)
plt.show()
