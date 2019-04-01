#include <stdio.h>
#include <bios.h>
#include <dos.h>

#define BENCHMARK
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
void (*hspan)(int xl, int xr, int y);
void (*vspan)(int x, int yt, int yb);
void (*pixel)(int x, int y);

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
#if 0 // C or ASM pixels
void hspan8rgb(int xl, int xr, int y)
{
    int dither;
    unsigned char far *pix;
    
    pix = vidmem + y * 320 + xl;
    y  &= 3;
    do
    {
        dither = dithmatrix[y][xl & 3];
        *pix++ = mapr[red8+dither] | mapg[grn8+dither] | mapb[blu8+dither*2];
    } while (++xl <= xr);
}
void hspan8(int xl, int xr, int y)
{
    int dither;
    unsigned char far *pix;
    
    pix = vidmem + y * 320 + xl;
    do
    {
        *pix++ = idx8;
    } while (++xl <= xr);
}
void vspan8rgb(int x, int yt, int yb)
{
    int dither;
    unsigned char far *pix;

    pix = vidmem + yt * 320 + x;
    x  &= 3;
    do
    {
        dither = dithmatrix[yt & 3][x];
        *pix   = mapr[red8+dither] | mapg[grn8+dither] | mapb[blu8+dither*2];
        pix   += 320;
    } while (++yt <= yb);
}
void vspan8(int x, int yt, int yb)
{
    int dither;
    unsigned char far *pix;

    pix = vidmem + yt * 320 + x;
    do
    {
        *pix   = idx8;
        pix   += 320;
    } while (++yt <= yb);
}
void pixel8rgb(int x, int y)
{
    int dither;
    dither = dithmatrix[y & 3][x & 3];
    vidmem[y * 320 + x] = mapr[red8 + dither] | mapg[grn8 + dither] | mapb[blu8 + dither*2];
}
void pixel8(int x, int y)
{
    vidmem[y * 320 + x] = idx8;
}
#else
extern void hspan8rgb(int xl, int xr, int y);
extern void vspan8rgb(int x, int yt, int yb);
extern void pixel8rgb(int x, int y);
extern void hspan8(int xl, int xr, int y);
extern void vspan8(int x, int yt, int yb);
extern void pixel8(int x, int y);
#endif
#if 0 // C or ASM line?
#include "fastline.c"
#include "line.c"
#else
extern void line(int x1, int y1, int x2, int y2);
extern void fast_line(int x1, int y1, int x2, int y2);
#endif
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
    if (c)
    {
        hspan = hspan8rgb;
        vspan = vspan8rgb;
        pixel = pixel8rgb;
    }
    else
    {
        hspan = hspan8;
        vspan = vspan8;
        pixel = pixel8;
    }
#if 0
    for (n = 100; n < 200; n++)
    {
        red8 = 255; grn8 = 255; blu8 = 255; idx8=RGB2I(red8,grn8,blu8);
        fast_line(160, 100, 319, n);
        getch();
        red8 = 0; grn8 = 0; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
        fast_line(160, 100, 319, n);
        red8 = 255; grn8 = 255; blu8 = 255; idx8=RGB2I(red8,grn8,blu8);
        line(160, 100,   319, n);
        getch();
        red8 = 0; grn8 = 0; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
        line(160, 100,   319, n);
    }
    for (n = 319; n >= 0; n--)
    {
        red8 = 255; grn8 = 255; blu8 = 255; idx8=RGB2I(red8,grn8,blu8);
        fast_line(160, 100, n, 199);
        red8 = 0; grn8 = 0; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
        line(160, 100, n, 199);
    }
#else
    normtime = gettime();
    for (n = 0; n < 200; n++)
    {
        c = n * 255 / 200;
        red8 = 0; grn8 = c; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
        line(160, 100, 319, n);
        red8 = 255-c; grn8 = 0; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
        line(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        c = n * 255 / 320;
        red8 = 0; grn8 = 0; blu8 = 255-c; idx8=RGB2I(red8,grn8,blu8);
        line(160, 100, n,   0);
        red8 = c; grn8 = c; blu8 = c; idx8=RGB2I(red8,grn8,blu8);
        line(160, 100, n, 199);
    }
    red8 = 0; grn8 = 0; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
    for (n = 0; n < 200; n++)
    {
        line(160, 100, 319, n); idx8=RGB2I(red8,grn8,blu8);
        line(160, 100,   0, n); idx8=RGB2I(red8,grn8,blu8);
    }
    for (n = 319; n >= 0; n--)
    {
        line(160, 100, n,   0); idx8=RGB2I(red8,grn8,blu8);
        line(160, 100, n, 199); idx8=RGB2I(red8,grn8,blu8);
    }
    normtime = gettime() - normtime;
    fasttime = gettime();
    for (n = 0; n < 200; n++)
    {
        c = n * 255 / 200;
        red8 = 0; grn8 = c; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
        fast_line(160, 100, 319, n);
        red8 = 255-c; grn8 = 0; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
        fast_line(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        c = n * 255 / 320;
        red8 = 0; grn8 = 0; blu8 = 255-c; idx8=RGB2I(red8,grn8,blu8);
        fast_line(160, 100, n,   0);
        red8 = c; grn8 = c; blu8 = c; idx8=RGB2I(red8,grn8,blu8);
        fast_line(160, 100, n, 199);
    }
    red8 = 0; grn8 = 0; blu8 = 0; idx8=RGB2I(red8,grn8,blu8);
    for (n = 0; n < 200; n++)
    {
        fast_line(160, 100, 319, n);
        fast_line(160, 100,   0, n);
    }
    for (n = 319; n >= 0; n--)
    {
        fast_line(160, 100, n,   0);
        fast_line(160, 100, n, 199);
    }
    fasttime = gettime() - fasttime;
#endif
    restoremode();
    printf("Normal line time = %lu\nFast line time = %lu\n", normtime, fasttime);
    return 0;
}
