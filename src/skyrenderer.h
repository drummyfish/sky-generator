#ifndef SKY_RENDERER_H
#define SKY_RENDERER_H

#include "raytracing.h"
#include "colorbuffer.h"

class sky_renderer
  {
    protected:
      void draw_terrain(t_color_buffer *buffer, unsigned char r1, unsigned char g1, unsigned char b1,
        unsigned char r2, unsigned char g2, unsigned char b2);
      void make_background_gradient(unsigned char background_color_from[3],unsigned char background_color_to[3], double time_of_day);
        /**<
          Makes a background sky color gradient depending on time of day.

          @param background_color_from in this array the first color of the
                 gradient will be returned
          @param background_color_from in this array the second color of the
                 gradient will be returned
          @param time_of_day time of day in range <0,1>
          */
      void get_sun_moon_attributes(double time_of_day, sphere_3D &sphere, unsigned char color[3]);
        /**<
          Computes the attributes of light or moon depending on time of day.

          @param time_of_day time of day in range <0,1>
          @param sphere in variable the sphere representing the sun/moon will
                 be returned
          @param color in this array the [r,g,b] color will be returned
          */
      void draw_stars(t_color_buffer *buffer, unsigned int number_of_stars);
        /**<
          Draws yellow stars on black background into given color buffer.

          @param buffer buffer that the stars will be drawn into, must be
                 initialised
          @param number of stars number of stras
          */
      void setup_sky_planes(vector<triangle_3D> *lower_plane, vector<triangle_3D> *upper_plane);
          /**<
           Sets up the sky planes.
           */
      double get_star_intensity(double day_time);
      double get_sun_intensity(double light_directness, double day_time);
          /**<
           Says how much given point of sky plane is lightened by the sun/moon.

           @param light_directness dot product of the vector from viewer and to
                  the sun/moon
           @param day_time time of the day
           @return intensity in range <0,1>
           */
      void fast_blur(t_color_buffer *buffer);
      void cloud_intensity_to_color(double intensity, double threshold, double cloud_density, unsigned char color[3]);
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
    public:
       void render_sky(t_color_buffer *buffer, double time_of_day, double clouds, double density, double offset);
           /**<
            Renders the sky into given color buffer. The sky is rendered only
            over white pixels (255,255,255).

            @param buffer buffer to render the sky into, it must be initialised
                   and only white pixels will be redrawn
            @param time_of_day says what time of day it is in range <0,1>,
                   0 meaning midnight, 0.5 noon etc.
            @param clouds how many clouds there should be in range <0,1>
            @param density cloud density in range <0,1>
            @param offset value in range <0,1>, sets noise offset in order to
                   animate clouds with constant time_of_day value, setting all
                   the values between 0 and 1 will make the animation loop, if
                   time_od_day is changing itself, this can be always 0
            @param progress_callback pointer to function that will be called
                   after each line rendered, this parameter can be NULL
            */
  };

#endif
