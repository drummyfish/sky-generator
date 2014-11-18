#include <iostream>
#include <math.h>
#include <vector>
#include "raytracing.hpp"

using namespace std;

extern "C"
{
#include "perlin.h"
#include "colorbuffer.h"
}

void draw_terrain(t_color_buffer *buffer, unsigned char r1, unsigned char g1, unsigned char b1,
  unsigned char r2, unsigned char g2, unsigned char b2)
  {
    int i, j, height;
    double x, ratio;
    unsigned char r, g, b;

    for (i = 0; i < buffer->width; i++)
      {
        x = i / ((double) buffer->width - 1) * 2.5 + 0.3;

        height = ((sin(x) + cos(5 * x) * x / 10.0)) * buffer->height * 0.1 + buffer->height * 0.15;

        for (j = height; j >= 0; j--)
          {
            ratio = j / ((double) height);

            r = interpolate_linear(r1,r2,ratio);
            g = interpolate_linear(g1,g2,ratio);
            b = interpolate_linear(b1,b2,ratio);

            color_buffer_set_pixel(buffer,i,buffer->height - j - 1,r,g,b);
          }
      }
  }

void render_sky(t_color_buffer *buffer)

  /**<
   Renders the sky into given color buffer. The sky is rendered only
   over white pixels (255,255,255).

   @param buffer buffer to render the sky into, it must be initialised
          and only white pixels will be redrawn
   */

  {
    unsigned int i,j,k;
    point_3D p1,p2,p3,p4,pt1,pt2,pt3,pt4;
    double u,v,w;
    unsigned char r,g,b;
    triangle_3D triangle;
    double aspect_ratio;
    double barycentric_a,barycentric_b,barycentric_c;
    vector<triangle_3D> triangles;

    aspect_ratio = buffer->height / ((double) buffer->width);

    p1.x = -14;
    p1.y = 2;
    p1.z = -2;

    p2.x = 14;
    p2.y = 2;
    p2.z = -2;

    p3.x = -10;
    p3.y = 10;
    p3.z = 5;

    p4.x = 10;
    p4.y = 10;
    p4.z = 5;

    pt1.x = 0;
    pt1.y = 0;

    pt2.x = 1;
    pt2.y = 0;

    pt3.x = 0;
    pt3.y = 1;

    pt4.x = 1;
    pt4.y = 1;

    triangle.a = p1;
    triangle.b = p2;
    triangle.c = p3;
    triangle.a_t = pt1;
    triangle.b_t = pt2;
    triangle.c_t = pt3;
    triangles.push_back(triangle);

    triangle.a = p2;
    triangle.b = p3;
    triangle.c = p4;
    triangle.a_t = pt2;
    triangle.b_t = pt3;
    triangle.c_t = pt4;
    triangles.push_back(triangle);


    p1.x = 0.0;  // point to cast the rays from
    p1.y = 0.0;
    p1.z = 0.0;

    for (j = 0; j < buffer->height; j++)
      {
        for (i = 0; i < buffer->width; i++)
          {
            color_buffer_get_pixel(buffer,i,j,&r,&g,&b);

            if (r != 255 && g != 255 && b != 255)  // not white => don't render
              continue;

            p2.x = ((i / ((double) buffer->width)) - 0.5);
            p2.y = 0.5;
            p2.z = ((j / ((double) buffer->height)) - 0.5) * aspect_ratio;

            line_3D line(p1,p2);

            for (k = 0; k < triangles.size(); k++)
              if (line.intersects_triangle(triangles[k],barycentric_a,barycentric_b,barycentric_c))
                {
                  u = barycentric_a * triangles[k].a_t.x + barycentric_b * triangles[k].b_t.x + barycentric_c * triangles[k].c_t.x;
                  v = barycentric_a * triangles[k].a_t.y + barycentric_b * triangles[k].b_t.y + barycentric_c * triangles[k].c_t.y;
                  w = barycentric_a * triangles[k].a_t.z + barycentric_b * triangles[k].b_t.z + barycentric_c * triangles[k].c_t.z;


                  float f = perlin(u * WIDTH, v * WIDTH, 0);
                  f = (f > 1.0) ? 1.0 : f; // saturace
                  color_buffer_set_pixel(buffer,i,j,f*255,f*255,f*255);
                }
          }
      }

  }

int main(void)
  {
    unsigned int width, height;
    t_color_buffer buffer;

    width = 1600/2;
    height = 1200/2;

    color_buffer_init(&buffer,width,height);

    draw_terrain(&buffer,150,250,50,50,150,10);

    render_sky(&buffer);

    color_buffer_save_to_png(&buffer,"picture.png");
    color_buffer_destroy(&buffer);

    return 0;
  }
