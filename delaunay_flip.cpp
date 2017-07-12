#include "navmesh.hpp"
#include "common.hpp"
#include <iostream>

int N;
vector<Vector3> points;

using namespace std;

void CheckNodeEdge(Navmesh* navmesh, int nodeID, int p1, int p2) {
    if (nodeID == -1) {
        return;
    }

    int cnt = 0;
    for (int i = 0; i < 3; i++) {
        if (navmesh->nodes[nodeID].points[i] == p1 ||
            navmesh->nodes[nodeID].points[i] == p2) {
            cnt++;
        }
    }

    if (cnt < 2) {
        cerr << "ERROR: Triangle " << nodeID << " doesn't contain " << p1 << " " << p2 << endl; 
    }
}

int main() {
    freopen("delaunay.in", "r", stdin);
    freopen("delaunay_flip.out", "w", stdout);

    // Read the N input points
    cin >> N;
    for (int i = 0; i < N; i++) {
        float x, y;        
        cin >> x >> y;
        points.push_back(Vector3(x, y, 0));
    }

    vector<int> convexPoints = ComputeConvexHull(points);

    Navmesh navmesh = Navmesh(points);

    for (int i = 1; i < convexPoints.size() - 1; i++) {
        NavmeshNode node;
        node.points[0] = convexPoints[0];
        node.points[1] = convexPoints[i];
        node.points[2] = convexPoints[i + 1];

        cerr << "ADDING NODE " << i << " - " << convexPoints[0] << " " << convexPoints[i] << " " << convexPoints[i + 1] << endl;
        navmesh.AddNode(node);
    }

    //for (int i = 0; i < navmesh.nodes.size(); i++) {
    //    cerr << "NODE " << i << " " << navmesh.nodes[i].points[0] << " " << navmesh.nodes[i].points[1] << " " << navmesh.nodes[i].points[2] << endl;
    //}

    navmesh.InitNeighbours(); 
    cerr << "DONE INIT" << endl;

    // Insert additional nodes
    for (int i = 0; i < points.size(); i++) {
        bool inConvexHull = false;
        cerr << "TESTING NODE: " << i << " " << points[i].ToString() << endl;
        for (int j = 0; j < convexPoints.size(); j++) {
            if (convexPoints[j] == i) {
                inConvexHull = true;
                break;
            }
        }

        if (inConvexHull) {
            cerr << "IN CONVEX HULL" << endl;
            continue;
        }

        cerr << "LOOKING FOR TRIANGLE FOR POINT " << i << endl;
        for (int j = 0; j < navmesh.nodes.size(); j++) {
            cerr << "Testing triangle " << j << endl;
            if (navmesh.nodes[j].ContainsPoint(points[i])) {
                cerr << "FOUND TRIANGLE " << j << endl;
                navmesh.SplitTriangle(j, i);
                break;
            }
        }
    }

    // Check navmesh neighbours
    for (int i = 0; i < navmesh.nodes.size(); i++) {
        cerr << "CHECKING NODE " << i << endl;
        CheckNodeEdge(&navmesh, navmesh.nodes[i].neighbours[0], navmesh.nodes[i].points[1], navmesh.nodes[i].points[2]);
        CheckNodeEdge(&navmesh, navmesh.nodes[i].neighbours[1], navmesh.nodes[i].points[0], navmesh.nodes[i].points[2]);
        CheckNodeEdge(&navmesh, navmesh.nodes[i].neighbours[2], navmesh.nodes[i].points[1], navmesh.nodes[i].points[0]);
    }

    // navmesh.InitNeighbours();   
    bool isDelaunay = false;
    while (!isDelaunay)
    {
        isDelaunay = true;
        cerr << "DELAUNAY CHECK" << endl;
        for (int i = 0; i < navmesh.nodes.size() && isDelaunay; i++) {
            for (int j = 0; j < 3; j++) {                
                // Check if these two triangles meet the delaunay condition
                int t1 = i, t2 = navmesh.nodes[i].neighbours[j];
                if (t2 == -1) {
                    continue;
                }

                int p1, p2, p3, p4;
                p1 = navmesh.nodes[t1].points[0];
                p2 = navmesh.nodes[t1].points[1];
                p3 = navmesh.nodes[t1].points[2];

                for (int k = 0; k < 3; k++) {
                    if (navmesh.nodes[t2].points[k] != p1 &&
                        navmesh.nodes[t2].points[k] != p2 &&
                        navmesh.nodes[t2].points[k] != p3) {
                        p4 = navmesh.nodes[t2].points[k];
                        break;
                    }
                }

                Vector3 point1, point2, point3, point4;
                point1 = navmesh.points[p1];
                point2 = navmesh.points[p2];
                point3 = navmesh.points[p3];
                point4 = navmesh.points[p4];

                if (InsideTriangleCircumcircle(point1, point2, point3, point4)) {
                    isDelaunay = false;
                    cerr << "BAD TRIANGLES " << t1 << " " << t2 << endl;
                    // InsideTriangleCircumcircle(point1, point2, point3, point4);
                    navmesh.FlipTriangles(t1, t2);
                    cerr << "DONE FLIPPING" << endl;
                    break;
                }
            }
        }
    }

    cout << navmesh.points.size() << " " << navmesh.nodes.size() << endl;
    for (int i = 0; i < navmesh.points.size(); i++) {
        cout << navmesh.points[i].x << " " << navmesh.points[i].y << " " << navmesh.points[i].z << endl;
    }

    for (int i = 0; i < navmesh.nodes.size(); i++) {
        for (int x = 0; x < 3; x++) {
            cout << navmesh.nodes[i].points[x] << " ";
        }

        for (int x = 0; x < 3; x++) {
            cout << navmesh.nodes[i].neighbours[x] << " ";
        }
        cout << endl;
    }

    return 0;
}
