extern "C"
{
#include "colorbuffer.h"
}

int main(void)
  {
    unsigned int i,j;

    t_color_buffer buffer;

    color_buffer_init(&buffer,400,300);

    for(j = 0; j < 300; j++)
      for(i = 0; i < 400; i++)
        color_buffer_set_pixel(&buffer,i,j,255,255 - j,i);

    color_buffer_save_to_png(&buffer,(char *) "picture.png");

    color_buffer_destroy(&buffer);

    return 0;
  }
