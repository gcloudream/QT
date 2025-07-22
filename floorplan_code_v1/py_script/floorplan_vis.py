import matplotlib.pyplot as plt

# Define a function to read the file and plot lines
def plot_lines_from_file(file_path, line_color='b-', title='Lines from TXT File'):
    try:
        # Open the file
        with open(file_path, 'r') as file:
            # Iterate through each line in the file
            for line in file:
                # Remove the newline character at the end of the line
                line = line.strip()
                # Skip if the line is empty
                if not line:
                    continue
                try:
                    # Split the line by space and convert the values to floats
                    x1, x2, y1, y2 = map(float, line.split())
                    # Plot the line using matplotlib
                    plt.plot([x1, x2], [y1, y2], line_color)
                except ValueError:
                    print(f"Invalid line format: {line}")
    except FileNotFoundError:
        print(f"File {file_path} not found.")

    # Set the axis labels
    plt.xlabel('X')
    plt.ylabel('Y')
    # Set the plot title
    plt.title(title)
    # Display the grid
    # plt.grid(True)


    plt.axis('equal')
    # Show the plot
    plt.show()

# Call the function and pass the path to your txt file
file_path = 'C:/SLAM/floorplan_code_v1/data/floorplan'
plot_lines_from_file(file_path)