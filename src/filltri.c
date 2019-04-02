#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include "rgb8.h"

int leftedge[200], rightedge[200];
int maxscan = 199;
int minscan = 0;
void (*fillspan)(int xl, int xr, int y);

unsigned long gettime(void)
{
    struct dostime_t time;

    _dos_gettime(&time);
    return time.hour * 360000UL + time.minute * 6000UL + time.second * 100UL + time.hsecond;
}
void hfill(int xl, int xr, int y)
{
    if (y >= 0 && y <= 199)
    {
        if (y < minscan) minscan = y;
        if (y > maxscan) maxscan = y;
        if (leftedge[y]  > xl) leftedge[y]  = xl;
        if (rightedge[y] < xr) rightedge[y] = xr;
    }
}
void vfill(int x, int yt, int yb)
{
    int i;

    if (yt < 0)   yt = 0;
    if (yb > 199) yb = 199;
    for (i = yt; i <= yb; i++)
    {
        if (i < minscan) minscan = i;
        if (i > maxscan) maxscan = i;
        if (leftedge[i]  > x) leftedge[i]  = x;
        if (rightedge[i] < x) rightedge[i] = x;
    }
}
void polyfill(void)
{
    int i;

    for (i = minscan; i <= maxscan; i++)
    {
        if (leftedge[i]  < 0)   leftedge[i]  = 0;
        if (rightedge[i] > 319) rightedge[i] = 319;
        if (leftedge[i] <= rightedge[i])
            fillspan(leftedge[i], rightedge[i], i);
        leftedge[i]  = 320;
        rightedge[i] = -1;
    }
    minscan = 199;
    maxscan = 0;
}
int main(int argc, char **argv)
{
    int c, f;
    int xv[3], yv[3], ix[3], iy[3];
    int rgb[3], irgb[3];
    c = 1;
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'd')
    {
        c = argv[1][2] - '0';
        argc--;
        argv++;
    }
    f = 1;
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'f')
    {
        f = argv[1][2] - '0';
        argc--;
        argv++;
    }
    setmodex(0x13, c);
    /*
     * Override span filling routines.
     */
    if (f)
    {
        fillspan = hspan;
        hspan    = hfill;
        vspan    = vfill;
    }
    for (minscan = maxscan = 0; minscan < 200; minscan++)
    {
        leftedge[minscan]  = 320;
        rightedge[minscan] = -1;
    }
    srand(gettime());
    for (c = 0; c < 3; c++)
    {
        xv[c]   = rand() % 320;
        yv[c]   = rand() % 200;
        ix[c]   = ((rand() % 5) - 3) | 1;
        iy[c]   = ((rand() % 5) - 3) | 1;
        rgb[c]  = rand() % 256;
        irgb[c] = ((rand() % 5) - 3) | 1;
    }
    while (!kbhit())
    {
        brush8rgb(rgb[0], rgb[1], rgb[2]);
        line(xv[0], yv[0], xv[1], yv[1]);
        line(xv[1], yv[1], xv[2], yv[2]);
        line(xv[2], yv[2], xv[0], yv[0]);
        if (f)
            polyfill();
        for (c = 0; c < 3; c++)
        {
            xv[c] += ix[c];
            if (xv[c] < 0)
            {
                ix[c] = -ix[c];
                xv[c] = 0;
            }
            if (xv[c] > 319)
            {
                ix[c] = -ix[c];
                xv[c] = 319;
            }
            yv[c] += iy[c];
            if (yv[c] < 0)
            {
                iy[c] = -iy[c];
                yv[c] = 0;
            }
            if (yv[c] > 199)
            {
                iy[c] = -iy[c];
                yv[c] = 199;
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
    }
    getch();
    restoremode();
    return 0;
}
