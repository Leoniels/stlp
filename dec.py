import sys

Ck = int(sys.argv[1], 0)
a = int(sys.argv[2], 0)
t = int(sys.argv[3], 0)
n = int(sys.argv[4], 0)

while (t>0):
    a = a**2 % n
    t -= 1

print(Ck - a)
