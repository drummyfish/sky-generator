#include <iostream>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include "raytracing.hpp"

using namespace std;

extern "C"
{
#include "perlin.h"
#include "colorbuffer.h"
#include "getopt.h"
}

void draw_terrain(t_color_buffer *buffer, unsigned char r1, unsigned char g1, unsigned char b1,
  unsigned char r2, unsigned char g2, unsigned char b2)
  {
    int i, j, height;
    double x, ratio;
    unsigned char r, g, b;

    for (i = 0; i < (int) buffer->width; i++)
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

void get_sun_moon_attributes(double time_of_day, double position[3], unsigned char color[3])
  /**<
   Computes the attributes of light or moon depending on time of day.

   @param time_of_day time of day in range <0,1>
   @param position in this array the [x,y,z] position will be returned
   @param color in this array the [r,g,b] color will be returned
   */

   {
     double ratio;

     if (time_of_day < 0.75 && time_of_day > 0.25)  // sun
       {
         ratio = (time_of_day - 0.25) / 0.5;

         position[0] = interpolate_linear(-20,20,ratio);
         position[1] = 15;
         position[2] = -5;

         color[0] = 255;
         color[1] = 255;
         color[2] = 94;
       }
     else                                         // moon
       {
         if (time_of_day <= 0.25)
           ratio = 0.5 + time_of_day / 0.5;
         else
           ratio = 0.5 * ((time_of_day - 0.75) / 0.25);

         position[0] = interpolate_linear(-10,10,ratio);
         position[1] = 15;
         position[2] = -0.5;

         color[0] = 237;
         color[1] = 237;
         color[2] = 237;
       }
   }

void draw_stars(t_color_buffer *buffer, unsigned int number_of_stars)
  /**<
   Draws yellow stars on black background into given color buffer.

   @param buffer buffer that the stars will be drawn into, must be
          initialised
   @param number of stars number of stras
   */

  {
    unsigned int i,j,x,y;
    unsigned char r,g,b;

    srand(10);

    for (j = 0; j < buffer->height; j++)
      for (i = 0; i < buffer->width; i++)
        color_buffer_set_pixel(buffer,i,j,0,0,0);

    for (i = 0; i < number_of_stars; i++)
      {
        x = rand() % buffer->width;
        y = rand() % buffer->height;

        r = 255;
        g = 255 - rand() % 20;
        b = 255 - rand() % 100;

        color_buffer_set_pixel(buffer,x,y,r,g,b);

        if (rand() % 7 == 0)  // a bigger star
          {                   // no need to check picture borders, the color buffer takes care of it
            color_buffer_set_pixel(buffer,x + 1,y,r,g,b);
            color_buffer_set_pixel(buffer,x + 1,y + 1,r,g,b);
            color_buffer_set_pixel(buffer,x,y + 1,r,g,b);
          }
      }
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
    t_color_buffer stars;
    double star_intensity;
    unsigned char sun_moon_color[3];
    double sun_moon_position[3];

    time_of_day = saturate(time_of_day,0.0,1.0);

    if (time_of_day > 0.25 && time_of_day < 0.75)
      star_intensity = 0;
    else
      {
        if (time_of_day > 0.75)
          star_intensity = 1.0 - time_of_day;
        else
          star_intensity = time_of_day;

          star_intensity = star_intensity * -4 + 1;
      }

    get_sun_moon_attributes(time_of_day,sun_moon_position,sun_moon_color);

    color_buffer_init(&stars,buffer->width,buffer->height);
    draw_stars(&stars,1000);

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

    sphere_3D sun_moon;
    sun_moon.center.x = sun_moon_position[0];
    sun_moon.center.y = sun_moon_position[1];
    sun_moon.center.z = sun_moon_position[2];
    sun_moon.radius = 1.5;

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

            color_buffer_get_pixel(&stars,i,j,&r,&g,&b);                            // stars

            r *= star_intensity;
            g *= star_intensity;
            b *= star_intensity;

            color_buffer_set_pixel(buffer,i,j,round_to_char(back_r + r),round_to_char(back_g + g),round_to_char(back_b + b));    // background gradient

            if (line.intersects_sphere(sun_moon))
              color_buffer_set_pixel(buffer,i,j,sun_moon_color[0],sun_moon_color[1],sun_moon_color[2]);

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
                }
          }
      }

    color_buffer_destroy(&stars);
  }

struct param_struct       // command line argument values
  {
    double time;
    double duration;
    unsigned int frames;
    string name;
    bool help;
  } params;

void print_help()
  {
     cout << "Skygen generates sky animations." << endl << endl;
     cout << "usage:" << endl << endl;
     cout << "skygen [[-t time][-d duration][-f frames][-o name] | [-h]]" << endl << endl;
     cout << "  -t specifies the day time, time is in HH:MM 24 hour format, for example 0:15, 12:00, 23:45. Default value is 12:00." << endl << endl;
     cout << "  -d specifies duration in minutes from the specified day time. If for example -t 12:00 -d 60 is set, the animation will be genrated from 12:00 to 13:00. If this flag is omitted, the whole animation will be generated at the same time of the day and will loop smoothly." << endl << endl;
     cout << "  -f specifies the number of frames of the animation. Default value is 1." << endl;
     cout << "  -o specifies output file(s) name. The files will be named nameX.png where X is the sequence number beginning with 1. If -f 1 is set, only one file with the name name.png will be generated. 'sky' is the default value." << endl << endl;
     cout << "  -h prints help." << endl;
  }

void parse_command_line_arguments(int argc, char **argv)
  {
    params.time = 0.5;     // 12:00
    params.duration = 0.0;
    params.frames = 1;
    params.name = "sky";
    params.help = false;

    unsigned int i = 0;
    string helper_string;
    char *helper_pointer;
    int hours;
    int minutes;

    while ((int) i < argc)
      {
        helper_string = argv[i];

        if ((int) i < argc - 1)   // options that take parameters
          {
            if (helper_string == "-t")
              {
                hours = saturate_int(atoi(argv[i + 1]),0,23);

                helper_pointer = argv[i + 1];

                while (*helper_pointer != 0 && *helper_pointer != ':')
                  helper_pointer++;

                if (*helper_pointer == ':')
                  helper_pointer++;

                minutes = saturate_int(atoi(helper_pointer),0,59);
                minutes = hours * 60 + minutes;

                params.time = minutes / ((double) (24 * 60));
              }
            else if (helper_string == "-d")
              params.duration = saturate_int(atoi(argv[i + 1]),0,65536) / ((double) (24 * 60));

            else if (helper_string == "-f")
              params.frames = saturate_int(atoi(argv[i + 1]),0,65536);
            else if (helper_string == "-o")
              params.name = argv[i + 1];
            else
              i--;

            i++;
          }

        if (helper_string == "-h")
          {
            params.help = true;
          }

        i++;
      }
  }

int main(int argc, char **argv)
  {
    vector<string> args(argv + 1, argv + argc);

    parse_command_line_arguments(argc,argv);

    if (params.help)
      {
        print_help();
        return 0;
      }

    unsigned int width, height;
    t_color_buffer buffer;

    width = 1600/2;
    height = 1200/2;

    char name[] = "picturex.png";

    color_buffer_init(&buffer,width,height);


    unsigned int i;

    for (i = 0; i < 10; i++)
      {
      //  cout << i << endl;

        name[7] = '0' + i;

        color_buffer_clear(&buffer);
        draw_terrain(&buffer,50,200,10,70,100,0);
        render_sky(&buffer,i / 9.0);

        color_buffer_save_to_png(&buffer,name);
      }

    color_buffer_destroy(&buffer);

    return 0;
  }
