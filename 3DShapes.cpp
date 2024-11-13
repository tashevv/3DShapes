#include <stdio.h>
#include <cmath>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

// Struct for a point in 3D space
struct Point3D {
    float x, y, z;
};

// Base class for all shapes
class Shape {
public:
    virtual void draw(float A, float B, float* z, char* b, const char* shades, Point3D lightDir, float offsetX, float offsetY, int width, int height) = 0;
    const float M_PI = 3.14159265358979323846;

protected:
    // Rotates a point by angles A (Y-axis) and B (X-axis)
    Point3D rotate_point(Point3D p, float A, float B) {
        float cosA = cos(A), sinA = sin(A), cosB = cos(B), sinB = sin(B);
        float nx = p.x * cosB - p.z * sinB;
        float ny = p.x * sinA * sinB + p.y * cosA + p.z * sinA * cosB;
        float nz = p.x * cosA * sinB - p.y * sinA + p.z * cosA * cosB;
        return { nx, ny, nz };
    }

    // Projects a 3D point to 2D screen coordinates
    void project_point(Point3D p, int& x, int& y, float& D, float offsetX, float offsetY, int width, int height) {
        D = 1 / (p.z + 5);
        x = offsetX + 30 * D * p.x;  // Apply horizontal offset
        y = offsetY + height / 2 + 15 * D * p.y;
    }

    // Draws a line between two points in 3D space
    void draw_line(Point3D p1, Point3D p2, float* z, char* b, char ch, float offsetX, float offsetY, int width, int height) {
        int steps = 20;
        for (int i = 0; i <= steps; i++) {
            float t = (float)i / steps;
            Point3D p = {
                p1.x * (1 - t) + p2.x * t,
                p1.y * (1 - t) + p2.y * t,
                p1.z * (1 - t) + p2.z * t
            };

            float D;
            int x, y;
            project_point(p, x, y, D, offsetX, offsetY, width, height);

            int o = x + width * y;
            if (y >= 0 && y < height && x >= 0 && x < width && D > z[o]) {
                z[o] = D;
                b[o] = ch;
            }
        }
    }

    // Fills a face of the shape using shading based on the face normal
    void fill_face(Point3D p1, Point3D p2, Point3D p3, Point3D p4, float* z, char* b, char shadeChar, float offsetX, float offsetY, int width, int height) {
        int steps = 20;
        for (int i = 0; i <= steps; i++) {
            for (int j = 0; j <= steps; j++) {
                float u = (float)i / steps;
                float v = (float)j / steps;

                Point3D p = {
                    p1.x * (1 - u) * (1 - v) + p2.x * u * (1 - v) + p3.x * u * v + p4.x * (1 - u) * v,
                    p1.y * (1 - u) * (1 - v) + p2.y * u * (1 - v) + p3.y * u * v + p4.y * (1 - u) * v,
                    p1.z * (1 - u) * (1 - v) + p2.z * u * (1 - v) + p3.z * u * v + p4.z * (1 - u) * v
                };

                float D;
                int x, y;
                project_point(p, x, y, D, offsetX, offsetY, width, height);

                int o = x + width * y;
                if (y >= 0 && y < height && x >= 0 && x < width && D > z[o]) {
                    z[o] = D;
                    b[o] = shadeChar;
                }
            }
        }
    }
};

// Derived class for Cube shape
class Cube : public Shape {
public:
    Cube(float scale = 1.0f) : scale(scale) {
        vertices = {
            {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},  // Bottom square (base)
            {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}       // Top square (top)
        };

        faces = {
            {0, 1, 2, 3},  // Bottom
            {4, 5, 6, 7},  // Top
            {0, 1, 5, 4},  // Front
            {1, 2, 6, 5},  // Right
            {2, 3, 7, 6},  // Back
            {3, 0, 4, 7}   // Left
        };
    }

    void set_scale(float new_scale) {
        scale = new_scale;
    }

    void draw(float A, float B, float* z, char* b, const char* shades, Point3D lightDir, float offsetX, float offsetY, int width, int height) override {
        std::vector<Point3D> rotatedVertices;
        for (auto& vertex : vertices) {
            // Apply scaling to each vertex
            Point3D scaledVertex = { vertex.x * scale, vertex.y * scale, vertex.z * scale };
            rotatedVertices.push_back(rotate_point(scaledVertex, A, B));
        }

        for (auto& face : faces) {
            Point3D p1 = rotatedVertices[face[0]];
            Point3D p2 = rotatedVertices[face[1]];
            Point3D p3 = rotatedVertices[face[2]];
            Point3D p4 = rotatedVertices[face[3]];

            Point3D u = { p2.x - p1.x, p2.y - p1.y, p2.z - p1.z };
            Point3D v = { p3.x - p1.x, p3.y - p1.y, p3.z - p1.z };
            Point3D normal = {
                u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x
            };
            float normalLength = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            normal = { normal.x / normalLength, normal.y / normalLength, normal.z / normalLength };

            float lightIntensity = normal.x * lightDir.x + normal.y * lightDir.y + normal.z * lightDir.z;
            int N = (int)((lightIntensity + 1) * 5.5);
            if (N < 0) N = 0;
            if (N > 11) N = 11;

            char shadeChar = shades[N];
            fill_face(p1, p2, p3, p4, z, b, shadeChar, offsetX, offsetY, width, height);
        }
    }

private:
    std::vector<Point3D> vertices;
    std::vector<std::vector<int>> faces;
    float scale; // Scaling factor
};

// Derived class for Pyramid shape
class Pyramid : public Shape {
public:
    Pyramid(float scale = 1.0f) : scale(scale) {
        vertices = {
            {0.0, 0.0, 1},
            {-1, -1, -1},
            {1, -1, -1},
            {1, 1, -1},
            {-1, 1, -1}
        };

        faces = {
            {0, 1, 2},
            {0, 2, 3},
            {0, 3, 4},
            {0, 4, 1}
        };
    }

    void set_scale(float new_scale) {
        scale = new_scale;
    }

    void draw(float A, float B, float* z, char* b, const char* shades, Point3D lightDir, float offsetX, float offsetY, int width, int height) override {
        std::vector<Point3D> rotatedVertices;
        for (auto& vertex : vertices) {
            // Apply scaling to each vertex
            Point3D scaledVertex = { vertex.x * scale, vertex.y * scale, vertex.z * scale };
            rotatedVertices.push_back(rotate_point(scaledVertex, A, B));
        }

        for (auto& face : faces) {
            Point3D p1 = rotatedVertices[face[0]];
            Point3D p2 = rotatedVertices[face[1]];
            Point3D p3 = rotatedVertices[face[2]];

            Point3D u = { p2.x - p1.x, p2.y - p1.y, p2.z - p1.z };
            Point3D v = { p3.x - p1.x, p3.y - p1.y, p3.z - p1.z };
            Point3D normal = {
                u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x
            };
            float normalLength = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            normal = { normal.x / normalLength, normal.y / normalLength, normal.z / normalLength };

            float lightIntensity = normal.x * lightDir.x + normal.y * lightDir.y + normal.z * lightDir.z;
            int N = (int)((lightIntensity + 1) * 5.5);
            if (N < 0) N = 0;
            if (N > 11) N = 11;

            char shadeChar = shades[N];
            fill_face(p1, p2, p3, p3, z, b, shadeChar, offsetX, offsetY, width, height);
        }
    }

private:
    std::vector<Point3D> vertices;
    std::vector<std::vector<int>> faces;
    float scale; // Scaling factor
};

// Derived class for Sphere shape
class Sphere : public Shape {
public:
    Sphere(int latitudeDivisions = 10, int longitudeDivisions = 10, float scale = 1.0f) : scale(scale) {
        // Generate vertices using spherical coordinates
        for (int i = 0; i <= latitudeDivisions; ++i) {
            float theta = M_PI * float(i) / float(latitudeDivisions); // Latitude angle
            for (int j = 0; j <= longitudeDivisions; ++j) {
                float phi = 2.0f * M_PI * float(j) / float(longitudeDivisions); // Longitude angle
                Point3D p = {
                    sin(theta) * cos(phi), // x = sin(theta) * cos(phi)
                    sin(theta) * sin(phi), // y = sin(theta) * sin(phi)
                    cos(theta)              // z = cos(theta)
                };
                vertices.push_back(p);
            }
        }

        // Generate faces (each face is a quad between four vertices)
        for (int i = 0; i < latitudeDivisions; ++i) {
            for (int j = 0; j < longitudeDivisions; ++j) {
                int first = i * (longitudeDivisions + 1) + j;
                int second = first + longitudeDivisions + 1;
                faces.push_back({ first, second, first + 1 });
                faces.push_back({ second, second + 1, first + 1 });
            }
        }
    }

    void set_scale(float new_scale) {
        scale = new_scale;
    }

    void draw(float A, float B, float* z, char* b, const char* shades, Point3D lightDir, float offsetX, float offsetY, int width, int height) override {
        std::vector<Point3D> rotatedVertices;
        for (auto& vertex : vertices) {
            // Scale the vertex coordinates before rotating
            Point3D scaledVertex = {
                vertex.x * scale,
                vertex.y * scale,
                vertex.z * scale
            };
            rotatedVertices.push_back(rotate_point(scaledVertex, A, B));
        }

        for (auto& face : faces) {
            Point3D p1 = rotatedVertices[face[0]];
            Point3D p2 = rotatedVertices[face[1]];
            Point3D p3 = rotatedVertices[face[2]];

            Point3D u = { p2.x - p1.x, p2.y - p1.y, p2.z - p1.z };
            Point3D v = { p3.x - p1.x, p3.y - p1.y, p3.z - p1.z };
            Point3D normal = {
                u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x
            };
            float normalLength = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            normal = { normal.x / normalLength, normal.y / normalLength, normal.z / normalLength };

            float lightIntensity = normal.x * lightDir.x + normal.y * lightDir.y + normal.z * lightDir.z;
            int N = (int)((lightIntensity + 1) * 5.5);
            if (N < 0) N = 0;
            if (N > 11) N = 11;

            char shadeChar = shades[N];
            fill_face(p1, p2, p3, p3, z, b, shadeChar, offsetX, offsetY, width, height);
        }
    }

private:
    std::vector<Point3D> vertices;
    std::vector<std::vector<int>> faces;
    float scale; // Scaling factor
};

int main() {
    // Adjustable screen size
    int width = 100, height = 40;

    float A = 0, B = 0;
    int k;

    // Dynamically allocate the buffers based on screen size
    float* z = new float[width * height];
    char* b = new char[width * height];

    printf("\x1b[2J");

    Pyramid pyramid(1.5f);
    Cube cube(1.5f);
    Sphere sphere(10, 10, 2.0f);

    Point3D lightDir = { 0, 0, -1 };
    const char* shades = ".,-~:;=!*#$@";

    for (;;) {
        memset(b, 32, width * height);  // Clear the buffer
        memset(z, 0, width * height * sizeof(float));  // Clear the depth buffer

        // Draw the cube on the left
        cube.draw(A, B, z, b, shades, lightDir, 25, -10, width, height);
        // Draw the pyramid on the right with an offset
        pyramid.draw(A, B, z, b, shades, lightDir, 50, 10, width, height);
        // Draw the sphere on the right with an offset
        sphere.draw(A, B, z, b, shades, lightDir, 75, -10, width, height);

        // Display the result
        printf("\x1b[H");
        for (k = 0; k < width * height; k++) {
            putchar(k % width ? b[k] : 10);
        }

        A += 0.04;
        B += 0.02;
    }

    delete[] z;
    delete[] b;

    return 0;
}
