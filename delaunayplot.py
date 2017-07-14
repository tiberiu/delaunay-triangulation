import matplotlib.pyplot as plt


class Point:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

class Triangle:
    def __init__(self, p1, p2, p3):
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3

class Navmesh:
    def __init__(self, filename):
        f = open(filename, "r")
        line = f.readline()
        parts = line.split(" ")

        pointsCnt = int(parts[0])
        triangleCnt = int(parts[1])

        self.points = []
        self.triangles = []
        self.neighbours = []

        for i in xrange(0, pointsCnt):
            line = f.readline()
            parts = line.split(" ")

            x, y, z = map(float, parts[0:3])
            self.points.append(Point(x, y, z))

        for i in xrange(0, triangleCnt):
            line = f.readline()
            parts = line.split(" ")

            p1, p2, p3 = map(int, parts[0:3])
            self.triangles.append(Triangle(p1, p2, p3))

            for j in xrange(3, 6):
                neighbour = int(parts[j])
                if neighbour != -1:
                    self.neighbours.append([i, neighbour])

    def draw(self, subplotNumber, title):
        subplot = plt.subplot(2, 2, subplotNumber)

        rangeX = [min([point.x for point in self.points]) - 20,
                  max([point.x for point in self.points]) + 20]

        rangeY = [min([point.y for point in self.points]) - 20,
                  max([point.y for point in self.points]) + 20]

        subplot.title.set_text(title)
        subplot.axis(rangeX + rangeY)

        X = [point.x for point in self.points]
        Y = [point.y for point in self.points]
        plt.plot(X, Y, "ro")

        for triangle in self.triangles:
            self.draw_triangle(triangle)

        for neighbour in self.neighbours:
            self.draw_neighbours(neighbour)

    def draw_triangle(self, triangle):
        X = [self.points[triangle.p1].x, self.points[triangle.p2].x, 
             self.points[triangle.p3].x, self.points[triangle.p1].x]
        Y = [self.points[triangle.p1].y, self.points[triangle.p2].y, 
             self.points[triangle.p3].y, self.points[triangle.p1].y]
        
        lines = plt.plot(X, Y)
        plt.setp(lines, "color", "b", "linewidth", 2.0)

    def draw_neighbours(self, pair):
        return
        t1 = self.triangles[pair[0]]
        t2 = self.triangles[pair[1]]

        x1 = (self.points[t1.p1].x + self.points[t1.p2].x + self.points[t1.p3].x) / 3
        y1 = (self.points[t1.p1].y + self.points[t1.p2].y + self.points[t1.p3].y) / 3 

        x2 = (self.points[t2.p1].x + self.points[t2.p2].x + self.points[t2.p3].x) / 3
        y2 = (self.points[t2.p1].y + self.points[t2.p2].y + self.points[t2.p3].y) / 3 

        lines = plt.plot([x1, x2], [y1, y2])
        plt.setp(lines, "color", "r", "linewidth", 2.0)



# Initial Points
points = []
f = open("data/delaunay.in")
N = int(f.readline())

for i in xrange(0, N):
    l = f.readline()
    parts = l.split(' ')
    points.append([float(parts[0]), float(parts[1])])

X = list(p[0] for p in points)
Y = list(p[1] for p in points)

rangeX = [min(X) - 20, max(X) + 20]
rangeY = [min(Y) - 20, max(Y) + 20]

plt.figure(figsize=(11, 7))
subplt0 = plt.subplot(2, 2, 1)
subplt0.title.set_text("Initial Points")

plt.plot(X, Y, "ro")
plt.axis(rangeX + rangeY)

for i in xrange(0, len(X)):
    subplt0.text(X[i], Y[i] + 3, str(i))

# Flip algorithm

flip_navmesh = Navmesh("data/delaunay_flip.out")
flip_navmesh.draw(2, "Flip Algorithm")

divide_navmesh = Navmesh("data/delaunay_bowyerwatson.out")
divide_navmesh.draw(3, "Bowyer-Watson Algorithm")

online_navmesh = Navmesh("data/delaunay_online.out")
online_navmesh.draw(4, "Online Algorithm")



plt.show()
