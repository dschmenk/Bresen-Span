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
    int mode, c, n;

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
    for (n = 0; n < 320; n++)
    {
        c = n * 255L / 320;
        color(c, c, c);
        line(n, 0, n, 199);
    }
    getch();
    restoremode();
    return 0;
}
