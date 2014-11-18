/*
 * Perlin Noise
 */

#include "perlin.h"
#include <stdio.h>

float perlin(int x, int y, int z);

/**
 * nejaky "nahodny" sum
 */
static inline float noise(int x, int y, int z)
{
    int n;
    n = x + y * 57 + z * 23;
    n = (n<<13) ^ n;
    return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0 / 2);
}


/*
 * linearni inerpolace a-b, podle parametru t <0, 1>
 */
static inline float interpolate(float a, float b, float t)
{
    return t * (b - a) + a; // == (1-t) * a + t * b
}


/*
 * perlinuv sum na souradnicich <x, y, z> z prostoru WIDTH^3
 */
float perlin(int x, int y, int z)
{

    float sum_noise = 0.0;

    for(int octave = 0; octave < OCTAVES; octave += 1) {

        int sample = WIDTH >> (octave);

        // vypocet souradnic rohu kostky, co se budou interpolovat
        int x_0 = (x / sample) * sample;
        int x_1 = (x_0 + sample) % WIDTH;
        int y_0 = (y / sample) * sample;
        int y_1 = (y_0 + sample) % WIDTH;
        int z_0 = (z / sample) * sample;
        int z_1 = (z_0 + sample) % WIDTH;

        // koeficienty interpolace
        float x_t = (float) (x - x_0) / sample;
        float y_t = (float) (y - y_0) / sample;
        float z_t = (float) (z - z_0) / sample;

        // sum v 8 rohovych bodech
        float a_0 = noise(x_0, y_0, z_0);
        float a_1 = noise(x_1, y_0, z_0);
        float a_2 = noise(x_0, y_1, z_0);
        float a_3 = noise(x_1, y_1, z_0);
        float a_4 = noise(x_0, y_0, z_1);
        float a_5 = noise(x_1, y_0, z_1);
        float a_6 = noise(x_0, y_1, z_1);
        float a_7 = noise(x_1, y_1, z_1);

        // interpolace
        float b_0 = interpolate(a_0, a_1, x_t);
        float b_1 = interpolate(a_2, a_3, x_t);
        float b_2 = interpolate(a_4, a_5, x_t);
        float b_3 = interpolate(a_6, a_7, x_t);
        float c_0 = interpolate(b_0, b_1, y_t);
        float c_1 = interpolate(b_2, b_3, y_t);
        float d_0 = interpolate(c_0, c_1, z_t);

        // vysledek
        sum_noise += d_0 / (1 << octave);
    }

    return sum_noise;
}
