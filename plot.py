from matplotlib import pyplot as plt
import sys

def readf():
    res = sys.stdin.readlines()
    res = [[float(c) for c in r.split()] for r in res]

    return list(zip(*res))

x, y = readf()
plt.plot(x, y)
plt.savefig("res.pdf")
