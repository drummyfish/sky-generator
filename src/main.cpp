#include <iostream>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "raytracing.hpp"

using namespace std;

// macro for int -> str conversion
#define SSTR( x ) dynamic_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

#include "perlin.h"
#include "colorbuffer.h"
#include "getopt.h"

struct param_struct       // command line argument values
  {
    double time;
    double duration;
    unsigned int frames;
    string name;
    unsigned int width;
    unsigned int height;
    bool help;
    bool silent;
    unsigned int supersampling;
    double clouds;        // how many clouds there are in range <0,1>
  } params;

void print_progress(int line)
  {
    if (line % 100 == 0)
      cout << ".";
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

void get_sun_moon_attributes(double time_of_day, sphere_3D &sphere, unsigned char color[3])
  /**<
   Computes the attributes of light or moon depending on time of day.

   @param time_of_day time of day in range <0,1>
   @param sphere in variable the sphere representing the sun/moon will
          be returned
   @param color in this array the [r,g,b] color will be returned
   */

   {
     double ratio;
     sphere.radius = 0.5;

     if (time_of_day < 0.75 && time_of_day > 0.25)  // sun
       {
         ratio = (time_of_day - 0.25) / 0.5;

         sphere.center.x = interpolate_linear(-20,20,ratio);
         sphere.center.y = 6;
         sphere.center.z = -1;

         color[0] = 255;
         color[1] = 255;
         color[2] = 94;
       }
     else                                           // moon
       {
         if (time_of_day <= 0.25)
           ratio = 0.5 + time_of_day / 0.5;
         else
           ratio = 0.5 * ((time_of_day - 0.75) / 0.25);

         sphere.center.x = interpolate_linear(-10,10,ratio);
         sphere.center.y = 6;
         sphere.center.z = -0.5;

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

void setup_sky_planes(vector<triangle_3D> *lower_plane, vector<triangle_3D> *upper_plane)
  /**<
   Sets up the sky planes.
   */

  {
    point_3D p1,p2,p3,p4,pt1,pt2,pt3,pt4;
    triangle_3D triangle;

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
    lower_plane->push_back(triangle);

    triangle.a = p2;
    triangle.b = p3;
    triangle.c = p4;
    triangle.a_t = pt2;
    triangle.b_t = pt3;
    triangle.c_t = pt4;
    lower_plane->push_back(triangle);

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
    upper_plane->push_back(triangle);

    triangle.a = p2;
    triangle.b = p3;
    triangle.c = p4;
    triangle.a_t = pt2;
    triangle.b_t = pt3;
    triangle.c_t = pt4;
    upper_plane->push_back(triangle);
  }

double get_star_intensity(double day_time)
  {
    if (day_time > 0.25 && day_time < 0.75)
      return 0;
    else
      {
        if (day_time > 0.75)
          return 1.0 - day_time;
        else
          return day_time * -4 + 1;
      }

    return 0; // to supress warnings
  }

double get_sun_intensity(double light_directness, double day_time)
  /**<
   Says how much given point of sky plane is lightened by the sun/moon.

   @param light_directness dot product of the vector from viewer and to
          the sun/moon
   @param day_time time of the day
   @return intensity in range <0,1>
   */

  {
    double intensity;

    if (day_time > 0.25 && day_time < 0.75) // day, sun
      {
        intensity = sin((day_time - 0.25) / 0.5 * PI);

        return saturate(intensity - 0.2,0,1) * 0.7 * saturate(light_directness,0,1) + 0.6;
      }

    // night, moon

    if (day_time <= 0.25)
      intensity = sin(PI / 2.0 + day_time / 0.25 * PI / 2.0);
    else
      intensity = sin((day_time - 0.75) / 0.25 * PI / 2.0);

    return saturate(intensity - 0.2,0,1) * 0.5 * saturate(light_directness,0,1) + 0.4;
  }

void fast_blur(t_color_buffer *buffer)
  {
    #define WINDOW_SIZE 9

    unsigned int i,j,k,l;
    unsigned char r,g,b;
    bool color1, color2;
    int window[WINDOW_SIZE][WINDOW_SIZE];
    int sum, value;
    t_color_buffer helper_buffer;

    color_buffer_copy(buffer,&helper_buffer);

    for (j = 0; j < buffer->height; j++)
      for (i = 0; i < buffer->width; i++)
        {
          color1 = false;
          color2 = false;

          for (k = 0; k < WINDOW_SIZE; k++)
            for (l = 0; l < WINDOW_SIZE; l++)
              {
                color_buffer_get_pixel(&helper_buffer,i + k - WINDOW_SIZE / 2,j + l - WINDOW_SIZE / 2,&r,&g,&b);

                if (r == 0)
                  color1 = true;

                if (r == 0)
                  color2 = true;

                window[k][l] = r;
              }

          if (color1 && color2)  // blur only if needed
            {
              sum = 0;

              for (k = 0; k < WINDOW_SIZE; k++)
                for (l = 0; l < WINDOW_SIZE; l++)
                  sum += window[k][l];

              value = sum / (WINDOW_SIZE * WINDOW_SIZE);

              color_buffer_set_pixel(buffer,i,j,value,value,value);
            }
        }

    color_buffer_destroy(&helper_buffer);
  }

void cloud_intensity_to_color(double intensity, double threshold, double cloud_density, unsigned char color[3])
  /**<
   Maps noise intensity to cloud color.

   @param intensity intensity to be mapped in range <0,1>
   @param threshold threshold up to which (from 0) the color will be
          black (0,0,0), must be in range <0,1>
   @param cloud_density says how dense the clouds should be in range
          <0,1>, this will affect maximum lightness of the returned
          color
   @param color in this variable the mapped color will be returned
   */

  {
    if (intensity < threshold)
      {
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
      }
    else
      {
        double q = (intensity - threshold) / (1 - threshold);

        color[0] = q * 255 * cloud_density;
        color[1] = q * 255 * cloud_density;
        color[2] = q * 255 * cloud_density;
      }
  }

void render_sky(t_color_buffer *buffer, double time_of_day, double clouds, void (* progress_callback)(int))

  /**<
   Renders the sky into given color buffer. The sky is rendered only
   over white pixels (255,255,255).
   *
   @param buffer buffer to render the sky into, it must be initialised
          and only white pixels will be redrawn
   @param time_of_day says what time of day it is in range <0,1>,
          0 meaning midnight, 0.5 noon etc.
   @param clouds how many clouds there should be in range <0,1>
   @param progress_callback pointer to function that will be called
          after each line rendered, this parameter can be NULL
   */

  {
    unsigned int i,j,k,sun_pixel_count;
    point_3D p1,p2;
    double u,v,w,t;
    unsigned char r,g,b;
    double aspect_ratio;
    double barycentric_a,barycentric_b,barycentric_c;
    vector<triangle_3D> sky_plane, sky_plane2;                          // triangles that make up the lower/upper sky plane
    unsigned char background_color_from[3], background_color_to[3];     // background color gradient, depends on time of the day
    t_color_buffer stars, sun_stencil;
    double star_intensity;
    unsigned char sun_moon_color[3], cloud_color[3];
    point_3D intersection, to_sun, to_camera;
    double sun_intensity;

    time_of_day = saturate(time_of_day,0.0,1.0);
    setup_sky_planes(&sky_plane,&sky_plane2);
    star_intensity = get_star_intensity(time_of_day);

    color_buffer_init(&stars,buffer->width,buffer->height);
    color_buffer_init(&sun_stencil,buffer->width,buffer->height);
    draw_stars(&stars,1000);

    make_background_gradient(background_color_from,background_color_to,time_of_day);
    aspect_ratio = buffer->height / ((double) buffer->width);

    p1.x = 0.0;  // point to cast the rays from
    p1.y = 0.0;
    p1.z = 0.0;

    sphere_3D sun_moon;
    get_sun_moon_attributes(time_of_day,sun_moon,sun_moon_color);

    sun_pixel_count = 0;

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

            if (line.intersects_sphere(sun_moon))                       // sun/moon
              {
                color_buffer_set_pixel(buffer,i,j,sun_moon_color[0],sun_moon_color[1],sun_moon_color[2]);
                color_buffer_set_pixel(&sun_stencil,i,j,0,0,0);
                sun_pixel_count++;
              }

            for (k = 0; k < sky_plane2.size(); k++)                     // upper sky plane
              if (line.intersects_triangle(sky_plane2[k],barycentric_a,barycentric_b,barycentric_c,t))
                {
                  get_triangle_uvw(sky_plane2[k],barycentric_a,barycentric_b,barycentric_c,u,v,w);

                  float f = saturate(perlin(u * PERLIN_WIDTH / 2 + PERLIN_WIDTH / 2, v * PERLIN_WIDTH / 2 + PERLIN_WIDTH / 2, PERLIN_WIDTH * time_of_day),0,1.0);

                  cloud_intensity_to_color(f,clouds,1,cloud_color);

                  line.get_point(t,intersection);
                  substract_vectors(sun_moon.center,intersection,to_sun);
                  normalize(to_sun);
                  to_camera = line.get_vector_to_origin();
                  sun_intensity = get_sun_intensity(dot_product(to_sun,to_camera),time_of_day);
                  color_buffer_add_pixel(buffer,i,j,cloud_color[0] * sun_intensity,cloud_color[1] * sun_intensity,cloud_color[2] * sun_intensity);
                }

             for (k = 0; k < sky_plane.size(); k++)                     // lower sky plane
              if (line.intersects_triangle(sky_plane[k],barycentric_a,barycentric_b,barycentric_c,t))
                {
                  get_triangle_uvw(sky_plane2[k],barycentric_a,barycentric_b,barycentric_c,u,v,w);

                  float f = saturate(perlin(u * PERLIN_WIDTH / 2 + PERLIN_WIDTH / 2, v * PERLIN_WIDTH / 2 + PERLIN_WIDTH / 2, PERLIN_WIDTH * (1 - time_of_day)),0,1.0);

                  cloud_intensity_to_color(f,clouds,1,cloud_color);

                  line.get_point(t,intersection);
                  substract_vectors(sun_moon.center,intersection,to_sun);
                  normalize(to_sun);
                  to_camera = line.get_vector_to_origin();
                  sun_intensity = get_sun_intensity(dot_product(to_sun,to_camera),time_of_day);
                  color_buffer_add_pixel(buffer,i,j,cloud_color[0] * sun_intensity,cloud_color[1] * sun_intensity,cloud_color[2] * sun_intensity);
                }
          }

        if (progress_callback != NULL)
          progress_callback(j);
      }

    if (sun_pixel_count > 10)  // postprocessing: only do this if something of the sun/moon is actually visible to save time
      {
        fast_blur(&sun_stencil);

        for (j = 0; j < buffer->height; j++)
          for (i = 0; i < buffer->width; i++)
            {
              color_buffer_get_pixel(&sun_stencil,i,j,&r,&g,&b);

              r = (255 - r) * 0.75;

              color_buffer_add_pixel(buffer,i,j,r,r,r);
            }
      }

    color_buffer_destroy(&sun_stencil);
  }

void print_help()
  {
     cout << "Skygen generates sky animations." << endl << endl;
     cout << "usage:" << endl << endl;
     cout << "skygen [[-t time][-d duration][-f frames][-o name][-c amount][-x width][-y height][-p level][-s] | [-h]]" << endl << endl;
     cout << "  -t specifies the day time, time is in HH:MM 24 hour format, for example 0:15, 12:00, 23:45. Default value is 12:00." << endl << endl;
     cout << "  -d specifies duration in minutes from the specified day time. If for example -t 12:00 -d 60 is set, the animation will be genrated from 12:00 to 13:00. If this flag is omitted, the whole animation will be generated at the same time of the day and will loop smoothly." << endl << endl;
     cout << "  -f specifies the number of frames of the animation. Default value is 1." << endl;
     cout << "  -o specifies output file(s) name. The files will be named nameX.png where X is the sequence number beginning with 1. If -f 1 is set, only one file with the name name.png will be generated. 'sky' is the default value." << endl << endl;
     cout << "  -x sets the resolution of the picture in x direction (width)." << endl << endl;
     cout << "  -y sets the resolution of the picture in y direction (height)." << endl << endl;
     cout << "  -p sets the supersampling level." << endl << endl;
     cout << "  -c say how many clouds there should be. amount is a whole number in range <0,100>." << endl << endl;
     cout << "  -s sets the silent mode, nothing will be written during rendering." << endl << endl;

     cout << "  -h prints help." << endl;
  }

void parse_command_line_arguments(int argc, char **argv)
  {
    // default values:
    params.time = 0.5;     // 12:00
    params.duration = 0.0;
    params.frames = 1;
    params.name = "sky";
    params.help = false;
    params.width = 1024;
    params.clouds = 0.5;
    params.height = 768;
    params.silent = false;
    params.supersampling = 1;

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
              params.frames = saturate_int(atoi(argv[i + 1]),1,65536);
            else if (helper_string == "-p")
              params.supersampling = saturate_int(atoi(argv[i + 1]),1,5);
            else if (helper_string == "-o")
              params.name = argv[i + 1];
            else if (helper_string == "-x")
              params.width = saturate_int(atoi(argv[i + 1]),0,65536);
            else if (helper_string == "-c")
              params.clouds = 1.0 - saturate_int(atoi(argv[i + 1]),0,100) / 100.0;
            else if (helper_string == "-y")
              params.height = saturate_int(atoi(argv[i + 1]),0,65536);
            else
              i--;

            i++;
          }

        if (helper_string == "-s")
          {
            params.silent = true;
          }
        else if (helper_string == "-h")
          {
            params.help = true;
          }

        i++;
      }
  }

int main(int argc, char **argv)
  {
    unsigned int i;
    t_color_buffer buffer;
    double step;
    string filename;

    parse_command_line_arguments(argc,argv);

    if (params.help)
      {
        print_help();
        return 0;
      }

    color_buffer_init(&buffer,params.width * params.supersampling,params.height * params.supersampling);

    step = params.duration / params.frames;

    for (i = 0; i < params.frames; i++)
      {
        color_buffer_clear(&buffer);
        draw_terrain(&buffer,50,200,10,70,100,0);

        if (!params.silent)
          {
            cout << "rendering image " << (i + 1) << endl;
            render_sky(&buffer,wrap(params.time + i * step,0.0,1.0),params.clouds,print_progress);
            cout << endl;
          }
        else
          {
            render_sky(&buffer,params.time + i * step,params.clouds,NULL);
          }

        if (params.frames == 1)
          filename = params.name + ".png";
        else
          filename = params.name + SSTR(i + 1) + ".png";

        t_color_buffer helper_buffer;

        supersampling(&buffer,params.supersampling,&helper_buffer);
        color_buffer_save_to_png(&helper_buffer,(char *) filename.c_str());
        color_buffer_destroy(&helper_buffer);
      }

    if (!params.silent)
      cout << "done" << endl;

    color_buffer_destroy(&buffer);

    return 0;
  }
