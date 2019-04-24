#include <stdio.h>
#include <math.h>
#include "gfx.h"
/*
 * World's dumbest routine to read PGM/PNM files.
 */
int main(int argc, char **argv)
{
    FILE *pbmfile;
    int pbmwidth, pbmheight, pbmdepth;
    int xorg, yorg, x, y, pbmrgb, mode;
    unsigned char r, g, b, gamma[256], pbmformat[8];
    float gammafunc;

    for (x = 0; x < 256; x++)
        gamma[x] = x;
    mode = MODE_BEST;
    while (argc > 1 && argv[1][0] == '-')
    {
        if (argc > 2 && argv[1][1] == 'g')
        {
            gammafunc = atof(argv[2]);
            for (x = 0; x < 256; x++)
                gamma[x] = pow((float)x/255.0, gammafunc) * 255.0;
            argc--;
            argv++;
        }
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
    if (argc > 1)
    {
        if ((pbmfile = fopen(argv[1], "rb")) == NULL)
        {
            fprintf(stderr, "Can't open %s\n", argv[1]);
            return -1;
        }
    }
    else
        pbmfile = stdin;
    fgets(pbmformat, 4, pbmfile);
    if (pbmformat[0] == 'P' && pbmformat[1] == '6')
        pbmrgb = 1;
    else if (pbmformat[0] == 'P' && pbmformat[1] == '5')
        pbmrgb = 0;
    else
    {
        fprintf(stderr, "Not a valid PBM file.\n");
        return -1;
    }
    if (fscanf(pbmfile, "%d\n%d\n%d\n", &pbmwidth, &pbmheight, &pbmdepth) != 3)
    {
        fprintf(stderr, "Bad PBM header.\n");
        return -1;
    }
    xorg = 160 - (pbmwidth / 2);
    yorg = 100 - (pbmheight / 2);
    if (!gfxmode(mode))
    {
        fprintf(stderr, "Unable to set graphics mode.\n");
        return -1;
    }
    for (y = 0; y < pbmheight; y++)
        for (x = 0; x < pbmwidth; x++)
        {
            r = gamma[getc(pbmfile)];
            if (pbmrgb)
            {
                g = gamma[getc(pbmfile)];
                b = gamma[getc(pbmfile)];
            }
            else
                g = b = r;
            if (x + xorg >= 0 && x + xorg < 320 && y + yorg >= 0 && y + yorg < 200)
            {
                color(r, g, b);
                pixel(x + xorg, y + yorg);
            }
        }
    getch();
    restoremode();
    return 0;
}
