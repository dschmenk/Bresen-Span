#define __MSC
#include <stdio.h>
#include <bios.h>
#include <dos.h>
#include <conio.h>
#include "gfx.h"

unsigned long gettime(void)
{
    struct dostime_t time;

    _dos_gettime(&time);
    return time.hour * 360000UL + time.minute * 6000UL + time.second * 100UL + time.hsecond;
}
int main(int argc, char **argv)
{
    long int c;
    int n, mode;
    unsigned long linetime, aatime;

    mode = MODE_BEST;
    while (argc > 1 && argv[1][0] == '-')
    {
        if (argv[1][1] == 'd')
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
    color(0, 0, 0);
    clear();
    linetime = gettime();
    for (n = 0; n < 200; n++)
    {
        c = n * 255L / 200;
        color(0, c, 0);
        line(160, 100, 319, n);
        color(255-c, 0, 0);
        line(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        c = n * 255L / 320;
        color(0, 0, 255-c);
        line(160, 100, n,   0);
        color(c, c, c);
        line(160, 100, n, 199);
    }
#if 0
    color(0, 0, 0);
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
#endif
    linetime = gettime() - linetime;
    flip();
    color(0, 0, 0);
    clear();
    aatime = gettime();
    for (n = 0; n < 200; n += 10)
    {
        c = n * 255L / 200;
        color(0, c, 0);
        aaline(160, 100, 319, n);
        color(255-c, 0, 0);
        aaline(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n -= 10)
    {
        c = n * 255L / 320;
        color(0, 0, 255-c);
        aaline(160, 100, n,   0);
        color(c, c, c);
        aaline(160, 100, n, 199);
    }
    aatime = gettime() - aatime;
    getch();
    flip();
    getch();
    restoremode();
    printf("Line time = %lu\nAnti-Alias line time = %lu\n", linetime, aatime);
    return 0;
}
