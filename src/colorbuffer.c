//**********************************************************************

/**
 * @author Miloslav Ciz
 */

//**********************************************************************

#include "colorbuffer.h"
#include "lodepng.h"
//#include "general.h"
#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------------

unsigned char round_to_char(int value)

  {
    if (value > 255)
      return 255;

    if (value < 0)
      return 0;

    return value;
  }

//----------------------------------------------------------------------

int color_buffer_clear(t_color_buffer *buffer)

  {
    unsigned int i,j;

    for (j = 0; j < buffer->height; j++)
      for (i = 0; i < buffer->width; i++)
        color_buffer_set_pixel(buffer,i,j,255,255,255);
  }

//----------------------------------------------------------------------

int transform_coordination(int coordination, int limit)

  {
    if (coordination < 0)
      {
        coordination = -1 * coordination;
        coordination = coordination % limit;
        coordination = limit - coordination;
      }

    if (coordination != 0)
      coordination = coordination % limit;

    return coordination;
  }

//----------------------------------------------------------------------

void color_buffer_set_pixel(t_color_buffer *buffer, int position_x,
  int position_y, unsigned char red, unsigned char green,
  unsigned char blue)

  {
    int index;

    position_x = transform_coordination(position_x,buffer->width);
    position_y = transform_coordination(position_y,buffer->height);

    index = 3 * (position_y * buffer->width + position_x);

    buffer->data[index] = red;
    buffer->data[index + 1] = green;
    buffer->data[index + 2] = blue;

    return;
  }

//----------------------------------------------------------------------

void color_buffer_add_pixel(t_color_buffer *buffer, int position_x,
  int position_y, unsigned char red, unsigned char green,
  unsigned char blue)

  {
    unsigned char help_red, help_green, help_blue;
    int help_red2, help_green2, help_blue2;

    color_buffer_get_pixel(buffer,position_x,position_y,&help_red,
      &help_green,&help_blue);

    help_red2 = help_red + red;
    help_green2 = help_green + green;
    help_blue2 = help_blue + blue;

    color_buffer_set_pixel(buffer,position_x,position_y,
      round_to_char(help_red2),round_to_char(help_green2),
      round_to_char(help_blue2));
  }

//----------------------------------------------------------------------

void color_buffer_substract_pixel(t_color_buffer *buffer,
  int position_x, int position_y, unsigned char red,
  unsigned char green, unsigned char blue)

  {
    unsigned char help_red, help_green, help_blue;
    int help_red2, help_green2, help_blue2;

    color_buffer_get_pixel(buffer,position_x,position_y,&help_red,
      &help_green,&help_blue);

    help_red2 = help_red - red;
    help_green2 = help_green - green;
    help_blue2 = help_blue - blue;

    color_buffer_set_pixel(buffer,position_x,position_y,
      round_to_char(help_red2),round_to_char(help_green2),
      round_to_char(help_blue2));
  }

//----------------------------------------------------------------------

void color_buffer_get_pixel(t_color_buffer *buffer, int position_x,
  int position_y, unsigned char *red, unsigned char *green,
  unsigned char *blue)

  {
    int index;

    position_x = transform_coordination(position_x,buffer->width);
    position_y = transform_coordination(position_y,buffer->height);

    index = 3 * (position_y * buffer->width + position_x);

    if (red != NULL)
      *red = buffer->data[index];

    if (green != NULL)
      *green = buffer->data[index + 1];

    if (blue != NULL)
      *blue = buffer->data[index + 2];

    return;
  }

//----------------------------------------------------------------------

void color_buffer_copy(t_color_buffer *buffer, t_color_buffer
  *destination)

  {
    unsigned int i, j;
    unsigned char red, green, blue;

    color_buffer_init(destination,buffer->width,buffer->height);

    for (j = 0; j < buffer->height; j++)
      for (i = 0; i < buffer->width; i++)
        {
          color_buffer_get_pixel(buffer,i,j,&red,&green,&blue);
          color_buffer_set_pixel(destination,i,j,red,green,blue);
        }
  }

//----------------------------------------------------------------------

void color_buffer_copy_data(t_color_buffer *buffer,
  t_color_buffer *destination)

  {
    unsigned int i, j;
    unsigned char red, green, blue;

    if (destination == NULL)
      return;

    for (j = 0; j < buffer->height; j++)
      for (i = 0; i < buffer->width; i++)
        {
          color_buffer_get_pixel(buffer,i,j,&red,&green,&blue);
          color_buffer_set_pixel(destination,i,j,red,green,blue);
        }
  }

//----------------------------------------------------------------------

void color_buffer_destroy(t_color_buffer *buffer)

  {
    if (buffer->data != NULL)
      free(buffer->data);

    buffer->data = NULL;
  }

//----------------------------------------------------------------------

int color_buffer_init(t_color_buffer *buffer, int width, int height)

  {
    int i;
    int length;

    buffer->width = width;         // set the new width and height
    buffer->height = height;

    length = width * height * 3 * sizeof(char);

    buffer->data = (unsigned char *) malloc(length);

    if (buffer->data == NULL)
      return 0;

    for (i = 0; i < length; i++)   // set the image to white color
      buffer->data[i] = 255;

    return 1;
  }

//----------------------------------------------------------------------

int color_buffer_save_to_png(t_color_buffer *buffer, char *filename)

  {
    if (lodepng_encode24_file(filename,buffer->data,
      buffer->width,buffer->height) == 0)
      return 1;
    else
      return 0;
  }

//----------------------------------------------------------------------

int color_buffer_load_from_png(t_color_buffer *buffer, char *filename)

  {
    if (lodepng_decode24_file(&(buffer->data),&buffer->width,
        &buffer->height,filename) == 0)
      return 1;
    else
      return 0;
  }

//----------------------------------------------------------------------

void supersampling(t_color_buffer *buffer, unsigned int level,
  t_color_buffer *destination)

  {
    unsigned int i, j, k, l, sum_red, sum_green, sum_blue;
    unsigned char red, green, blue;

    color_buffer_init(destination,buffer->width / level,
      buffer->height / level);

    for (j = 0; j < destination->height; j++)
      for (i = 0; i < destination->width; i++)
        {
          sum_red = 0;
          sum_green = 0;
          sum_blue = 0;

          for (k = 0; k < level; k++)          // get 4 x 4 area pixels sums
            for (l = 0; l < level; l++)
              {
                color_buffer_get_pixel(buffer,(level * i) + k,
                  (level * j) + l,&red,&green,&blue);

                sum_red += red;
                sum_green += green;
                sum_blue += blue;
              }

          red = sum_red / (level * level);     // average
          green = sum_green / (level * level);
          blue = sum_blue / (level * level);

          color_buffer_set_pixel(destination,i,j,red,green,blue);
        }
  }

//----------------------------------------------------------------------
