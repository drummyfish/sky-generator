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

typedef struct         /**< point, also a vector */
  {
    double x;
    double y;
    double z;
  } point_3D;

typedef struct         /**< point, also a vector */
  {
    point_3D center;
    double radius;
  } sphere_3D;

typedef struct
  {
    point_3D a;        /**< position coordinates */
    point_3D b;
    point_3D c;

    point_3D a_t;      /**< texturing coordinates */
    point_3D b_t;
    point_3D c_t;
  } triangle_3D;

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

      void get_point(double t, point_3D &point);

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

void print_point(point_3D point);
double wrap(double value, double min, double max);
double saturate(double value, double min, double max);
int saturate_int(int value, int min, int max);
double interpolate_linear(double value1, double value2, double ratio);
void cross_product(point_3D vector1, point_3D vector2, point_3D &final_vector);
void substract_vectors(point_3D vector1, point_3D vector2, point_3D &final_vector);
double point_distance(point_3D a, point_3D b);
double vector_length(point_3D vector);
double dot_product(point_3D vector1, point_3D vector2);
void normalize(point_3D &vector);
double vectors_angle(point_3D vector1, point_3D vector2);
void revert_vector(point_3D &vector);
double triangle_area(triangle_3D triangle);
void get_triangle_uvw(triangle_3D triangle, double barycentric_a, double barycentric_b, double barycentric_c, double &u, double &v, double &w);
#endif
