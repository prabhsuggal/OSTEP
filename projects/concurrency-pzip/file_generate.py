import random, sys

def add(c,x):
    return chr(ord(c) + x)

file = open("file.txt", "w+")
file_ans = open("file_ans.txt", "w+")
if(len(sys.argv) >= 2):
    random.seed(int(sys.argv[1]))
else:
    random.seed(2)

if(len(sys.argv) == 3):
    size = int(sys.argv[2])
else:
    size = 2

print("value of size:", size)

for i in range(size):
    s = add('a',random.randrange(0,26,1))
    count = random.randrange(50,100,1)
    file_ans.write(str(count) + " ")
    file_ans.write(str(hex(count)) + " ")
    file_ans.write(s + '\n')
    for j in range(count):
        file.write(s)
file.close()
file_ans.close()

print(open("file.txt").read())
print(open("file_ans.txt").read())
