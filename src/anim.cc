#include <iostream>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <SDL2/SDL.h>
#include "raytracing.h"
#include "skyrenderer.h"
#include "perlin.h"
#include "colorbuffer.h"
#include "getopt.h"

using namespace std;

// macro for int -> str conversion
#define SSTR( x ) dynamic_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

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
    double cloud_density;
  } params;

void print_help()
  {
     cout << "Skygen generates sky animations." << endl << endl;
     cout << "usage:" << endl << endl;
     cout << "skygen [[-t time][-d duration][-f frames][-o name][-c amount][-e density][-x width][-y height][-p level][-s] | [-h]]" << endl << endl;
     cout << "  -t specifies the day time, time is in HH:MM 24 hour format, for example 0:15, 12:00, 23:45. Default value is 12:00." << endl << endl;
     cout << "  -d specifies duration in minutes from the specified day time. If for example -t 12:00 -d 60 is set, the animation will be genrated from 12:00 to 13:00. If this flag is omitted, the whole animation will be generated at the same time of the day and will loop smoothly." << endl << endl;
     cout << "  -f specifies the number of frames of the animation. Default value is 1." << endl;
     cout << "  -o specifies output file(s) name. The files will be named nameX.png where X is the sequence number beginning with 1. If -f 1 is set, only one file with the name name.png will be generated. 'sky' is the default value." << endl << endl;
     cout << "  -x sets the resolution of the picture in x direction (width)." << endl << endl;
     cout << "  -y sets the resolution of the picture in y direction (height)." << endl << endl;
     cout << "  -p sets the supersampling level." << endl << endl;
     cout << "  -c say how many clouds there should be. amount is a whole number in range <0,100>." << endl << endl;
     cout << "  -e sets the cloud density. density is a whole number in range <0,100>." << endl << endl;
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
    params.cloud_density = 0.75;
    params.height = 768;
    params.silent = false;
    params.supersampling = 1;

    int i = 0;
    string helper_string;
    char *helper_pointer;
    int hours;
    int minutes;

    while (i < argc)
      {
        helper_string = argv[i];

        if (i < argc - 1)     // options that take parameters
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
            else if (helper_string == "-e")
              params.cloud_density = saturate_int(atoi(argv[i + 1]),0,100) / 100.0;
            else if (helper_string == "-y")
              params.height = saturate_int(atoi(argv[i + 1]),0,65536);
            else
              i--;

            i++;
          }

        if (helper_string == "-s")
          params.silent = true;
        else if (helper_string == "-h")
          params.help = true;

        i++;
      }
  }

int main(int argc, char **argv)
  {
    unsigned int i;
    t_color_buffer buffer;
    double step, noise_offset, noise_step;
    string filename;
    sky_renderer renderer;

    parse_command_line_arguments(argc,argv);

    if (params.help)
      {
        print_help();
        return 0;
      }

    color_buffer_init(&buffer,params.width * params.supersampling,params.height * params.supersampling);

    step = params.duration / params.frames;        // step in time
    noise_offset = 0;                              // noise offset for animating the noise, only used with static daytime
    noise_step = 1.0 / ((double) params.frames);   // step for noise_offset

    SDL_Window *sdlWindow;
    SDL_Renderer *sdlRenderer;
    SDL_CreateWindowAndRenderer(params.width, params.height, 0, &sdlWindow, &sdlRenderer);

    SDL_Texture *sdlTexture;
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, params.width, params.height);
    SDL_Event sdlEvent;

    for (i = 0; i < params.frames; i++)
      {
        if (!params.silent)
          {
            cout << "rendering image " << (i + 1) << endl;
          }

        renderer.render_sky(&buffer,params.time + i * step,params.clouds,params.cloud_density,noise_offset);

        if (params.duration == 0.0)        // hopefully this is safe
          noise_offset += noise_step;

        filename = params.frames == 1 ? params.name + ".png" : params.name + SSTR(i + 1) + ".png";

        SDL_UpdateTexture(sdlTexture, NULL, buffer.data, params.width*4);
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
        SDL_PollEvent(&sdlEvent);
      }

    if (!params.silent)
      cout << "done" << endl;

    color_buffer_destroy(&buffer);

    return 0;
  }
