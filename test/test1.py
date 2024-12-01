




file = "../log.log"

with open(file, "r") as f:
    for line in f:
        print(line)
        # numbers = [float(num.strip()) for num in line.split(', ')]
        numbers = [float(num.strip()) for num in line.split(',') if num.strip()]
        print(numbers)