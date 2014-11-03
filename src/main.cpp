#include <iostream>
#include <math.h>

using namespace std;

#define PI 3.1415926535897932384626

extern "C"
{
#include "colorbuffer.h"
}

typedef struct         /**< point, also a vector */
  {
    float x;
    float y;
    float z;
  } point_3D;

typedef struct
  {
    point_3D a;
    point_3D b;
    point_3D c;
  } triangle_3D;

class line_3D
  {
    protected:
      /* parametric line equation:
         x(t) = c0 + q0 * t;
         y(t) = c1 + q1 * t;
         z(t) = c2 + q2 * t; */

      float c0;
      float q0;
      float c1;
      float q1;
      float c2;
      float q2;

    public:
      line_3D(point_3D point1, point_3D point2);

        /**<
          Class constructor, makes a line by two given points.

          @param point1 first point, the value t = 0 in parametric
                 equation will give this point
          @param point2 second point, the value t = 1 in parametric
                 equation will give this point
          */

      void get_point(float t, point_3D &point);

        /**<
          Gets a point of this line by given parameter value.

          @param t parameter value
          @param point in this variable the line point will be returned
         */

      bool intersects_triangle(triangle_3D triangle, float &a, float &b, float &c);

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
          @return true if the triangle is intersected by the line
         */
  };

void print_point(point_3D point)
  {
    cout << "(" << point.x << ", " << point.y << ", " << point.z << ")" << endl;
  }

void cross_product(point_3D vector1, point_3D vector2, point_3D &final_vector)
  {
    final_vector.x = vector1.y * vector2.z - vector1.z * vector2.y;
    final_vector.y = vector1.z * vector2.x - vector1.x * vector2.z;
    final_vector.z = vector1.x * vector2.y - vector1.y * vector2.x;
  }

void substract_vectors(point_3D vector1, point_3D vector2, point_3D &final_vector)
  {
    final_vector.x = vector2.x - vector1.x;
    final_vector.y = vector2.y - vector1.y;
    final_vector.z = vector2.z - vector1.z;
  }

float vector_length(point_3D vector)
  {
    return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
  }

float dot_product(point_3D vector1, point_3D vector2)
  {
    return vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z;
  }

float vectors_angle(point_3D vector1, point_3D vector2)
  {
    return acos(dot_product(vector1,vector2) / (vector_length(vector1) * vector_length(vector2)));
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

void line_3D::get_point(float t, point_3D &point)
  {
    point.x = this->c0 + this->q0 * t;
    point.y = this->c1 + this->q1 * t;
    point.z = this->c2 + this->q2 * t;
  }

bool line_3D::intersects_triangle(triangle_3D triangle, float &a, float &b, float &c)
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

    float qa = normal.x;
    float qb = normal.y;
    float qc = normal.z;
    float d = -1 * (qa * triangle.a.x + qb * triangle.a.y + qc * triangle.a.z);

    /*
     Solve for t:
     */

    float denominator = (qa * this->q0 + qb * this->q1 + qc * this->q2);

    if (denominator == 0)
      return false;

    float t = (-qa * this->c0 - qb * this->c1 - qc * this->c2 - d) / denominator;

    /* t now contains parameter value for the intersection */

    if (t < 0.0)
      return false;

    point_3D intersection;

    this->get_point(t,intersection);  // intersection in 3D space

    // vectors from the intersection to each triangle vertex:

    substract_vectors(intersection,triangle.a,vector1);
    substract_vectors(intersection,triangle.b,vector2);
    substract_vectors(intersection,triangle.c,vector3);

    point_3D normal1, normal2, normal3;

    // now multiply the vectors to get their normals:

    cross_product(vector1,vector2,normal1);
    cross_product(vector2,vector3,normal2);
    cross_product(vector3,vector1,normal3);

    // if one of the vectors points in other direction than the others, the point is not inside the triangle:

    if (dot_product(normal1,normal2) < 0 || dot_product(normal2,normal3) < 0)
      return false;

    // now compute the barycentric coordination:

    point_3D helper_vector1, helper_vector2;
    float alpha, beta, gamma, alpha2, beta2, gamma2;

    substract_vectors(triangle.a,triangle.b,helper_vector1);
    substract_vectors(triangle.a,triangle.c,helper_vector2);
    alpha = vectors_angle(helper_vector1,helper_vector2);

    substract_vectors(triangle.b,triangle.a,helper_vector1);
    substract_vectors(triangle.b,triangle.c,helper_vector2);
    beta = vectors_angle(helper_vector1,helper_vector2);

    substract_vectors(triangle.c,triangle.a,helper_vector1);
    substract_vectors(triangle.c,triangle.b,helper_vector2);
    gamma = vectors_angle(helper_vector1,helper_vector2);

    alpha2 = vectors_angle(vector2,vector3);
    beta2 = vectors_angle(vector1,vector3);
    gamma2 = vectors_angle(vector1,vector2);

    a = (alpha2 - alpha) / (PI / 2.0 - alpha);
    b = (beta2 - beta) / (PI / 2.0 - beta);
    c = (gamma2 - gamma) / (PI / 2.0 - gamma);

    // now adjust the coords to sum up to 1.0:

    float sum = a + b + c;

    a /= sum;
    b /= sum;
    c /= sum;

    return true;
  }

int main(void)
  {
    point_3D p1,p2,p3,pt1,pt2,pt3;

    triangle_3D triangle;

    pt1.x = 0.327;
    pt1.y = 1.517;
    pt1.z = 0.349;

    pt2.x = 0.585;
    pt2.y = 2.095;
    pt2.z = -0.098;

    pt3.x = -0.062;
    pt3.y = 2.095;
    pt3.z = 0.0;

    triangle.a = pt1;
    triangle.b = pt2;
    triangle.c = pt3;

    unsigned int width, height, i, j;
    t_color_buffer buffer;

    width = 640;
    height = 480;

    color_buffer_init(&buffer,width,height);

    p1.x = 0.0;
    p1.y = 0.0;
    p1.z = 0.0;

    float a,b,c;

    for (j = 0; j < height; j++)
      {
        for (i = 0; i < width; i++)
          {
            p2.x = ((i / ((float) width)) - 0.5);
            p2.y = 0.5;
            p2.z = ((j / ((float) height)) - 0.5) * 0.8;

            line_3D line(p1,p2);

            if (line.intersects_triangle(triangle,a,b,c))
              {
                color_buffer_set_pixel(&buffer,i,j,a * 255,b * 255,c * 255);
                cout << a << " " << b << " " << c << " (" << (a + b + c) << ")" << endl;
              }
            else
              color_buffer_set_pixel(&buffer,i,j,0,0,0);
          }

        cout << "line " << j << endl;
      }

    color_buffer_save_to_png(&buffer,"picture.png");
    color_buffer_destroy(&buffer);

    return 0;
  }
