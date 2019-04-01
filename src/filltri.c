#include <stdio.h>
#include <conio.h>
#include <bios.h>
#include <dos.h>

#define RGB2I(r,g,b)    (((b)&0xC0)|(((g)&0xE0)>>2)|(((r)&0xE0)>>5))

static unsigned char far *vidmem = (unsigned char far *)0xA0000000L;
static int orgmode;
static unsigned int mapsel[] = {0x0102, 0x0202, 0x0402, 0x0802};
signed char dithmatrix[4][4] =
{
    {  0, 25,   6, 31},
    { 17,  8,  23, 14},
    {  4, 29,   2, 27},
    { 21, 12,  19, 10}
};
unsigned char mapr[256+32];
unsigned char mapg[256+32];
unsigned char mapb[256+64];
unsigned red8, grn8, blu8, idx8;
int leftedge[200], rightedge[200];
int maxscan = 199;
int minscan = 0;
void (*fillspan)(int xl, int xr, int y);
void (*hspan)(int xl, int xr, int y);
void (*vspan)(int x, int yt, int yb);
extern void hspan8rgb(int xl, int xr, int y);
extern void vspan8rgb(int x, int yt, int yb);
extern void pixel8rgb(int x, int y);
extern void hspan8(int xl, int xr, int y);
extern void vspan8(int x, int yt, int yb);
extern void pixel8(int x, int y);

unsigned long gettime(void)
{
    struct dostime_t time;

    _dos_gettime(&time);
    return time.hour * 360000UL + time.minute * 6000UL + time.second * 100UL + time.hsecond;
}
void setmodex(int modex, unsigned char noise)
{
    union REGS regs;
    int shift, offset, x, y, red, grn, blu;
    long c;

    //
    // Adjust dither matrix for noise
    //
    if (noise < 5)
    {
        shift  = 5 - noise;
        offset = 16 - (16 >> shift);
        for (y = 0; y < 4; y++)
            for (x = 0; x < 4; x++)
                dithmatrix[y][x] = (dithmatrix[y][x] >> shift) + offset;
    }
    //
    // Fill RGB mapping arrays
    //
    for (c = 0; c < 256+32; c++)
    {
        mapr[c] = (((unsigned char)(c * 255 / (256+32))) >> 5);
        mapg[c] = (((unsigned char)(c * 255 / (256+32))) >> 2) & 0x38;
    }
    for (c = 0; c < 256+64; c++)
        mapb[c] = ((unsigned char)(c * 255 / (256+64))) & 0xC0;
    //
    // Get current mode
    //
    regs.x.ax = 0x0F00;
    int86(0x10, &regs, &regs);
    orgmode = regs.h.al;
    regs.x.ax = 0x0013;
    int86(0x10, &regs, &regs);
    c = 0;
    for (blu = 0; blu < 64; blu += 21)
    {
        for (grn = 0; grn < 64; grn += 9)
        {
            for (red = 0; red < 64; red += 9)
            {
                regs.x.ax = 0x1010;
                regs.x.bx = c++;
                regs.h.cl = blu;
                regs.h.ch = grn;
                regs.h.dh = red;
                int86(0x10, &regs, &regs);
            }
        }
    }
}
void restoremode(void)
{
    union REGS regs;

    regs.x.ax = orgmode;
    int86(0x10, &regs, &regs);
}
void hfill(int xl, int xr, int y)
{
    if (y > 0 && y < 199)
    {
        if (y < minscan) minscan = y;
        if (y > maxscan) maxscan = y;
        if (leftedge[y]  > xl) leftedge[y]  = xl;
        if (rightedge[y] < xr) rightedge[y] = xr;
    }
}
void vfill(int x, int yt, int yb)
{
    if (yt < 0)   yt = 0;
    if (yb > 199) yb = 199;
    while (yt <= yb)
    {
        if (yt < minscan) minscan = yt;
        if (yt > maxscan) maxscan = yt;
        if (leftedge[yt]  > x) leftedge[yt]  = x;
        if (rightedge[yt] < x) rightedge[yt] = x;
        yt++;
    }
}
void fill(int red, int grn, int blu)
{
    red8 = red; grn8 = grn; blu8 = blu; idx8=RGB2I(red8,grn8,blu8);
    while (minscan <= maxscan)
    {
        fillspan(leftedge[minscan], rightedge[minscan], minscan);
        leftedge[minscan]  = 319;
        rightedge[minscan] = 0;
        minscan++;
    }
    minscan = 199;
    maxscan = 0;
}
int main(int argc, char **argv)
{
    int c;
    int xv[3], yv[3], ix[3], iy[3];
    int rgb[3], irgb[3];
    c = 5;
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'd')
    {
        c = argv[1][2] - '0';
        argc--;
        argv++;
    }
    setmodex(0x13, c);
    if (c)
        fillspan = hspan8rgb;
    else
        fillspan = hspan8;
    hspan = hfill;
    vspan = vfill;
    for (minscan = maxscan = 0; minscan < 200; minscan++)
    {
        leftedge[minscan]  = 319;
        rightedge[minscan] = 0;
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
        fast_line(xv[0], yv[0], xv[1], yv[1]);
        fast_line(xv[1], yv[1], xv[2], yv[2]);
        fast_line(xv[2], yv[2], xv[0], yv[0]);
        fill(rgb[0], rgb[1], rgb[2]);
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