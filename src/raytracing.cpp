#include "raytracing.h"


double saturate(double value, double min, double max)
  {
    if (value < min)
      return min;

    if (value > max)
      return max;

    return value;
  }

void make_color(unsigned char color[3],unsigned char r, unsigned char g, unsigned char b)
  {
    color[0] = r;
    color[1] = g;
    color[2] = b;
  }

void blend_colors(unsigned char color1[3], unsigned char color2[3], double ratio)
  {
    color1[0] = interpolate_linear(color1[0],color2[0],ratio);
    color1[1] = interpolate_linear(color1[1],color2[1],ratio);
    color1[2] = interpolate_linear(color1[2],color2[2],ratio);
  }

int saturate_int(int value, int min, int max)
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
    return ratio * (value2 - value1) + value1; // rychlejsi nez ratio * value2 + (1 - ratio) * value1;
  }

double wrap(double value, double min, double max)
  {
    double difference = max - min;

    if (difference < 0)
      return value;

    while (value < min)
      value += difference;

    while (value > max)
      value -= difference;

    return value;
  }

// Metody point3D

// vektorovy soucin
point_3D point_3D::cross_product(point_3D src) {
    return point_3D(this->y * src.z - this->z * src.y,
        this->z * src.x - this->x * src.z,
        this->x * src.y - this->y * src.x);
}

// skalarni soucin
double point_3D::dot_product(point_3D src)
{
    return this->x * src.x + this->y * src.y + this->z * src.z;
}

// vzdalenost bodu od pocatku souradnic
double point_3D::vector_length()
  {
    return sqrt(this->dot_product(*this));
  }

// vzdalenost od druheho bodu
double point_3D::point_distance(point_3D src)
  {
    return (*this - src).vector_length();
  }

// normalizace delky vektoru
void point_3D::normalize()
  {
    double length = this->vector_length();
    this->x /= length;
    this->y /= length;
    this->z /= length;
  }

// uhel mezi vektory
double point_3D::vectors_angle(point_3D src)
  {
    return acos(this->dot_product(src) / this->vector_length() / src.vector_length());
  }

// Metody triangle_3D

// vraci obsah trojuhelnika
double triangle_3D::area()
  {
    double a_length, b_length, gamma;
    point_3D a_vector, b_vector;

    a_length = this->a.point_distance(this->b);
    b_length = this->a.point_distance(this->c);

    a_vector = this->b - this->a;
    b_vector = this->c - this->a;

    gamma = a_vector.vectors_angle(b_vector);

    return 1/2.0 * a_length * b_length * sin(gamma);
  }

// TODO: co to dela?
void triangle_3D::get_uvw(double barycentric_a, double barycentric_b, double barycentric_c, double &u, double &v, double &w)
  {
    u = barycentric_a * this->a_t.x + barycentric_b * this->b_t.x + barycentric_c * this->c_t.x;
    v = barycentric_a * this->a_t.y + barycentric_b * this->b_t.y + barycentric_c * this->c_t.y;
    w = barycentric_a * this->a_t.z + barycentric_b * this->b_t.z + barycentric_c * this->c_t.z;
  }

// Metody line_3D

// konstruktor
line_3D::line_3D(point_3D point1, point_3D point2)
  {
    this->c0 = point1.x;
    this->q0 = point2.x - point1.x;
    this->c1 = point1.y;
    this->q1 = point2.y - point1.y;
    this->c2 = point1.z;
    this->q2 = point2.z - point1.z;
  }

// vraci bod na primce dany parametrem t
point_3D line_3D::get_point(const double t)
  {
    return point_3D(
        this->c0 + this->q0 * t,
        this->c1 + this->q1 * t,
        this->c2 + this->q2 * t);
  }

// vraci true pokud primka protina kouli sphere
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

// vraci smerovy vektor primky
point_3D line_3D::get_vector_to_origin()
  {
    point_3D a,b,result;

    a = this->get_point(0);
    b = this->get_point(1);
    result = b - a;

    result.normalize();
    return result;
  }


bool line_3D::intersects_triangle(triangle_3D triangle, double &a, double &b, double &c, double &t)
  {
    point_3D vector1,vector2,vector3,normal;

    a = 0.0;
    b = 0.0;
    c = 0.0;

    vector1 = triangle.a - triangle.b;
    vector2 = triangle.a - triangle.c;

    normal = vector1.cross_product(vector2);

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

    t = (-qa * this->c0 - qb * this->c1 - qc * this->c2 - d) / denominator;

    /* t now contains parameter value for the intersection */

    if (t < 0.0)
      return false;

    point_3D intersection;

    intersection = this->get_point(t);  // intersection in 3D space

    // vectors from the intersection to each triangle vertex:

    vector1 = triangle.a - intersection;
    vector2 = triangle.b - intersection;
    vector3 = triangle.c - intersection;

    point_3D normal1, normal2, normal3;

    // now multiply the vectors to get their normals:

    normal1 = vector1.cross_product(vector2);
    normal2 = vector2.cross_product(vector3);
    normal3 = vector3.cross_product(vector1);

    // if one of the vectors points in other direction than the others, the point is not inside the triangle:

    if (normal1.dot_product(normal2) < 0 || normal2.dot_product(normal3) < 0)
      return false;

    // now compute the barycentric coordinates:

    double total_area = triangle.area();

    a = triangle_3D(intersection, triangle.b, triangle.c).area()/total_area;
    b = triangle_3D(triangle.a, intersection, triangle.c).area()/total_area;
    c = triangle_3D(triangle.a, triangle.b, intersection).area()/total_area;

    return true;
  }
