import random

N = 30
rangeX = [0, 100]
rangeY = [0, 100]

f = open("data/delaunay.in", "w")
f.write("%d\n" % N)
for i in xrange(0, N):
    f.write("%.2f %.2f\n" % (random.uniform(*rangeX), random.uniform(*rangeY)))
