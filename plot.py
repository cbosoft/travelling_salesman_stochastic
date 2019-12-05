from matplotlib import pyplot as plt
import sys

def readf():
    res = sys.stdin.readlines()
    res = [[float(c) for c in r.split()] for r in res]

    return list(zip(*res))

x, y = readf()
x = list(x)
y = list(y)
x.append(x[0])
y.append(y[0])

plt.plot(x, y)
plt.savefig("res.pdf")
