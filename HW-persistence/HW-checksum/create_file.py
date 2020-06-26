import random, sys

if(len(sys.argv) < 3):
    sys.exit("Usage: ./create_file.py [file_name] [num_values]")

file = open(sys.argv[1], "w+")
for i in range(int(sys.argv[2])):
    file.write(str(int(random.random() * 256))+" ")

file.close()
