import matplotlib.pyplot as plt

# Initialize lists to store trace names and accuracy percentages
trace_names = []
accuracy_percentages = []

# Read the accuracy values from the file
with open("accuracy.txt", "r") as file:
    for line in file:
        # Split each line into trace name and accuracy percentage
        parts = line.strip().split(": ")
        if len(parts) == 2:
            trace_name, accuracy = parts[0], float(parts[1][:-1])  # Remove the "%" character and convert to float
            trace_names.append(trace_name)
            accuracy_percentages.append(accuracy)

# Create a bar graph
plt.figure(figsize=(10, 6))
plt.bar(trace_names, accuracy_percentages, color='skyblue')
plt.xlabel('Trace Names')
plt.ylabel('Accuracy Percentage')
plt.title('Accuracy Percentage for Traces')
plt.xticks(rotation=45, ha='right')

# Set the y-axis range from 0 to 100
plt.ylim(0, 10)

# Display the graph
plt.tight_layout()

plt.savefig("accuracy_graph.png")
plt.show()
