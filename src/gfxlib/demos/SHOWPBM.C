#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfx.h"

#define SCREEN_ASPECT   (320.0/200.0)

#define greyscan    grnscan
#define greyaccum   grnaccum

unsigned char redscan[320];
unsigned char grnscan[320];
unsigned char bluscan[320];
unsigned int  redaccum[320];
unsigned int  grnaccum[320];
unsigned int  bluaccum[320];
unsigned char gamma[256];

FILE *pbmfile;
int pbmwidth, pbmheight, pbmdepth;
int left, right, top, bottom;

/*
 * Image stretch with help of span line routine
 */
void hstretchrgb(int xl, int xr, int y)
{
    unsigned int r, g, b;

    r = gamma[getc(pbmfile)];
    g = gamma[getc(pbmfile)];
    b = gamma[getc(pbmfile)];
    do
    {
        redscan[xl] = r;
        grnscan[xl] = g;
        bluscan[xl] = b;
    } while (++xl <= xr);
}
void hstretchgrey(int xl, int xr, int y)
{
    unsigned int grey;

    grey = gamma[getc(pbmfile)];
    do
    {
        greyscan[xl] = grey;
    } while (++xl <= xr);
}
void hshrinkrgb(int x, int yt, int yb)
{
    unsigned int r, g, b, n;

    r = g = b = 0;
    n = yb - yt + 1;
    do
    {
        r += gamma[getc(pbmfile)];
        g += gamma[getc(pbmfile)];
        b += gamma[getc(pbmfile)];
    } while (++yt <= yb);
    redscan[x] = r / n;
    grnscan[x] = g / n;
    bluscan[x] = b / n;
}
void hshrinkgrey(int x, int yt, int yb)
{
    unsigned int g, n;

    g = 0;
    n = yb - yt + 1;
    do
    {
        g += gamma[getc(pbmfile)];
    } while (++yt <= yb);
    greyscan[x] = g / n;
}
void vstretchrgb(int xl, int xr, int y)
{
    int p;

    hspan = hstretchrgb;
    vspan = hshrinkrgb;
    line(left, 0, right, pbmwidth-1);
    do
    {
        for (p = 0; p < 320; p++)
        {
            color(redscan[p], grnscan[p], bluscan[p]);
            pixel(p, scan);
        }
    } while (++xl <= xr);
    hspan = vstretchrgb;
}
void vstretchgrey(int xl, int xr, int y)
{
    int p;

    hspan = hstretchgrey;
    vspan = hshrinkgrey;
    line(left, 0, right, pbmwidth-1);
    do
    {
        for (p = 0; p < 320; p++)
        {
            color(greyscan[p], greyscan[p], greyscan[p]);
            pixel(p, scan);
        }
    } while (++xl <= xr);
    hspan = vstretchgrey;
}
void vshrinkrgb(int x, int yt, int yb)
{
    unsigned int n, p;

    memset(redaccum, 0, 320 * sizeof(unsigned int));
    memset(grnaccum, 0, 320 * sizeof(unsigned int));
    memset(bluaccum, 0, 320 * sizeof(unsigned int));
    hspan = hstretchrgb;
    vspan = hshrinkrgb;
    n = yb - yt + 1;
    do
    {
        line(left, 0, right, pbmwidth-1);
        for (p = 0; p < 320; p++)
        {
            redaccum[p] += redscan[p];
            grnaccum[p] += grnscan[p];
            bluaccum[p] += bluscan[p];
        }
    } while (++yt <= yb);
    for (p = 0; p < 320; p++)
    {
        color(redaccum[p] / n, grnaccum[p] / n, bluaccum[p] / n);
        pixel(p, x);
    }
    vspan = vshrinkrgb;
}
void vshrinkgrey(int x, int yt, int yb)
{
    unsigned int n, p, g;

    memset(greyaccum, 0, 320 * sizeof(unsigned int));
    hspan = hstretchgrey;
    vspan = hshrinkgrey;
    n = yb - yt + 1;
    do
    {
        line(0, 0, pbmwidth, 319);
        for (p = 0; p < 320; p++)
        {
            greyaccum[p] += greyscan[p];
        }
    } while (++yt <= yb);
    for (p = 0; p < 320; p++)
    {
        g = greyaccum[p] / n;
        color(g, g, g);
        pixel(p, x);
    }
    vspan = vshrinkgrey;
}
/*
 * World's dumbest routine to read PGM/PNM files.
 */
int main(int argc, char **argv)
{
    int x, pbmrgb, scale, mode;
    char pbmstring[80];
    float gammafunc, pbmaspect;

    for (x = 0; x < 256; x++)
        gamma[x] = x;
    left   =
    top    = 0;
    right  = 319;
    bottom = 199;
    scale  = 0; // Stretch or keep aspect
    mode   = MODE_BEST;
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
        else if (argv[1][1] == 'a')
            scale = 1;
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
    fgets(pbmstring, 80, pbmfile);
    if (pbmstring[0] == 'P' && pbmstring[1] == '6')
        pbmrgb = 1;
    else if (pbmstring[0] == 'P' && pbmstring[1] == '5')
        pbmrgb = 0;
    else
    {
        fprintf(stderr, "Not a valid PBM file.\n");
        return -1;
    }
    do
    {
        fgets(pbmstring, 80, pbmfile);
    } while (pbmstring[0] == '#');
    if (sscanf(pbmstring, "%d %d %d", &pbmwidth, &pbmheight, &pbmdepth) != 3)
    {
        if (sscanf(pbmstring, "%d %d", &pbmwidth, &pbmheight) != 2)
        {
            fprintf(stderr, "Bad PBM header.\n");
            return -1;
        }
        do
        {
            fgets(pbmstring, 80, pbmfile);
        } while (pbmstring[0] == '#');
        if (sscanf(pbmstring, "%d", &pbmdepth) != 1)
        {
            fprintf(stderr, "Bad PBM header.\n");
            return -1;
        }
    }
    if (scale)
    {
        pbmaspect = (float)pbmwidth/(float)pbmheight;
        if (pbmaspect > SCREEN_ASPECT)
        {
            x       = 100.0 * (1.0 - SCREEN_ASPECT / pbmaspect);
            top     = x;
            bottom -= x;
        }
        else
        {
            x      = 160.0 * (1.0 - pbmaspect / SCREEN_ASPECT);
            left   = x;
            right -= x;
        }
    }
    if (!gfxmode(mode))
    {
        fprintf(stderr, "Unable to set graphics mode.\n");
        return -1;
    }
    //
    // Use span line routines to stretch/shrink source image to 320x200
    //
    if (pbmrgb)
    {
        hspan = vstretchrgb;
        vspan = vshrinkrgb;
    }
    else
    {
        hspan = vstretchgrey;
        vspan = vshrinkgrey;
    }
    line(top, 0, bottom, pbmheight-1);
    getch();
    restoremode();
    return 0;
}

