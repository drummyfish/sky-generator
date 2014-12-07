#include "skyrenderer.h"
#include <stdlib.h>
#include "perlin.h"

void sky_renderer::draw_terrain(t_color_buffer *buffer, unsigned char r1, unsigned char g1, unsigned char b1,
  unsigned char r2, unsigned char g2, unsigned char b2)
  {
    int i, j, height;
    double x, ratio;
    unsigned char r, g, b;

    for (i = 0; i < (int) buffer->width; i++)
      {
        x = i / ((double) buffer->width - 1) * 2.5 + 0.3;

        height = ((sin(x) + cos(5 * x) * x / 10.0)) * buffer->height * 0.05 + buffer->height * 0.20;

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

void sky_renderer::make_background_gradient(unsigned char background_color_from[3],unsigned char background_color_to[3], double time_of_day)
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

void sky_renderer::get_sun_moon_attributes(double time_of_day, sphere_3D &sphere, unsigned char color[3])
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

void sky_renderer::draw_stars(t_color_buffer *buffer, unsigned int number_of_stars)
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

void sky_renderer::setup_sky_planes(vector<triangle_3D> *lower_plane, vector<triangle_3D> *upper_plane)
  {
    // lower skyplane
    point_3D p1(-14.0, 0.0, -1.0); // souradnice
    point_3D p2( 14.0, 0.0, -1.0);
    point_3D p3(-20.0, 20.0, 11.0);
    point_3D p4( 20.0, 20.0, 11.0);
    point_3D pt1(0.0, 0.0, 0.0); // texturovaci souradnice
    point_3D pt2(0.75, 0.0, 0.0);
    point_3D pt3(0.0, 0.75, 0.0);
    point_3D pt4(0.75, 0.75, 0.0);
    lower_plane->push_back(triangle_3D(p1, p2, p3, pt1, pt2, pt3));
    lower_plane->push_back(triangle_3D(p2, p3, p4, pt2, pt3, pt4));

    // upper skyplane
    p1 = point_3D(-14.0, 0.0, -2.0); // souradnice
    p2 = point_3D( 14.0, 0.0, -2.0);
    p3 = point_3D(-21.0, 20.0, 11.0);
    p4 = point_3D( 21.0, 20.0, 11.0);
    pt1 = point_3D(0.0, 0.0, 0.0); // texturovaci souradnice
    pt2 = point_3D(2.0, 0.0, 0.0);
    pt3 = point_3D(0.0, 2.0, 0.0);
    pt4 = point_3D(2.0, 2.0, 0.0);
    upper_plane->push_back(triangle_3D(p1, p2, p3, pt1, pt2, pt3));
    upper_plane->push_back(triangle_3D(p2, p3, p4, pt2, pt3, pt4));
  }

double sky_renderer::get_star_intensity(double day_time)
  {
    if (day_time > 0.25 && day_time < 0.75)
      return 0;
    else
      {
        if (day_time > 0.75)
          return (day_time - 0.75) / 0.25;
        else
          return (0.25 - day_time) / 0.25;
      }

    return 0; // to supress warnings
  }

double sky_renderer::get_sun_intensity(double light_directness, double day_time)
  {
    double intensity;

    if (day_time > 0.25 && day_time < 0.75) // day, sun
      {
        intensity = sin((day_time - 0.25) / 0.5 * PI);
        return saturate(intensity - 0.2,0,1) * 0.6 * saturate(light_directness,0,1) + 0.4;
      }

    // night, moon

    if (day_time <= 0.25)
      intensity = sin(PI / 2.0 + day_time / 0.25 * PI / 2.0);
    else
      intensity = sin((day_time - 0.75) / 0.25 * PI / 2.0);

    return saturate(intensity - 0.2,0,1) * 0.4 * saturate(light_directness,0,1) + 0.2;
  }

void sky_renderer::fast_blur(t_color_buffer *buffer)
  {
    #define WINDOW_SIZE 9

    unsigned int i,j,k,l,x,y;
    unsigned char r,g,b;
    bool color1, color2;
    int window[WINDOW_SIZE][WINDOW_SIZE];
    int width_minus_one, height_minus_one;
    int sum, value;
    t_color_buffer helper_buffer;

    width_minus_one = buffer->width - 1;
    height_minus_one = buffer->height - 1;

    color_buffer_copy(buffer,&helper_buffer);

    for (j = 0; j < buffer->height; j++)
      #pragma omp for
      for (i = 0; i < buffer->width; i++)
        {
          color1 = false;
          color2 = false;

          for (k = 0; k < WINDOW_SIZE; k++)
            for (l = 0; l < WINDOW_SIZE; l++)
              {
                x = saturate_int(i + k - WINDOW_SIZE / 2,0,width_minus_one);
                y = saturate_int(j + l - WINDOW_SIZE / 2,0,height_minus_one);

                color_buffer_get_pixel(&helper_buffer,x,y,&r,&g,&b);

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

void sky_renderer::cloud_intensity_to_color(double intensity, double threshold, double cloud_density, unsigned char color[3])
  {
    if (intensity < threshold)
      {
        color[0] = 0; color[1] = 0; color[2] = 0;
      }
    else
      {
        double q = (intensity - threshold) / (1 - threshold);

        color[0] = q * 255 * cloud_density;
        color[1] = q * 255 * cloud_density;
        color[2] = q * 255 * cloud_density;
      }
  }

void sky_renderer::render_sky(t_color_buffer *buffer, double time_of_day, const double clouds, const double density, const double offset)
  {

    t_color_buffer stars, sun_stencil;
    color_buffer_init(&stars,buffer->width,buffer->height);             // buffer to which stars will be drawn
    color_buffer_init(&sun_stencil,buffer->width,buffer->height);       // buffer to which sun stencil will be drawn

    #pragma omp parallel default(none) firstprivate(time_of_day, clouds, density, offset) shared(buffer, sun_stencil, stars)
    {
    unsigned int i,j,k,l;
    point_3D p1, p2, intersection, to_sun, to_camera;
    double u, v, w, t, star_intensity, sun_intensity, aspect_ratio, barycentric_a, barycentric_b, barycentric_c;
    unsigned char r, g, b;
    vector<triangle_3D> sky_plane, sky_plane2;                          // triangles that make up the lower/upper sky plane
    unsigned char background_color_from[3], background_color_to[3], sun_moon_color[3], cloud_color[3];
    unsigned char terrain_color1[3], terrain_color2[3];

    color_buffer_clear(buffer);

    time_of_day = wrap(time_of_day,0.0,1.0);

    make_color(terrain_color1,50,200,10);     // terraing color gradient
    make_color(terrain_color2,70,100,0);
    make_background_gradient(background_color_from,background_color_to,time_of_day);
    blend_colors(terrain_color1,background_color_to,0.2);                                           // slightly alter the terrain color with background color
    blend_colors(terrain_color2,background_color_from,0.4);

    draw_terrain(buffer,terrain_color2[0],terrain_color2[1],terrain_color2[2],terrain_color1[0],terrain_color1[1],terrain_color1[2]);   // draw the terrain before rendering the sky

    setup_sky_planes(&sky_plane,&sky_plane2);
    star_intensity = get_star_intensity(time_of_day);

    #pragma omp master
    draw_stars(&stars,1000);

    aspect_ratio = buffer->height / ((double) buffer->width);

    sphere_3D sun_moon;
    get_sun_moon_attributes(time_of_day,sun_moon,sun_moon_color);

    for (j = 0; j < buffer->height; j++)            // for each picture line
      {
        // make the background color from gradient:

        double ratio = j / ((double) buffer->height);
        unsigned char back_r, back_g, back_b;
        back_r = interpolate_linear(background_color_from[0],background_color_to[0],ratio);
        back_g = interpolate_linear(background_color_from[1],background_color_to[1],ratio);
        back_b = interpolate_linear(background_color_from[2],background_color_to[2],ratio);

        #pragma omp for
        for (i = 0; i < buffer->width; i++)        // for each picture column
          {
            color_buffer_get_pixel(buffer,i,j,&r,&g,&b);

            if (r != 255 && g != 255 && b != 255)  // not white (terrain) => don't render
              continue;

            // point at the projection plane, 0.4 is focal distance
            p2 = point_3D(((i / ((double) buffer->width)) - 0.5),0.4,((j / ((double) buffer->height)) - 0.5) * aspect_ratio);

            line_3D line(p1,p2);                                        // make the ray line

            color_buffer_get_pixel(&stars,i,j,&r,&g,&b);                // stars

            r *= star_intensity;
            g *= star_intensity;
            b *= star_intensity;

            color_buffer_set_pixel(buffer,i,j,round_to_char(back_r + r),round_to_char(back_g + g),round_to_char(back_b + b)); // background gradient + stars

            if (line.intersects_sphere(sun_moon))                       // sun/moon
              {
                color_buffer_set_pixel(buffer,i,j,sun_moon_color[0],sun_moon_color[1],sun_moon_color[2]);
                color_buffer_set_pixel(&sun_stencil,i,j,0,0,0);
              }

            for (l = 0; l < 2; l++)   // for both sky planes
              {
                vector<triangle_3D> *plane;

                plane = l == 0 ? &sky_plane : &sky_plane2;              // get the pointer to plane being rendered

                for (k = 0; k < plane->size(); k++)                     // for all triangles of the sky plane
                  if (line.intersects_triangle((*plane)[k],barycentric_a,barycentric_b,barycentric_c,t))
                  {
                    (*plane)[k].get_uvw(barycentric_a,barycentric_b,barycentric_c,u,v,w);

                    u = wrap(u + offset + time_of_day * 2,0,1);
                    v = wrap(v + offset + time_of_day * 2,0,1);
                    w = (l == 0 ? time_of_day : 1 - time_of_day);

                    float f = saturate(perlin(u * PERLIN_WIDTH,v * PERLIN_WIDTH, w * PERLIN_WIDTH),0,1.0);

                    cloud_intensity_to_color(f,clouds,density,cloud_color);   // maps f to [r,g,b] with threshold

                    intersection = line.get_point(t);
                    to_sun = sun_moon.center - intersection;
                    to_sun.normalize();
                    to_camera = line.get_vector_to_origin();
                    sun_intensity = get_sun_intensity(to_sun.dot_product(to_camera), time_of_day);

                    color_buffer_add_pixel(buffer,i,j,cloud_color[0] * sun_intensity,cloud_color[1] * sun_intensity,cloud_color[2] * sun_intensity);
                  }
              }
          }
      }
    
    fast_blur(&sun_stencil);

    for (j = 0; j < buffer->height; j++)
      #pragma omp for
      for (i = 0; i < buffer->width; i++)
        {
          color_buffer_get_pixel(&sun_stencil,i,j,&r,&g,&b);
          r = (255 - r) * 0.75;
          color_buffer_add_pixel(buffer,i,j,r,r,r);
        }

    } // omp parallel end

    color_buffer_destroy(&sun_stencil);
        
  }
