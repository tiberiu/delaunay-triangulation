#include "triangulation.hpp"
#include "common.hpp"
#include <iostream>
#include <algorithm>

using namespace std;

struct QueueNode {
    int nodeId;

    // Edge
    int p1;
    int p2;
};

Triangulation triangulation;
vector<Vector3> points;
vector<QueueNode> queue;
vector<int> badTriangles;
vector<int> goodTriangles;
vector<int> visitedNodes;
vector<QueueNode> edges;
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
    // We will double l until all the points are inside this triangle
    double l = 2;
    bool allPointsInside = false;
    while (!allPointsInside)
    {
        Vector3 p1 = Vector3(x, l, 0);
        Vector3 p2 = Vector3(x - l, 0, 0);
        Vector3 p3 = Vector3(x + l, 0, 0);
        allPointsInside = true;
        for (int i = 0; i < points.size(); i++)
        {
            if (!InsideTriangle(p1, p2, p3, points[i]))
            {
                allPointsInside = false;
                break;
            }
        }

        if (!allPointsInside) {
            l = l * 2;
        }
    }

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

void AddQueueNode(int nodeId, int p1Id, int p2Id)
{
    if (nodeId != -1 && visitedNodes[nodeId])
    {
        return;
    }

    QueueNode qNode = QueueNode();
    qNode.nodeId = nodeId;
    qNode.p1 = p1Id;
    qNode.p2 = p2Id;

    if (nodeId != -1)
    {
        visitedNodes[nodeId] = 1;
    }


    queue.push_back(qNode);
}

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
    bool insideTriangle = InsideTriangle(p1, p2, p3, triangulation.points[pointId]);

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

void AddPointTriangle(int pointId, int triangleId)
{
    if (pointTriangles[pointId].first == -1) {
        pointTriangles[pointId].first = triangleId;
    } else {
        pointTriangles[pointId].second = triangleId;
    }
}

void LinkPointTriangles(int pointId)
{
    int t1 = pointTriangles[pointId].first;
    int t2 = pointTriangles[pointId].second;


    if (t1 == -1 || t2 == -1) {
        return;
    }

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
    int nodeId = 0;
    for (int i = 0; i < triangulation.nodes.size(); i++) {
        if (triangulation.nodes[i].ContainsPoint(points[pointId]))
        {
           nodeId = i;
           break;
        }
    }

    queue.clear();
    badTriangles.clear();
    edges.clear();
    for (int i = 0; i < visitedNodes.size(); i++) {
        visitedNodes[i] = 0;
    }

    // This will definetly return true
    visitedNodes[nodeId] = 1;
    CheckBadTriangle(nodeId, pointId);
    for (int i = 0; i < queue.size(); i++)
    {
        bool bad = CheckBadTriangle(queue[i].nodeId, pointId);
        if (!bad)
        {
            // Add to edge list
            if (queue[i].nodeId != -1) {
                visitedNodes[queue[i].nodeId] = 2;
            }
        }
    }

    for (int i = 0; i < badTriangles.size(); i++) {
        int badTriangle = badTriangles[i];
        for (int x = 0; x < 3; x++) {
            int neighbour = triangulation.nodes[badTriangle].neighbours[x];           
            if (neighbour == -1 || visitedNodes[neighbour] == 2) {
                // Neighbour is a good triangle so add the edge to the list
                QueueNode qNode = QueueNode();
                qNode.nodeId = neighbour;
                qNode.p1 = triangulation.nodes[badTriangle].points[(x + 1) % 3];
                qNode.p2 = triangulation.nodes[badTriangle].points[(x + 2) % 3];

                edges.push_back(qNode);
            }
        }
    }

    int crtPos = 0;
    for (int i = 0; i < edges.size(); i++) {
        // Add triangle edge.first, edge.second, pointId
        int triangleId;
        if (crtPos >= badTriangles.size())
        {
            TriangulationNode node = TriangulationNode();
            triangleId = triangulation.AddNode(node);
        } else {
            triangleId = badTriangles[crtPos];
            crtPos++;
        }

        int p1 = edges[i].p1;
        int p2 = edges[i].p2;
        triangulation.EditNode(triangleId, pointId, p1, p2, edges[i].nodeId, -1, -1);
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

        AddPointTriangle(p1, triangleId);
        AddPointTriangle(p2, triangleId);
    }

    for (int i = 0; i < edges.size(); i++) {
        LinkPointTriangles(edges[i].p1);
        LinkPointTriangles(edges[i].p2);
        pointTriangles[edges[i].p1] = make_pair(-1, -1);
        pointTriangles[edges[i].p2] = make_pair(-1, -1);
    }

    for (int i = 0; i < edges.size(); i++) {

        if (edges[i].nodeId == -1) {
            continue;
        }

        visitedNodes[edges[i].nodeId] = 0;
    }
    visitedNodes[nodeId] = 0;
    badTriangles.clear();
    edges.clear();
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
