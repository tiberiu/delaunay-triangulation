#include "triangulation.hpp"
#include "common.hpp"
#include <iostream>
#include <algorithm>

using namespace std;

// Store the info for an edge of the polygon-hole that needs to be retriangulated
struct PolygonEdge {
    // the neighbouring triangle for this edge from outside the polygon
    int nodeId;

    // Edge
    int p1;
    int p2;
};

Triangulation triangulation;
vector<Vector3> points;
vector<int> queue;
vector<int> badTriangles;
vector<int> goodTriangles;
vector<int> visitedNodes;
vector<PolygonEdge> edges;
vector<pair<int, int>> pointTriangles;

void GenerateSuperTriangle()
{
    // center of the pointset on x calculated as (minX + maxX) / 2
    double x = 0;
    double minX = points[0].x;
    double maxX = points[0].x;
    for (int i = 0; i < points.size(); i++) {
        minX = min(points[i].x, minX);
        maxX = max(points[i].x, maxX);
    }
    x = (minX + maxX) / 2;

    // the super triangle will be the points (x - l, 0), (x + l, 0), (x, l)
    // Here we can use a constant or try some algorithm that sets l depending on the points
    l = 10000;

    Vector3 p1 = Vector3(x, l, 0);
    Vector3 p2 = Vector3(x - l, -l, 0);
    Vector3 p3 = Vector3(x + l, -l, 0);

    int p1ID = triangulation.AddPoint(p1);
    int p2ID = triangulation.AddPoint(p2);
    int p3ID = triangulation.AddPoint(p3);

    TriangulationNode node = TriangulationNode();
    int nodeId = triangulation.AddNode(node);
    triangulation.EditNode(nodeId, p1ID, p2ID, p3ID, -1, -1, -1);
}

// Adds a node to the queue to be checked
// Also marks checks if the node is already added to the queue, and if it isn't
// it adds it and marks it as visited
void AddQueueNode(int nodeId, int p1Id, int p2Id)
{
    if (nodeId != -1 && visitedNodes[nodeId])
    {
        return;
    }

    if (nodeId != -1)
    {
        visitedNodes[nodeId] = 1;
    }

    queue.push_back(nodeId);
}

// Check if a triangle contains the newly added point in it's circumcircle
// If it does it will also add it's neighbours to the queue so they can also be checked later on
bool CheckBadTriangle(int nodeId, int pointId)
{
    if (nodeId == -1) {
        return false;
    }

    int p1Id = triangulation.nodes[nodeId].points[0];
    int p2Id = triangulation.nodes[nodeId].points[1];
    int p3Id = triangulation.nodes[nodeId].points[2];

    Vector3 p1, p2, p3;
    p1 = triangulation.points[p1Id];
    p2 = triangulation.points[p2Id];
    p3 = triangulation.points[p3Id];

    bool insideCircumcircle = InsideTriangleCircumcircle(p1, p2, p3, triangulation.points[pointId]); 

    if (!insideCircumcircle) {
        // not a bad triangle so we don't care
        return false;
    }

    // This is a bad triangle so we need to add it's neighbours to the queue
    badTriangles.push_back(nodeId);
    TriangulationNode node = triangulation.nodes[nodeId];
    AddQueueNode(node.neighbours[0], node.points[1], node.points[2]);
    AddQueueNode(node.neighbours[1], node.points[2], node.points[0]);
    AddQueueNode(node.neighbours[2], node.points[0], node.points[1]);

    return true;
}

// During retriangulation one border point will always have two triangles using it so
// we store this to be able to reconstruct neighbours
void AddPointTriangle(int pointId, int triangleId)
{
    if (pointTriangles[pointId].first == -1) {
        pointTriangles[pointId].first = triangleId;
    } else {
        pointTriangles[pointId].second = triangleId;
    }
}

// Link two newly added triangles that have a common edge
void LinkPointTriangles(int pointId)
{
    int t1 = pointTriangles[pointId].first;
    int t2 = pointTriangles[pointId].second;

    if (t1 == -1 || t2 == -1) {
        return;
    }

    // We know points[0] is the newly added point for both triangles 
    // so we only check points[1] and points[2] to see where we should add the neighbour
    if (triangulation.nodes[t1].points[1] == pointId) {
        triangulation.nodes[t1].neighbours[2] = t2;
    } else {
        triangulation.nodes[t1].neighbours[1] = t2;
    }

    if (triangulation.nodes[t2].points[1] == pointId) {
        triangulation.nodes[t2].neighbours[2] = t1;
    } else {
        triangulation.nodes[t2].neighbours[1] = t1;
    }
}

void AddPointAndRetriangulate(int pointId)
{
    // Find the triangle containint this point
    int nodeId = triangulation.JumpAndWalk(points[pointId]);

    // Starting from this triangle we go through it's neighbours to find all the
    // triangles containing this point in it's circumcircle
    visitedNodes[nodeId] = 1;
    CheckBadTriangle(nodeId, pointId);
    for (int i = 0; i < queue.size(); i++)
    {
        bool bad = CheckBadTriangle(queue[i], pointId);
        if (!bad)
        {
            if (queue[i] != -1) {
                // Mark this triangle as a good triangle
                // This means this is triangle is a good triangle and it has bad triangle as a neighbour
                visitedNodes[queue[i]] = 2;
            }
        }
    }

    // Go through all the bad triangles and see if they have any good neighbours
    for (int i = 0; i < badTriangles.size(); i++) {
        int badTriangle = badTriangles[i];
        for (int x = 0; x < 3; x++) {
            int neighbour = triangulation.nodes[badTriangle].neighbours[x];           
            if (neighbour == -1 || visitedNodes[neighbour] == 2) {
                // Neighbour is a good triangle so add the edge to the list
                PolygonEdge edge = PolygonEdge();
                edge.nodeId = neighbour;
                edge.p1 = triangulation.nodes[badTriangle].points[(x + 1) % 3];
                edge.p2 = triangulation.nodes[badTriangle].points[(x + 2) % 3];

                edges.push_back(edge);
            }
        }
    }

    // Go through the edges of the polygon-hole and add the new triangles
    // We are reusing the old bad-triangles as spots for the new triangles
    int crtPos = 0;  
    for (int i = 0; i < edges.size(); i++) {
        // Add triangle edge.first, edge.second, pointId
        int triangleId;
        if (crtPos >= badTriangles.size())
        {
            // We finished using the badtriangles so we need to add new triangles
            TriangulationNode node = TriangulationNode();
            triangleId = triangulation.AddNode(node);
        } else {
            triangleId = badTriangles[crtPos];
            crtPos++;
        }

        int p1 = edges[i].p1;
        int p2 = edges[i].p2;
        triangulation.EditNode(triangleId, pointId, p1, p2, edges[i].nodeId, -1, -1);

        // Add the neigbour from the outer edge
        int neighbourId = edges[i].nodeId;
        if (neighbourId != -1)
        {
            for (int x = 0; x < 3; x++) {
                if (triangulation.nodes[neighbourId].points[x] != p1 &&
                    triangulation.nodes[neighbourId].points[x] != p2) {
                    triangulation.nodes[neighbourId].neighbours[x] = triangleId;
                }
            }
        }

        // Store that this triangle uses the edge p1-point and p2-point
        // This will be used to link the newly added triangles among them as neighbours
        AddPointTriangle(p1, triangleId);
        AddPointTriangle(p2, triangleId);
    }

    for (int i = 0; i < edges.size(); i++) {
        // For every edge from point to a edgepoint we link the two triangles using that edge as neighbours
        LinkPointTriangles(edges[i].p1);
        LinkPointTriangles(edges[i].p2);

        // Also clean after ourselves
        pointTriangles[edges[i].p1] = make_pair(-1, -1);
        pointTriangles[edges[i].p2] = make_pair(-1, -1);
    }

    // Cleanup (note we only clean what we used, otherwise we increase time complexity to N^2)
    for (int i = 0; i < queue.size(); i++) {
        if (queue[i] == -1) {
            continue;
        }

        visitedNodes[queue[i]] = 0;
    }

    visitedNodes[nodeId] = 0;
    badTriangles.clear();
    edges.clear();
    queue.clear();
}

int main() {
    freopen("data/delaunay.in", "r", stdin);
    freopen("data/delaunay_bowyerwatson.out", "w", stdout);

    // Read the N input points
    int N;
    cin >> N;
    for (int i = 0; i < N; i++) {
        double x, y;        
        cin >> x >> y;
        points.push_back(Vector3(x, y, 0));
    }


    triangulation = Triangulation(points);
    GenerateSuperTriangle();

    // Add points to the triangulation
    visitedNodes.resize(N * 3);
    pointTriangles.resize(N + 3);
    for (int i = 0; i < pointTriangles.size(); i++)
    {
        pointTriangles[i] = make_pair(-1, -1);
    }

    int pointsToAdd = N;
    for (int i = 0; i < pointsToAdd; i++) {
        AddPointAndRetriangulate(i);
    }

    // Remove Supertriangle points
    triangulation.RemovePoint(triangulation.points.size() - 1);
    triangulation.RemovePoint(triangulation.points.size() - 1);
    triangulation.RemovePoint(triangulation.points.size() - 1);

    triangulation.Print();

    return 0;
}
