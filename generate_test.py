import random

N = 150
rangeX = [0, 1000]
rangeY = [0, 1000]

f = open("data/delaunay.in", "w")
f.write("%d\n" % N)
for i in xrange(0, N):
    f.write("%.2f %.2f\n" % (random.uniform(*rangeX), random.uniform(*rangeY)))
