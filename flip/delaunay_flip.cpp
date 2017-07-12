#include "triangulation.hpp"
#include "common.hpp"
#include <iostream>

int N;
vector<Vector3> points;

using namespace std;

// Creates a triangulation using only the points on the convex hull
void InitConvexHullTriangulation(vector<Vector3>points, Triangulation& triangulation)
{
    vector<int> convexPoints = ComputeConvexHull(points);

    for (int i = 1; i < convexPoints.size() - 1; i++) {
        TriangulationNode node;
        node.points[0] = convexPoints[0];
        node.points[1] = convexPoints[i];
        node.points[2] = convexPoints[i + 1];

        triangulation.AddNode(node);
    }

    // Make sure all triangles have their neighbouring triangles properly set up
    triangulation.InitNeighbours(); 
}

// Look for points that are not on the convex hull and add them to the triangulation
void InsertNonConvexHullPoints(vector<Vector3> points, Triangulation& triangulation)
{
    vector<int> convexPoints = ComputeConvexHull(points);

    for (int i = 0; i < points.size(); i++) {
        bool inConvexHull = false;
        for (int j = 0; j < convexPoints.size(); j++) {
            if (convexPoints[j] == i) {
                inConvexHull = true;
                break;
            }
        }

        if (inConvexHull) {
            continue;
        }

        for (int j = 0; j < triangulation.nodes.size(); j++) {
            if (triangulation.nodes[j].ContainsPoint(points[i])) {
                triangulation.SplitTriangle(j, i);
                break;
            }
        }
    }
}

// Look for triangles not respecting the delaunay condition and flip the common edge
// until the triangulation becomes a delaunay triangulation
void FlipEdges(vector<Vector3> points, Triangulation& triangulation)
{
    bool isDelaunay = false;
    while (!isDelaunay)
    {
        isDelaunay = true;
        for (int i = 0; i < triangulation.nodes.size() && isDelaunay; i++) {
            for (int j = 0; j < 3; j++) {                
                // Check if these two triangles meet the delaunay condition
                int t1 = i, t2 = triangulation.nodes[i].neighbours[j];
                if (t2 == -1) {
                    continue;
                }

                int p1, p2, p3, p4;
                p1 = triangulation.nodes[t1].points[0];
                p2 = triangulation.nodes[t1].points[1];
                p3 = triangulation.nodes[t1].points[2];

                for (int k = 0; k < 3; k++) {
                    if (triangulation.nodes[t2].points[k] != p1 &&
                        triangulation.nodes[t2].points[k] != p2 &&
                        triangulation.nodes[t2].points[k] != p3) {
                        p4 = triangulation.nodes[t2].points[k];
                        break;
                    }
                }

                Vector3 point1, point2, point3, point4;
                point1 = triangulation.points[p1];
                point2 = triangulation.points[p2];
                point3 = triangulation.points[p3];
                point4 = triangulation.points[p4];

                if (InsideTriangleCircumcircle(point1, point2, point3, point4)) {
                    isDelaunay = false;
                    triangulation.FlipTriangles(t1, t2);
                    break;
                }
            }
        }
    }
}

int main() {
    freopen("data/delaunay.in", "r", stdin);
    freopen("data/delaunay_flip.out", "w", stdout);

    // Read the N input points
    cin >> N;
    for (int i = 0; i < N; i++) {
        float x, y;        
        cin >> x >> y;
        points.push_back(Vector3(x, y, 0));
    }

    Triangulation triangulation = Triangulation(points);

    InitConvexHullTriangulation(points, triangulation);
    InsertNonConvexHullPoints(points, triangulation);
    FlipEdges(points, triangulation);

    cout << triangulation.points.size() << " " << triangulation.nodes.size() << endl;
    for (int i = 0; i < triangulation.points.size(); i++) {
        cout << triangulation.points[i].x << " " << triangulation.points[i].y << " " << triangulation.points[i].z << endl;
    }

    for (int i = 0; i < triangulation.nodes.size(); i++) {
        for (int x = 0; x < 3; x++) {
            cout << triangulation.nodes[i].points[x] << " ";
        }

        for (int x = 0; x < 3; x++) {
            cout << triangulation.nodes[i].neighbours[x] << " ";
        }
        cout << endl;
    }

    return 0;
}
