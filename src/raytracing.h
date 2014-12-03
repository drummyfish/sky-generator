#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <iostream>
#include <math.h>
#include <vector>

using namespace std;

/**<
 General stuff for raytracing.
 */

#define PI 3.1415926535897932384626

struct point_3D        /**< point, also a vector */
  {
    double x;
    double y;
    double z;

    // konstruktory
    point_3D() : x(0.0), y(0.0), z(0.0) {};
    point_3D(double x, double y, double z) : x(x), y(y), z(z) {};

    point_3D cross_product(point_3D);
    double dot_product(point_3D);
    double vector_length();
    void normalize();
    double point_distance(point_3D);
    double vectors_angle(point_3D);

    point_3D operator - (point_3D src) { return point_3D(this->x - src.x, this->y - src.y, this->z - src.z); }
  };

typedef struct         /**< point, also a vector */
  {
    point_3D center;
    double radius;
  } sphere_3D;

struct triangle_3D
  {
    point_3D a;        /**< position coordinates */
    point_3D b;
    point_3D c;

    point_3D a_t;      /**< texturing coordinates */
    point_3D b_t;
    point_3D c_t;

    triangle_3D(point_3D a, point_3D b, point_3D c) : a(a), b(b), c(c) {}
    triangle_3D(point_3D a, point_3D b, point_3D c,
    point_3D a_t, point_3D b_t, point_3D c_t) : a(a), b(b), c(c), a_t(a_t), b_t(b_t), c_t(c_t) {}

    double area();
    void get_uvw(double barycentric_a, double barycentric_b, double barycentric_c, double &u, double &v, double &w);
  };

class line_3D
  {
    protected:
      /* parametric line equation:
         x(t) = c0 + q0 * t;
         y(t) = c1 + q1 * t;
         z(t) = c2 + q2 * t; */

      double c0;
      double q0;
      double c1;
      double q1;
      double c2;
      double q2;

    public:
      line_3D(point_3D point1, point_3D point2);

        /**<
          Class constructor, makes a line by two given points.

          @param point1 first point, the value t = 0 in parametric
                 equation will give this point
          @param point2 second point, the value t = 1 in parametric
                 equation will give this point
          */

      point_3D get_vector_to_origin();

        /**<
          Gets a vector that's parallel with the line and points towards
          it's origin.

          @return normalized vector that's parallel with the line and
                  points towards it's origin
         */

      point_3D get_point(double t);

        /**<
          Gets a point of this line by given parameter value.

          @param t parameter value
          @param point in this variable the line point will be returned
         */

      bool intersects_triangle(triangle_3D triangle, double &a, double &b, double &c, double &t);

        /**<
          Checks whether the line intersects given triangle plus
          computes the barycentric coordination od the intersection in
          the triangle.

          @param a in this variable the first coordination of the
                 barycentric coordinations of the intersection will
                 be returned
          @param b in this variable the second coordination of the
                 barycentric coordinations of the intersection will
                 be returned
          @param c in this variable the third coordination of the
                 barycentric coordinations of the intersection will
                 be returned
          @param t in this variable the line parameter value of the
                 intersection will be returned
          @return true if the triangle is intersected by the line
         */

      bool intersects_sphere(sphere_3D sphere);

        /**<
          Checks whether the line intersects given sphere.

          @param sphere sphere to check the intersection with
          @return true if the line intersects the sphere, false
                  otherwise
         */
  };

void make_color(unsigned char color[3],unsigned char r, unsigned char g, unsigned char b);
double wrap(double value, double min, double max);
double saturate(double value, double min, double max);
int saturate_int(int value, int min, int max);
double interpolate_linear(double value1, double value2, double ratio);
void blend_colors(unsigned char color1[3], unsigned char color2[3], double ratio);

#endif
