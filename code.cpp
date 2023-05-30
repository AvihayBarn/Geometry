#include "pch.h"
#include <iostream>
#include <fstream>
#include <random>
#include <vector>

using namespace std;

class facet;

class point {
public:
    int x, y, z, id;
    vector<facet*> conflict;
};

class facet {
public:
    point* v[3];
    facet* neighbor[3];
    bool alive;
    vector<point*> conflict;
    int id;
    static int count;

    facet() {
        id = count++;
    }
};

int facet::count = 0;

class horizon_edge {
public:
    point* v1;
    point* v2;
    facet* gray_f;
    facet* white_f;
    int v1_index_in_white_f;
};

// Global variables:
int n;
point* pts;
int* pointorder;
vector<facet*> facets;

void get_random_point_order(int n, int* pointorder) {
    for (int i = 0; i < n; ++i) {
        pointorder[i] = i;
    }

    // Shuffle the point order using Fisher-Yates algorithm
    for (int i = n - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        swap(pointorder[i], pointorder[j]);
    }
}

int orient(point* p1, point* p2, point* p3, point* p4) {
    int determinant = p1->x * (p2->y * p3->z - p2->z * p3->y) -
                      p1->y * (p2->x * p3->z - p2->z * p3->x) +
                      p1->z * (p2->x * p3->y - p2->y * p3->x) -
                      p2->z * (p3->x * p4->y - p3->y * p4->x) +
                      p2->y * (p3->x * p4->z - p3->z * p4->x) -
                      p2->x * (p3->y * p4->z - p3->z * p4->y) +
                      p3->x * (p4->y * p1->z - p4->z * p1->y) -
                      p3->y * (p4->x * p1->z - p4->z * p1->x) +
                      p3->z * (p4->x * p1->y - p4->y * p1->x) -
                      p4->x * (p1->y * p2->z - p1->z * p2->y) +
                      p4->y * (p1->x * p2->z - p1->z * p2->x) -
                      p4->z * (p1->x * p2->y - p1->y * p2->x);

    if (determinant > 0)
        return 1;   // Points are in counter-clockwise order
    else if (determinant < 0)
        return -1;  // Points are in clockwise order
    else
        return 0;   // Points are collinear
}

bool facet_point_conflict(facet* f, point* p) {
    int orientation = orient(f->v[0], f->v[1], f->v[2], p);

    return orientation == -1;
}

void create_simplex() {
    // Create the initial tetrahedron
    facet* f1 = new facet();
    facet* f2 = new facet();
    facet* f3 = new facet();
    facet* f4 = new facet();

    f1->v[0] = &pts[0];
    f1->v[1] = &pts[1];
    f1->v[2] = &pts[2];

    f2->v[0] = &pts[0];
    f2->v[1] = &pts[1];
    f2->v[2] = &pts[3];

    f3->v[0] = &pts[0];
    f3->v[1] = &pts[2];
    f3->v[2] = &pts[3];

    f4->v[0] = &pts[1];
    f4->v[1] = &pts[2];
    f4->v[2] = &pts[3];

    f1->neighbor[0] = f4;
    f1->neighbor[1] = f3;
    f1->neighbor[2] = f2;

    f2->neighbor[0] = f4;
    f2->neighbor[1] = f1;
    f2->neighbor[2] = f3;

    f3->neighbor[0] = f4;
    f3->neighbor[1] = f2;
    f3->neighbor[2] = f1;

    f4->neighbor[0] = f1;
    f4->neighbor[1] = f3;
    f4->neighbor[2] = f2;

    f1->alive = true;
    f2->alive = true;
    f3->alive = true;
    f4->alive = true;

    facets.push_back(f1);
    facets.push_back(f2);
    facets.push_back(f3);
    facets.push_back(f4);
}

bool compare_facets_for_sorting(facet* f1, facet* f2) {
    // Compare the vertices' indices in f1 and f2
    for (int i = 0; i < 3; ++i) {
        int v1_id = f1->v[i]->id;
        int v2_id = f2->v[i]->id;

        if (v1_id < v2_id)
            return true;
        else if (v1_id > v2_id)
            return false;
    }

    return false;  // Facets are equal
}

void print_polytope() {
    for (facet* f : facets) {
        cout << "Facet " << f->id << ": ";
        for (int i = 0; i < 3; ++i) {
            cout << f->v[i]->id << " ";
        }
        cout << endl;
    }
}

void print_he(horizon_edge he) {
    cout << "Horizon Edge: ";
    cout << "(" << he.v1->id << ", " << he.v2->id << ") ";
    cout << "Gray Facet: " << he.gray_f->id << " ";
    cout << "White Facet: " << he.white_f->id << " ";
    cout << "v1 Index in White Facet: " << he.v1_index_in_white_f << endl;
}

int main() {
    ifstream inputFile("input.txt");
    if (!inputFile) {
        cout << "Failed to open input file." << endl;
        return 0;
    }

    inputFile >> n;

    pts = new point[n];
    pointorder = new int[n];

    for (int i = 0; i < n; ++i) {
        inputFile >> pts[i].x >> pts[i].y >> pts[i].z;
        pts[i].id = i;
    }

    inputFile.close();

    get_random_point_order(n, pointorder);
    create_simplex();

    for (int i = 0; i < n; ++i) {
        point* p = &pts[pointorder[i]];

        for (facet* f : facets) {
            if (facet_point_conflict(f, p)) {
                f->conflict.push_back(p);
                p->conflict.push_back(f);
            }
        }

        vector<horizon_edge> horizonEdges;

        for (facet* f : p->conflict) {
            for (int j = 0; j < 3; ++j) {
                int k = (j + 1) % 3;
                facet* neighbor = f->neighbor[j];

                if (!neighbor->alive)
                    continue;

                if (facet_point_conflict(neighbor, p)) {
                    horizon_edge he;
                    he.v1 = f->v[j];
                    he.v2 = f->v[k];
                    he.gray_f = f;
                    he.white_f = neighbor;
                    he.v1_index_in_white_f = k;
                    horizonEdges.push_back(he);
                }
            }
        }

        for (horizon_edge he : horizonEdges) {
            int v1_index = he.v1_index_in_white_f;
            int v2_index = (v1_index + 1) % 3;

            facet* new_f = new facet();
            new_f->v[0] = he.v2;
            new_f->v[1] = he.v1;
            new_f->v[2] = p;

            new_f->neighbor[0] = he.gray_f;
            new_f->neighbor[1] = he.white_f;
            new_f->neighbor[2] = nullptr;

            he.gray_f->neighbor[v1_index] = new_f;

            for (int i = 0; i < 3; ++i) {
                facet* neighbor = he.white_f->neighbor[i];

                if (neighbor == he.gray_f) {
                    he.white_f->neighbor[i] = new_f;
                    break;
                }
            }

            facets.push_back(new_f);
        }

        for (facet* f : p->conflict) {
            f->alive = false;
        }

        p->conflict.clear();
    }

    ofstream outputFile("output.txt");
    if (!outputFile) {
        cout << "Failed to open output file." << endl;
        return 0;
    }

    sort(facets.begin(), facets.end(), compare_facets_for_sorting);

    for (facet* f : facets) {
        outputFile << f->v[0]->id << " " << f->v[1]->id << " " << f->v[2]->id << endl;
    }

    outputFile.close();

    cout << "Convex hull generated successfully." << endl;

    delete[] pts;
    delete[] pointorder;

    return 0;
}





