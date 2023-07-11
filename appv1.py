from flask import Flask, render_template
import pandas as pd
import matplotlib.pyplot as plt
import os

app = Flask(__name__)

@app.route('/')
def home():
    #csv_file = 'data/192_168_1_102_soil_data.csv'  # Path to your CSV file
    csv_file = 'data/192_168_1_167_soil_data.csv'  # Path to your CSV file

    # Check if the file exists
    if not os.path.isfile(csv_file):
        return "Data file not found"

    # Load the data
    data = pd.read_csv(csv_file)

    # Generate the graphs and save them as images
    generate_graph(data, 'num_dry_pixels', 'Number of Dry Pixels', 'dry_pixels.png')
    generate_graph(data, 'num_wet_pixels', 'Number of Wet Pixels', 'wet_pixels.png')
    generate_graph(data, 'num_sprout_pixels', 'Number of Sprout Pixels', 'sprout_pixels.png')
    generate_graph(data, 'temperature', 'Temperature', 'temperature.png')
    generate_graph(data, 'humidity', 'Humidity', 'humidity.png')
    generate_graph(data, 'pressure', 'Pressure', 'pressure.png')
    generate_graph(data, 'light_level', 'Light Level', 'light_level.png')

    # Render the home page
    return render_template('home.html')

def generate_graph(data, column, title, filename):
    plt.figure()
    plt.plot(data['timestamp'], data[column])
    plt.title(title)
    plt.grid(True)
    plt.savefig('static/' + filename)  # Save the graph as an image in the static folder

if __name__ == '__main__':
    app.run(debug=True)
