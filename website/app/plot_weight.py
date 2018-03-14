from datetime import datetime
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def plot_weight():
    file_name = "weight"
    weight_data = []
    date_data = []
    with open(file_name, 'r') as f:
        file_data = f.readlines()
        for line in file_data:
            line = line.strip().split(', ')
            line = [line[0], datetime.strptime(line[1], 
                                               "%Y-%m-%d %H:%M:%S.%f")]
            weight_data.append(float(line[0]))
            date_data.append(line[1])

    fig = plt.figure()
    plt.plot(date_data, weight_data)
    plt.xticks(rotation = 75)
    fig.savefig('images/plot.png', bbox_inches='tight')

if __name__ == "__main__":
    plot_weight()
