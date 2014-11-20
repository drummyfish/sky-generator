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

void make_background_gradient(unsigned char background_color_from[3],unsigned char background_color_to[3], double time_of_day)
  /**<
   Makes a background sky color gradient depending on time of day.

   @param background_color_from in this array the first color of the
          gradient will be returned
   @param background_color_from in this array the second color of the
          gradient will be returned
   @param time_of_day time of day in range <0,1>
   */

  {
    unsigned char gradient_points[2][4][3];

    gradient_points[0][0][0] = 0;          // midnight
    gradient_points[0][0][1] = 0;
    gradient_points[0][0][2] = 0;
    gradient_points[1][0][0] = 170;
    gradient_points[1][0][1] = 146;
    gradient_points[1][0][2] = 239;

    gradient_points[0][1][0] = 218;        // morning
    gradient_points[0][1][1] = 215;
    gradient_points[0][1][2] = 226;
    gradient_points[1][1][0] = 237;
    gradient_points[1][1][1] = 177;
    gradient_points[1][1][2] = 194;

    gradient_points[0][2][0] = 60;         // noon
    gradient_points[0][2][1] = 13;
    gradient_points[0][2][2] = 183;
    gradient_points[1][2][0] = 185;
    gradient_points[1][2][1] = 221;
    gradient_points[1][2][2] = 232;

    gradient_points[0][3][0] = 98;         // evening
    gradient_points[0][3][1] = 13;
    gradient_points[0][3][2] = 183;
    gradient_points[1][3][0] = 245;
    gradient_points[1][3][1] = 137;
    gradient_points[1][3][2] = 96;

    double ratio;
    unsigned int interpolate_from, interpolate_to;

    if (time_of_day < 0.25)       // midnight to morning
      {
        ratio = time_of_day / 0.25;
        interpolate_from = 0;
        interpolate_to = 1;
      }
    else if (time_of_day < 0.5)   // morning to noon
      {
        ratio = (time_of_day - 0.25) / 0.25;
        interpolate_from = 1;
        interpolate_to = 2;
      }
    else if (time_of_day < 0.75)  // noon to evening
      {
        ratio = (time_of_day - 0.5) / 0.25;
        interpolate_from = 2;
        interpolate_to = 3;
      }
    else                          // evening to midnight
      {
        ratio = (time_of_day - 0.75) / 0.25;
        interpolate_from = 3;
        interpolate_to = 0;
      }

    background_color_from[0] = (unsigned char) interpolate_linear(gradient_points[0][interpolate_from][0],gradient_points[0][interpolate_to][0],ratio);
    background_color_from[1] = (unsigned char) interpolate_linear(gradient_points[0][interpolate_from][1],gradient_points[0][interpolate_to][1],ratio);
    background_color_from[2] = (unsigned char) interpolate_linear(gradient_points[0][interpolate_from][2],gradient_points[0][interpolate_to][2],ratio);

    background_color_to[0] = (unsigned char) interpolate_linear(gradient_points[1][interpolate_from][0],gradient_points[1][interpolate_to][0],ratio);
    background_color_to[1] = (unsigned char) interpolate_linear(gradient_points[1][interpolate_from][1],gradient_points[1][interpolate_to][1],ratio);
    background_color_to[2] = (unsigned char) interpolate_linear(gradient_points[1][interpolate_from][2],gradient_points[1][interpolate_to][2],ratio);
  }

void render_sky(t_color_buffer *buffer, double time_of_day)

  /**<
   Renders the sky into given color buffer. The sky is rendered only
   over white pixels (255,255,255).
   *
   @param buffer buffer to render the sky into, it must be initialised
          and only white pixels will be redrawn
   @param time_of_day says what time of day it is in range <0,1>,
          0 meaning midnight, 0.5 noon etc.
   */

  {
    unsigned int i,j,k;
    point_3D p1,p2,p3,p4,pt1,pt2,pt3,pt4;
    double u,v,w;
    unsigned char r,g,b;
    triangle_3D triangle;
    double aspect_ratio;
    double barycentric_a,barycentric_b,barycentric_c;
    vector<triangle_3D> sky_plane, sky_plane2;                          // triangles that make up the lower/upper sky plane
    unsigned char background_color_from[3], background_color_to[3];     // background color gradient, depends on time of the day

    time_of_day = saturate(time_of_day,0.0,1.0);

    make_background_gradient(background_color_from,background_color_to,time_of_day);

    aspect_ratio = buffer->height / ((double) buffer->width);

    p1.x = -14;   p1.y = 2;   p1.z = -2;    // lower sky plane geometry
    p2.x = 14;    p2.y = 2;   p2.z = -2;
    p3.x = -10;   p3.y = 10;  p3.z = 5;
    p4.x = 10;    p4.y = 10;  p4.z = 5;
    pt1.x = 0;    pt1.y = 0;                // texture coordinates
    pt2.x = 1;    pt2.y = 0;
    pt3.x = 0;    pt3.y = 1;
    pt4.x = 1;    pt4.y = 1;

    triangle.a = p1;
    triangle.b = p2;
    triangle.c = p3;
    triangle.a_t = pt1;
    triangle.b_t = pt2;
    triangle.c_t = pt3;
    sky_plane.push_back(triangle);

    triangle.a = p2;
    triangle.b = p3;
    triangle.c = p4;
    triangle.a_t = pt2;
    triangle.b_t = pt3;
    triangle.c_t = pt4;
    sky_plane.push_back(triangle);

    p1.x = -14;   p1.y = -1;   p1.z = -2;    // upper sky plane geometry
    p2.x = 14;    p2.y = -1;   p2.z = -2;
    p3.x = -10;   p3.y = 10;   p3.z = 5;
    p4.x = 10;    p4.y = 10;   p4.z = 5;

    triangle.a = p1;
    triangle.b = p2;
    triangle.c = p3;
    triangle.a_t = pt1;
    triangle.b_t = pt2;
    triangle.c_t = pt3;
    sky_plane2.push_back(triangle);

    triangle.a = p2;
    triangle.b = p3;
    triangle.c = p4;
    triangle.a_t = pt2;
    triangle.b_t = pt3;
    triangle.c_t = pt4;
    sky_plane2.push_back(triangle);

    p1.x = 0.0;  // point to cast the rays from
    p1.y = 0.0;
    p1.z = 0.0;

    for (j = 0; j < buffer->height; j++)
      {
        double ratio = j / ((double) buffer->height);
        unsigned char back_r, back_g, back_b;

        back_r = interpolate_linear(background_color_from[0],background_color_to[0],ratio);
        back_g = interpolate_linear(background_color_from[1],background_color_to[1],ratio);
        back_b = interpolate_linear(background_color_from[2],background_color_to[2],ratio);

        for (i = 0; i < buffer->width; i++)
          {
            color_buffer_get_pixel(buffer,i,j,&r,&g,&b);

            if (r != 255 && g != 255 && b != 255)  // not white => don't render
              continue;

            p2.x = ((i / ((double) buffer->width)) - 0.5);
            p2.y = 0.5;
            p2.z = ((j / ((double) buffer->height)) - 0.5) * aspect_ratio;

            line_3D line(p1,p2);

            color_buffer_set_pixel(buffer,i,j,back_r,back_g,back_b);    // background gradient
/*
            for (k = 0; k < sky_plane2.size(); k++)                     // upper sky plane
              if (line.intersects_triangle(sky_plane2[k],barycentric_a,barycentric_b,barycentric_c))
                {
                  get_triangle_uvw(sky_plane2[k],barycentric_a,barycentric_b,barycentric_c,u,v,w);

                  float f = perlin(u * WIDTH / 2 + WIDTH / 2, v * WIDTH / 2 + WIDTH / 2, 100);
                  f = saturate(f,0.0,1.0);

                  if (f >= 0.75)
                    color_buffer_set_pixel(buffer,i,j,f*255,f*255,f*255);
                }

            for (k = 0; k < sky_plane.size(); k++)                      // lower sky plane
              if (line.intersects_triangle(sky_plane[k],barycentric_a,barycentric_b,barycentric_c))
                {
                  get_triangle_uvw(sky_plane[k],barycentric_a,barycentric_b,barycentric_c,u,v,w);

                  float f = perlin(u * WIDTH / 2 + WIDTH / 2, v * WIDTH / 2 + WIDTH / 2, 10);
                  f = saturate(f,0.0,1.0);

                  if (f >= 0.78)
                    color_buffer_set_pixel(buffer,i,j,interpolate_linear(100,255,f),interpolate_linear(100,255,f),interpolate_linear(50,100,f));
                } */
          }
      }
  }

int main(void)
  {
    unsigned int width, height;
    t_color_buffer buffer;

    width = 1600/2;
    height = 1200/2;

    char name[] = "picturex.png";

    color_buffer_init(&buffer,width,height);


    unsigned int i;

    for (i = 0; i < 10; i++)
      {
        name[7] = '0' + i;

        color_buffer_clear(&buffer);
        draw_terrain(&buffer,50,200,10,70,100,0);
        render_sky(&buffer,i / 9.0);

        color_buffer_save_to_png(&buffer,name);
      }


    color_buffer_destroy(&buffer);

    return 0;
  }
