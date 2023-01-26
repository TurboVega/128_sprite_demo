#include <stdio.h>
#include <math.h>

/*
Dec  Hex 
160 $00A0
208 $00D0
256 $0100
304 $0130
352 $0160
400 $0190
464 $01D0
512 $0200
*/

static const int num_points[8] = { 160, 208, 256, 304, 352, 400, 464, 512 };

int main()
{
    // ring radii: 50, 70, 90, 110, 130, 150, 170, 190
    int n = 0;
    for (int radius = 50; radius <= 190; radius+=20) {
        int wradius = (radius*4)/3;
        printf("sprite_path_%i_%i_x_%i: ; %i points\n", n, radius, wradius, num_points[n]);
        float inc = 360.0/num_points[n];
        int pt = 0;
        for (float angle = 0.0; angle < 360.0; angle+=inc) {
            float rads = 2.0f*3.1415926f*(float)angle/360.0f;
            unsigned short x=(unsigned short)(int)(cos(rads)*(float)wradius+320.0)-8;
            unsigned short y=(unsigned short)(int)(sin(rads)*(float)radius+240.0)-8;
            printf("    .word    %i,%i    ;   %i, angle: %f, rads: %f\n", x, y, pt++, angle, rads);
            if (pt >= num_points[n]) {
                break;
            }
        }
        printf("end_sprite_path_%i_%i_x_%i:\n\n", n, radius, wradius);
        n++;
    }

    return 0;
}

