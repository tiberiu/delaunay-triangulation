#ifndef __COMMON__H
#define __COMMON__H

#include <vector>
#include <iostream>
#include <cmath>

const double EPS = 0.0001;

using namespace std;

// 3D Vector class
class Vector3 {
public:
    double x, y, z;

    Vector3() : x(0), y(0), z(0) {};
    Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};

    string ToString() {
        char buff[100];
        sprintf(buff, "(%.2f, %.2f, %.2f)", x, y, z);
        std::string str = buff;
        
        return str;
    }
};

Vector3 operator+(Vector3 lhs, const Vector3& rhs)
{
    return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

Vector3 operator*(Vector3 lhs, const double alpha)
{
    return Vector3(lhs.x * alpha, lhs.y * alpha, lhs.z * alpha);
}


// Returns the distance between two points
double GetDistance(Vector3 p1, Vector3 p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

/* Return value can be interpreted as:
 * - if negative points are in clocwise order, counterclockwise order otherwise
 * - absolute value is double the area of the triangle defined by p1, p2 and p3
 */
double det(Vector3 p1, Vector3 p2, Vector3 p3)
{
    return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y -
           p1.x * p3.y - p3.x * p2.y - p2.x * p1.y;
}

// Comparison method for sorting points by x, and by y in case of equality
int ConvexHullCMP(pair<Vector3, int> p1, pair<Vector3, int> p2)
{
    if (p1.first.x == p2.first.x) {
        return p1.first.y < p2.first.y;
    }

    return p1.first.x < p2.first.x;
}

// Returns a vector of indices of the points on the convex hull of the points 
// received as argument
vector<int> ComputeConvexHull(vector<Vector3> points)
{
    vector<pair<Vector3, int>> sortedPoints;
    for (int i = 0; i < points.size(); i++) {
        sortedPoints.push_back(make_pair(points[i], i));
    }

    sort(sortedPoints.begin(), sortedPoints.end(), ConvexHullCMP);

    vector<int> st;
    // Bottom Part of convex hull
    int stackLimit = 2;
    for (int i = 0; i < sortedPoints.size(); i++) {
        bool ccwAngle = false;
        while (!ccwAngle && st.size() >= stackLimit)
        {
            ccwAngle = true;
            Vector3 p1 = sortedPoints[st[st.size() - 2]].first;
            Vector3 p2 = sortedPoints[st[st.size() - 1]].first;
            Vector3 p3 = sortedPoints[i].first;

            if (det(p1, p2, p3) < EPS) {
                st.pop_back();
                ccwAngle = false;
            }
        }

        st.push_back(i); 
    }

    // This is to make sure we don't remove points from the bottom part
    stackLimit = st.size() + 1;
    // Top part of convex hull
    for (int i = sortedPoints.size() - 2; i >= 0; i--) {
        bool ccwAngle = false;
        while (!ccwAngle && st.size() >= stackLimit)
        {
            ccwAngle = true;
            Vector3 p1 = sortedPoints[st[st.size() - 2]].first;
            Vector3 p2 = sortedPoints[st[st.size() - 1]].first;
            Vector3 p3 = sortedPoints[i].first;

            if (det(p1, p2, p3) < EPS) {
                st.pop_back();
                ccwAngle = false;
            }
        }
        st.push_back(i); 
    }

    // First element is added twice so pop it
    st.pop_back();

    vector<int> ret;
    for (int i = 0; i < st.size(); i++) {
        ret.push_back(sortedPoints[st[i]].second);
    }
    return ret;
}

// Checks if point p is inside triangle p1, p2, p3 by using barycentric coordinates
bool InsideTriangle(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p)
{
    double alpha = ((p2.y - p3.y)*(p.x - p3.x) + (p3.x - p2.x)*(p.y - p3.y)) /
                  ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));
    double beta = ((p3.y - p1.y)*(p.x - p3.x) + (p1.x - p3.x)*(p.y - p3.y)) /
                 ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));
    double gamma = 1.0f - alpha - beta;

    if (alpha >= -EPS && beta >= -EPS && gamma >= -EPS) {
        return true;
    } else {
        return false;
    }
}

// Returns the distance from a point to a line by computing the area of the triangle in 2 ways
// to find the height of the triangle considering (p1, p2) as base
double LinePointDistance(Vector3 p1, Vector3 p2, Vector3 point) {
    double d1 = GetDistance(p1, point);
    double d2 = GetDistance(p2, point);

    double baseLength = GetDistance(p1, p2);
    double doubleArea = fabs(det(p1, p2, point));
    double d3 = doubleArea / baseLength;

    return min(d1, min(d2, d3));
}

// Check if point is inside the circumcircle of the triangle (p1, p2, p3)
bool InsideTriangleCircumcircle(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 point) {
    double d11, d12, d13, d21, d22, d23, d31, d32, d33;

    if (det(p1, p2, p3) < EPS) {
        Vector3 aux = p1;
        p1 = p2;
        p2 = aux;
    }

    d11 = p1.x - point.x;
    d12 = p1.y - point.y;
    d13 = (p1.x * p1.x - point.x * point.x) + (p1.y * p1.y - point.y * point.y);

    d21 = p2.x - point.x;
    d22 = p2.y - point.y;
    d23 = (p2.x * p2.x - point.x * point.x) + (p2.y * p2.y - point.y * point.y);

    d31 = p3.x - point.x;
    d32 = p3.y - point.y;
    d33 = (p3.x * p3.x - point.x * point.x) + (p3.y * p3.y - point.y * point.y);

    double detVal = (d11 * d22 * d33) + (d21 * d32 * d13) + (d31 * d12 * d23) -
                   (d11 * d32 * d23) - (d31 * d22 * d13) - (d21 * d12 * d33);

    return detVal > EPS;
}

bool BoundingBoxIntersect(Vector3 b1, Vector3 b2, Vector3 b3, Vector3 b4)
{
    if ((b1.x < b4.x && b2.x > b3.x) &&
        (b1.y < b4.y && b2.y > b3.y)) {
        return true;
    }

    return false;
}

bool SegmentIntersect(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)
{
    // Check bounding box
    Vector3 b1 = Vector3(min(p1.x, p2.x), min(p1.y, p2.y), min(p1.z, p2.z));
    Vector3 b2 = Vector3(max(p1.x, p2.x), max(p1.y, p2.y), max(p1.z, p2.z));

    Vector3 b3 = Vector3(min(p3.x, p4.x), min(p3.y, p4.y), min(p3.z, p4.z));
    Vector3 b4 = Vector3(max(p3.x, p4.x), max(p3.y, p4.y), max(p3.z, p4.z));

    if (!BoundingBoxIntersect(b1, b2, b3, b4)) {
        return false;
    }

    float d1 = det(p3, p4, p1);
    float d2 = det(p3, p4, p2);
    float d3 = det(p1, p2, p3);
    float d4 = det(p1, p2, p4);

    if (d1 * d2 < EPS && d3 * d4 < EPS) {
        return true;
    }

    return false;
}

#endif
