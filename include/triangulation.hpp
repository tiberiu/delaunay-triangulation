#ifndef __TRIANGULATION__H
#define __TRIANGULATION__H

#include <vector>
#include <iostream>

#include "common.hpp"

using namespace std;

class Triangulation;

class TriangulationNode {
public:
    Triangulation* triangulation;

    // Triangle Points
    int points[3];

    // Neighbouring triangles
    int neighbours[3];

    bool ContainsPoint(Vector3 point);
    double GetDistanceToPoint(Vector3 point);

};

class Triangulation {
public:
    vector<Vector3> points;
    vector<TriangulationNode> nodes;

    Triangulation() {};
    Triangulation(vector<Vector3> _points) : points(_points) {};

    // Adds a new point to the pointset and returns it's id
    int AddPoint(Vector3 point) {
        points.push_back(point);
        return points.size() - 1;
    }

    // Adds a new node (triangle) to the triangulation and returns it's id
    int AddNode(TriangulationNode node)
    {
        node.triangulation = this;
        nodes.push_back(node);
        return nodes.size() - 1;
    }

    // Removes a point and all the nodes containing this point
    // Note this will invalidate any external stored pointIds
    void RemovePoint(int pointId)
    {
        // Remove point
        vector<int> pointNewIds;
        pointNewIds.resize(points.size());
        int crtPos = 0;       
        for (int i = 0; i < points.size() - 1; i++) {
            if (i == pointId) {
                pointNewIds[i] = -1;
                continue;
            }
            points[crtPos] = points[i];
            pointNewIds[i] = crtPos;
            crtPos++;
        }

        points.resize(crtPos);

        // Now remove all nodes containing this
        vector<int> nodeNewIds;
        nodeNewIds.resize(nodes.size());
        crtPos = 0;
        for (int i = 0; i < nodes.size(); i++) {
            bool containsPoint = false;
            if (nodes[i].points[0] == pointId || nodes[i].points[1] == pointId || 
                nodes[i].points[2] == pointId) {
                containsPoint = true;
            }

            if (containsPoint) {
                nodeNewIds[i] = -1;
                continue;
            }

            nodes[crtPos] = nodes[i];
            nodeNewIds[i] = crtPos;
            crtPos++;
        }
        nodes.resize(crtPos);

        // Pass through all nodes and correct any pointIds and neighbourIds that are now incorrect
        for (int i = 0; i < nodes.size(); i++) {

            nodes[i].points[0] = pointNewIds[nodes[i].points[0]];
            nodes[i].points[1] = pointNewIds[nodes[i].points[1]];
            nodes[i].points[2] = pointNewIds[nodes[i].points[2]];

            if (nodes[i].neighbours[0] != -1) {
                nodes[i].neighbours[0] = nodeNewIds[nodes[i].neighbours[0]];
            }

            if (nodes[i].neighbours[1] != -1) {
                nodes[i].neighbours[1] = nodeNewIds[nodes[i].neighbours[1]];
            }

            if (nodes[i].neighbours[2] != -1) {
                nodes[i].neighbours[2] = nodeNewIds[nodes[i].neighbours[2]];
            }
        }
    }

    // Splits one triangle into three triangles given a point inside the triangle
    // Note this assumes the triangle defined by nodeID contains point
    void SplitTriangle(int nodeID, int point)
    {
        // Save the old values of the initial triangle
        int p1, p2, p3;
        p1 = nodes[nodeID].points[0];
        p2 = nodes[nodeID].points[1];
        p3 = nodes[nodeID].points[2];

        int t1, t2, t3;
        t1 = nodes[nodeID].neighbours[0];
        t2 = nodes[nodeID].neighbours[1];
        t3 = nodes[nodeID].neighbours[2];

        // Add 2 new triangles
        TriangulationNode node1, node2;
        int node1ID = AddNode(node1);
        int node2ID = AddNode(node2);

        // First triangle (inplace edit for initial triangle) - p1, p2, point
        EditNode(nodeID, p1, p2, point, node1ID, node2ID, t3);
        EditNodeEdge(t3, p1, p2, nodeID);

        // Second triangle - p2, p3, point
        EditNode(node1ID, p2, p3, point, node2ID, nodeID, t1);
        EditNodeEdge(t1, p2, p3, node1ID);

        // Third triangle - p3, p1, point
        EditNode(node2ID, p3, p1, point, nodeID, node1ID, t2);
        EditNodeEdge(t2, p3, p1, node2ID);
    }

    // Flips the edge between two triangles
    // Note this assumes the triangles are adjancent
    void FlipTriangles(int node1, int node2) {
        // Find edge
        bool foundPoint = false;
        int p1, p2, p3, p4, p5, p6;
        int t1, t2, t3, t4, t5, t6;
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                if (nodes[node1].points[x] == nodes[node2].points[y]) {
                    if (foundPoint) {
                        p2 = nodes[node1].points[x];
                        t2 = nodes[node1].neighbours[x];

                        p5 = nodes[node2].points[y];
                        t5 = nodes[node2].neighbours[y];
                    } else {
                        foundPoint = true;
                        p1 = nodes[node1].points[x];
                        t1 = nodes[node1].neighbours[x];

                        p4 = nodes[node2].points[y];
                        t4 = nodes[node2].neighbours[y];
                    }
                }
            }
        }

        for (int x = 0; x < 3; x++) {
            if (nodes[node1].points[x] != p1 &&
                nodes[node1].points[x] != p2) {
                p3 = nodes[node1].points[x];
                t3 = nodes[node1].neighbours[x];
            }
        }

        for (int x = 0; x < 3; x++) {
            if (nodes[node2].points[x] != p4 &&
                nodes[node2].points[x] != p5) {
                p6 = nodes[node2].points[x];
                t6 = nodes[node1].neighbours[x];
            }
        }

        // Now edit the two triangles
        EditNode(node1, p3, p6, p2, t4, t1, node2);
        EditNodeEdge(t4, p6, p2, node1);
        EditNodeEdge(t1, p2, p3, node1);

        EditNode(node2, p3, p6, p1, t5, t2, node1);
        EditNodeEdge(t5, p1, p6, node2);
        EditNodeEdge(t2, p1, p3, node2);
    }

    void EditNodeEdge(int nodeID, int p1, int p2, int neighbourID) {
        if (nodeID == -1) {
            return;
        }

        for (int x = 0; x < 3; x++) {
            if (nodes[nodeID].points[x] != p1 && nodes[nodeID].points[x] != p2) {
                nodes[nodeID].neighbours[x] = neighbourID;
                break;
            }
        }
    }

    void InitNeighbours() {
        for (int i = 0; i < nodes.size(); i++) {
            // Check neighbours for node i
            for (int idx = 0; idx < 3; idx++) {
                int x = nodes[i].points[idx];
                int y = nodes[i].points[(idx + 1) % 3];
                int neighbour = -1;

                for (int j = 0; j < nodes.size(); j++) {
                    if (j == i) {
                        continue;
                    }

                    int cnt = 0;
                    for (int k = 0; k < 3; k++) {
                        if (nodes[j].points[k] == x ||
                            nodes[j].points[k] == y) {
                            cnt++;
                        }
                    }

                    if (cnt == 2) {
                        neighbour = j;
                        break;
                    }
                }

                nodes[i].neighbours[(idx + 2) % 3] = neighbour;
            }
        }
    }

    void Print()
    {
        cout << points.size() << " " << nodes.size() << endl;
        for (int i = 0; i < points.size(); i++) {
            cout << points[i].x << " " << points[i].y << " " << points[i].z << endl;
        }

        for (int i = 0; i < nodes.size(); i++) {
            for (int x = 0; x < 3; x++) {
                cout << nodes[i].points[x] << " ";
            }

            for (int x = 0; x < 3; x++) {
                cout << nodes[i].neighbours[x] << " ";
            }
            cout << endl;
        }
    }

    void EditNode(int nodeID, int p1, int p2, int p3, int t1, int t2, int t3) {
        nodes[nodeID].points[0] = p1;
        nodes[nodeID].points[1] = p2;
        nodes[nodeID].points[2] = p3;

        nodes[nodeID].neighbours[0] = t1;
        nodes[nodeID].neighbours[1] = t2;
        nodes[nodeID].neighbours[2] = t3;
    }

    int JumpAndWalk(Vector3 point)
    {
        // TODO: start with a rando node
        int nodeId = 0;

        while (!nodes[nodeId].ContainsPoint(point))
        {
            Vector3 p1 = points[nodes[nodeId].points[0]];
            Vector3 p2 = points[nodes[nodeId].points[1]];
            Vector3 p3 = points[nodes[nodeId].points[2]];                       

            Vector3 triangleCenter = p1 * (1.0 / 3) + p2 * (1.0 / 3) + p3 * (1.0 / 3);
            for (int i = 0; i < 3; i++) {
                Vector3 p1 = points[nodes[nodeId].points[(i + 1) % 3]];
                Vector3 p2 = points[nodes[nodeId].points[(i + 2) % 3]];
                if (SegmentIntersect(triangleCenter, point, p1, p2)) {
                    nodeId = nodes[nodeId].neighbours[i];
                    break;
                }
            }

            if (nodeId == -1) {
                return -1;
            }
        }

        return nodeId;
    }
};

// TODO: Move this to a proper cpp file
bool TriangulationNode::ContainsPoint(Vector3 point) {
    Vector3 p1 = triangulation->points[points[0]];
    Vector3 p2 = triangulation->points[points[1]];
    Vector3 p3 = triangulation->points[points[2]];

    return InsideTriangle(p1, p2, p3, point);
}

double TriangulationNode::GetDistanceToPoint(Vector3 point) {
    if (ContainsPoint(point)) {
        return 0;
    }

    Vector3 p1 = triangulation->points[points[0]];
    Vector3 p2 = triangulation->points[points[1]];
    Vector3 p3 = triangulation->points[points[2]];

    double d1 = LinePointDistance(p1, p2, point);
    double d2 = LinePointDistance(p2, p3, point);
    double d3 = LinePointDistance(p1, p3, point);

    return min(d1, min(d2, d3));
}
#endif
