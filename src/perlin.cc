/*
 * Perlin Noise
 */

#include "perlin.h"
#include <stdio.h>

/**
 * nejaky "nahodny" sum
 * v intervalu (-1, 1)
 */
static inline float noise(int x, int y, int z)
{
    int n;
    n = x + y * 57 + z * 23;
    n = (n<<13) ^ n;
    return ( 1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}


/*
 * linearni inerpolace a-b, podle parametru t <0, 1>
 */
static inline float interpolate(float a, float b, float t)
{
    return t * (b - a) + a; // == (1-t) * a + t * b
}


// kolikatou iteraci zacit - nizke frekvence moc nemaji smysl
#define OCT_START 3
// posledni iterace
#define OCTAVES 10

/*
 * perlinuv sum na souradnicich <x, y, z> z prostoru PERLIN_WIDTH^3
 */
float perlin(float x, float y, float z)
{
    float sum_noise = 0.0;

    for(int octave = OCT_START; octave < OCTAVES; octave += 1) {

        int sample = PERLIN_WIDTH >> (octave);

        // vypocet souradnic rohu kostky, ve kerych se bude pocitat sum
        // a mezi nimi iterpolovat
        int x_0 = ((int)x / sample) * sample;
        int x_1 = (x_0 + sample) % PERLIN_WIDTH;
        int y_0 = ((int)y / sample) * sample;
        int y_1 = (y_0 + sample) % PERLIN_WIDTH;
        int z_0 = ((int)z / sample) * sample;
        int z_1 = (z_0 + sample) % PERLIN_WIDTH;

        // koeficienty pro interpolaci
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

        // interpolace v ose x
        float b_0 = interpolate(a_0, a_1, x_t);
        float b_1 = interpolate(a_2, a_3, x_t);
        float b_2 = interpolate(a_4, a_5, x_t);
        float b_3 = interpolate(a_6, a_7, x_t);
        // interpolace v ose y
        float c_0 = interpolate(b_0, b_1, y_t);
        float c_1 = interpolate(b_2, b_3, y_t);
        // interpolace v ose z
        float d_0 = interpolate(c_0, c_1, z_t);

        // suma sumu ruznych frekvenci
        sum_noise += d_0 / (1 << (octave-OCT_START));
    }

    // sum je (-1, 1), my chceme (0, 1)
    return (sum_noise + 1) / 2;
}
