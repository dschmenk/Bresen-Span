#include <stdio.h>
#include <bios.h>
#include <dos.h>
#include <conio.h>
#include "rgb8.h"
/*
 * Pixel-at-a-time line routines.
 */
#if 0
#include "pixline.c"
#else
void pixline(int x1, int y1, int x2, int y2);
#endif

unsigned long gettime(void)
{
    struct dostime_t time;

    _dos_gettime(&time);
    return time.hour * 360000UL + time.minute * 6000UL + time.second * 100UL + time.hsecond;
}
int main(int argc, char **argv)
{
    long int n, c;
    unsigned long normtime, fasttime;

    c = 5;
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'd')
    {
        c = argv[1][2] - '0';
        argc--;
        argv++;
    }
    setmodex(0x13, c);
    normtime = gettime();
    for (n = 0; n < 200; n++)
    {
        c = n * 255 / 200;
        brush8rgb(0, c, 0);
        pixline(160, 100, 319, n);
        brush8rgb(255-c, 0, 0);
        pixline(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        c = n * 255 / 320;
        brush8rgb(0, 0, 255-c);
        pixline(160, 100, n,   0);
        brush8rgb(c, c, c);
        pixline(160, 100, n, 199);
    }
    brush8rgb(0, 0, 0);
    for (n = 0; n < 200; n++)
    {
        pixline(160, 100, 319, n);
        pixline(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        pixline(160, 100, n,   0);
        pixline(160, 100, n, 199);
    }
    normtime = gettime() - normtime;
    fasttime = gettime();
    for (n = 0; n < 200; n++)
    {
        c = n * 255 / 200;
        brush8rgb(0, c, 0);
        line(160, 100, 319, n);
        brush8rgb(255-c, 0, 0);
        line(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        c = n * 255 / 320;
        brush8rgb(0, 0, 255-c);
        line(160, 100, n,   0);
        brush8rgb(c, c, c);
        line(160, 100, n, 199);
    }
    brush8rgb(0, 0, 0);
    for (n = 0; n < 200; n++)
    {
        line(160, 100, 319, n);
        line(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        line(160, 100, n,   0);
        line(160, 100, n, 199);
    }
    fasttime = gettime() - fasttime;
    restoremode();
    printf("Normal line time = %lu\nFast line time = %lu\n", normtime, fasttime);
    return 0;
}
