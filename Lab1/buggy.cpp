#include <iostream>

struct Point
{
    int x, y;

    Point() : x(), y() {}
    Point(int _x, int _y) : x(_x), y(_y) {}
};

class Shape
{
private:
    int vertices;
    Point **points;

public:
    Shape(int _vertices)
    {
        vertices = _vertices;
        points = new Point *[vertices];
        for (int i = 0; i < vertices; i++)
            points[i] = new Point();
    }

    ~Shape()
    {
        for (int i = 0; i < vertices; i++)
        {
            delete points[i];
        }

        delete points;
    }

    void addPoints(Point pts[])
    {
        for (int i = 0; i < vertices; i++)
        {
            *points[i] = pts[i];
        }
    }

    double area()
    {
        int temp = 0;
        for (int i = 0; i < vertices - 1; i++)
        {
            int lhs = points[i]->x * points[i + 1]->y;
            int rhs = (*points[i + 1]).x * (*points[i]).y;
            temp += (lhs - rhs);
        }
        double area = abs(temp) / 2.0;
        return area;
    }
};

int main()
{
    Point tri1 = Point(0, 0);
    Point tri2 = Point(1, 2);
    Point tri3 = Point(2, 0);

    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape *tri = new Shape(3);
    tri->addPoints(triPts);

    Point quad1 = Point(0, 0);
    Point quad2 = Point(0, 2);
    Point quad3 = Point(2, 2);
    Point quad4 = Point(2, 0);

    // adding points to quad
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape *quad = new Shape(4);
    quad->addPoints(quadPts);

    std::cout << "Triangle area: " << tri->area() << "\n";
    std::cout << "Quadrilateral area: " << quad->area() << "\n";

    delete tri;
    delete quad;
}
