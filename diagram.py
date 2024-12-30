import matplotlib.pyplot as plt

paths = "D:\\python\\project1\\vrptwData\\"
def read_file(paths):
    data = []
    with open(paths, 'r') as file:
        for line in file:
            if line.strip():
                numbers = line.split()
                numbers = [float(num) for num in numbers]
                data.append(numbers)
                
    return data

def draw_diagramOut(x_data, y_data):
    plt.bar(x_data, y_data) 
    plt.xlabel('RC2')
    plt.ylabel('num_routes')
    plt.title(f'Mean Routes: {sum(y_data)/len(y_data)}')
    plt.grid(True)
    plt.show()

def draw_diagramIn(x_data, y_data):
    plt.scatter(x_data, y_data)
    plt.xlabel('X_Coordination')
    plt.ylabel('Y_Coordination')
    plt.title('C2')
    plt.grid(True)
    plt.show()   

data = read_file(paths + "C201.txt")
data = data[1:]
x_data = [ x[1] for x in data]
y_data = [ x[2] for x in data]
draw_diagramIn(x_data,y_data)
# y_data = [3,4,3,4,2,3,3,4]
# x_data = range(1,1+len(y_data))
# draw_diagramOut(x_data,y_data)