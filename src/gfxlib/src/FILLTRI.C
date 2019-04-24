
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include "gfx.h"

unsigned long gettime(void)
{
    struct dostime_t time;

    _dos_gettime(&time);
    return time.hour * 360000UL + time.minute * 6000UL + time.second * 100UL + time.hsecond;
}
int main(int argc, char **argv)
{
    int mode, c, f, b;
    int xv[3], yv[3], ix[3], iy[3];
    int rgb[3], irgb[3];
    f = 1;
    b = 0;
    mode = MODE_BEST;
    while (argc > 1 && argv[1][0] == '-')
    {
        if (argv[1][1] == 'f')
            f = argv[1][2] - '0';
        else if (argv[1][1] == 'b')
            b = 1;
        else if (argv[1][1] == 'd')
            switch (argv[1][2] - '0')
            {
                case 2: mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_2BPP;
                    break;
                case 4: mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_4BPP;
                    break;
                case 8: mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_8BPP;
                    break;
            }
        else if (argv[1][1] == 'm')
            mode |= MODE_MONO;
        else if (argv[1][1] == 'n')
            mode |= MODE_NODITHER;
        argc--;
        argv++;
    }
    gfxmode(mode);
    if (b)
        render(BACK_PAGE);
    srand(gettime());
    for (c = 0; c < 3; c++)
    {
        xv[c]   = rand() % 319 + 1;
        yv[c]   = rand() % 199 + 1;
        ix[c]   = ((rand() % 20) - 10) | 1;
        iy[c]   = ((rand() % 20) - 10) | 1;
        rgb[c]  = rand() % 256;
        irgb[c] = ((rand() % 5) - 3) | 1;
    }
    while (!kbhit())
    {
        if (b)
        {
            color(0, 0, 0);
            clear();
        }
        color(rgb[0], rgb[1], rgb[2]);
        if (f)
        {
            beginfill();
            line(xv[0], yv[0], xv[1], yv[1]);
            line(xv[1], yv[1], xv[2], yv[2]);
            line(xv[2], yv[2], xv[0], yv[0]);
            endfill();
            color(255, 255, 255);
        }
        aaline(xv[0], yv[0], xv[1], yv[1]);
        aaline(xv[1], yv[1], xv[2], yv[2]);
        aaline(xv[2], yv[2], xv[0], yv[0]);
        for (c = 0; c < 3; c++)
        {
            xv[c] += ix[c];
            if (xv[c] < 1)
            {
                ix[c] = -ix[c];
                xv[c] = 1;
            }
            if (xv[c] > 318)
            {
                ix[c] = -ix[c];
                xv[c] = 318;
            }
            yv[c] += iy[c];
            if (yv[c] < 1)
            {
                iy[c] = -iy[c];
                yv[c] = 1;
            }
            if (yv[c] > 198)
            {
                iy[c] = -iy[c];
                yv[c] = 198;
            }
            rgb[c] += irgb[c];
            if (rgb[c] < 0)
            {
                irgb[c] = -irgb[c];
                rgb[c]  = 0;
            }
            if (rgb[c] > 255)
            {
                irgb[c] = -irgb[c];
                rgb[c]  = 255;
            }
        }
        if (b)
            flip(VSYNC);
    }
    getch();
    restoremode();
    return 0;
}
