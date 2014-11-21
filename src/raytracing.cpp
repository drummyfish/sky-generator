#include "raytracing.hpp"

void print_point(point_3D point)
  {
    cout << "(" << point.x << ", " << point.y << ", " << point.z << ")" << endl;
  }

double saturate(double value, double min, double max)
  {
    if (value < min)
      return min;

    if (value > max)
      return max;

    return value;
  }

double interpolate_linear(double value1, double value2, double ratio)
  {
    ratio = saturate(ratio,0.0,1.0);
    return ratio * value2 + (1 - ratio) * value1;
  }

void cross_product(point_3D vector1, point_3D vector2, point_3D &final_vector)
  {
    final_vector.x = vector1.y * vector2.z - vector1.z * vector2.y;
    final_vector.y = vector1.z * vector2.x - vector1.x * vector2.z;
    final_vector.z = vector1.x * vector2.y - vector1.y * vector2.x;
  }

void substract_vectors(point_3D vector1, point_3D vector2, point_3D &final_vector)
  {
    final_vector.x = vector1.x - vector2.x;
    final_vector.y = vector1.y - vector2.y;
    final_vector.z = vector1.z - vector2.z;
  }

double point_distance(point_3D a, point_3D b)
  {
    point_3D difference;

    substract_vectors(a,b,difference);

    return sqrt(difference.x * difference.x + difference.y * difference.y + difference.z * difference.z);
  }

double vector_length(point_3D vector)
  {
    return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
  }

double dot_product(point_3D vector1, point_3D vector2)
  {
    return vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z;
  }

void normalize(point_3D &vector)
  {
    double length = vector_length(vector);

    vector.x /= length;
    vector.y /= length;
    vector.z /= length;
  }

double vectors_angle(point_3D vector1, point_3D vector2)
  {
    normalize(vector1);
    normalize(vector2);
    return acos(dot_product(vector1,vector2));
  }

line_3D::line_3D(point_3D point1, point_3D point2)
  {
    this->c0 = point1.x;
    this->q0 = point2.x - point1.x;
    this->c1 = point1.y;
    this->q1 = point2.y - point1.y;
    this->c2 = point1.z;
    this->q2 = point2.z - point1.z;
  }

void revert_vector(point_3D &vector)
  {
    vector.x *= -1;
    vector.y *= -1;
    vector.z *= -1;
  }

void line_3D::get_point(double t, point_3D &point)
  {
    point.x = this->c0 + this->q0 * t;
    point.y = this->c1 + this->q1 * t;
    point.z = this->c2 + this->q2 * t;
  }

double triangle_area(triangle_3D triangle)
  {
    double a_length, b_length, gamma;
    point_3D a_vector, b_vector;

    a_length = point_distance(triangle.a,triangle.b);
    b_length = point_distance(triangle.a,triangle.c);

    substract_vectors(triangle.b,triangle.a,a_vector);
    substract_vectors(triangle.c,triangle.a,b_vector);

    gamma = vectors_angle(a_vector,b_vector);

    return 1/2.0 * a_length * b_length * sin(gamma);
  }

void get_triangle_uvw(triangle_3D triangle, double barycentric_a, double barycentric_b, double barycentric_c, double &u, double &v, double &w)
  {
    u = barycentric_a * triangle.a_t.x + barycentric_b * triangle.b_t.x + barycentric_c * triangle.c_t.x;
    v = barycentric_a * triangle.a_t.y + barycentric_b * triangle.b_t.y + barycentric_c * triangle.c_t.y;
    w = barycentric_a * triangle.a_t.z + barycentric_b * triangle.b_t.z + barycentric_c * triangle.c_t.z;
  }

bool line_3D::intersects_sphere(sphere_3D sphere)
  {
    double a,b,c;

    a = this->q0 * this->q0 + this->q1 * this->q1 + this->q2 * this->q2;
    b = 2 * this->c0 * this->q0 - 2 * sphere.center.x * this->q0 +
        2 * this->c1 * this->q1 - 2 * sphere.center.y * this->q1 +
        2 * this->c2 * this->q2 - 2 * sphere.center.z * this->q2;
    c = this->c0 * this->c0 + this->c1 * this->c1 + this->c2 * this->c2 -
        2 * sphere.center.x * this->c0 - 2 * sphere.center.y * this->c1 - 2 * sphere.center.z * this->c2 +
        sphere.center.x * sphere.center.x + sphere.center.y * sphere.center.y + sphere.center.z * sphere.center.z -
        sphere.radius * sphere.radius;

    return b * b - 4 * a * c >= 0;
  }

bool line_3D::intersects_triangle(triangle_3D triangle, double &a, double &b, double &c)
  {
    point_3D vector1,vector2,vector3,normal;

    a = 0.0;
    b = 0.0;
    c = 0.0;

    substract_vectors(triangle.a,triangle.b,vector1);
    substract_vectors(triangle.a,triangle.c,vector2);

    cross_product(vector1,vector2,normal);

    /*
     Compute general plane equation in form
     qa * x + qb * y + qc * z + d = 0:
     */

    double qa = normal.x;
    double qb = normal.y;
    double qc = normal.z;
    double d = -1 * (qa * triangle.a.x + qb * triangle.a.y + qc * triangle.a.z);

    /* Solve for t: */

    double denominator = (qa * this->q0 + qb * this->q1 + qc * this->q2);

    if (denominator == 0)
      return false;

    double t = (-qa * this->c0 - qb * this->c1 - qc * this->c2 - d) / denominator;

    /* t now contains parameter value for the intersection */

    if (t < 0.0)
      return false;

    point_3D intersection;

    this->get_point(t,intersection);  // intersection in 3D space

    // vectors from the intersection to each triangle vertex:

    substract_vectors(triangle.a,intersection,vector1);
    substract_vectors(triangle.b,intersection,vector2);
    substract_vectors(triangle.c,intersection,vector3);

    point_3D normal1, normal2, normal3;

    // now multiply the vectors to get their normals:

    cross_product(vector1,vector2,normal1);
    cross_product(vector2,vector3,normal2);
    cross_product(vector3,vector1,normal3);

    // if one of the vectors points in other direction than the others, the point is not inside the triangle:

    if (dot_product(normal1,normal2) < 0 || dot_product(normal2,normal3) < 0)
      return false;

    // now compute the barycentric coordinates:

    triangle_3D helper_triangle;
    double total_area;

    total_area = triangle_area(triangle);

    helper_triangle.a = intersection;
    helper_triangle.b = triangle.b;
    helper_triangle.c = triangle.c;
    a = triangle_area(helper_triangle) / total_area;

    helper_triangle.a = triangle.a;
    helper_triangle.b = intersection;
    helper_triangle.c = triangle.c;
    b = triangle_area(helper_triangle) / total_area;

    helper_triangle.a = triangle.a;
    helper_triangle.b = triangle.b;
    helper_triangle.c = intersection;
    c = triangle_area(helper_triangle) / total_area;

    return true;
  }
