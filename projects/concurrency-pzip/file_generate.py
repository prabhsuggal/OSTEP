import random, sys

def add(c,x):
    return chr(ord(c) + x)

file = open("file3.txt", "w+")
file_ans = open("file_ans.txt", "w+")
if(len(sys.argv) >= 2):
    random.seed(int(sys.argv[1]))
else:
    random.seed(2)

if(len(sys.argv) >= 3):
    size = int(sys.argv[2])
else:
    size = 2

print("value of size:", size)

str_repeat=""
prev_s = '/0'
for i in range(size):
    s = add('a',random.randrange(0,26))
    if(s == prev_s):
        continue
    else:
        prev_s = s
    count = random.randrange(1,10)
    file_ans.write(str(count)+s)
    str_repeat = str_repeat + (str(count) + s)
    #file_ans.write(str(hex(count)) + " ")
    for j in range(count):
        file.write(s)

if(len(sys.argv) == 4):
    file_repeat = open("file_repeat.txt", "w+")
    for i in range(int(sys.argv[3])):
        file_repeat.write(str_repeat)
    file_repeat.close()

file.close()
file_ans.close()

#print(open("file.txt").read())
#print(open("file_ans.txt").read())
