#include <bios.h>
#include <dos.h>

#define RGB2I(r,g,b)    (((b)&0xC0)|(((g)&0xE0)>>2)|(((r)&0xE0)>>5))
#define RED 0
#define GRN 1
#define BLU 2

unsigned char far *vidmem = (unsigned char far *)0xA0000000L;
static int orgmode, dithernoise;
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
unsigned char idx8, rgb8[3], brush8[16];
void (*hspan)(int xl, int xr, int y);
void (*vspan)(int x, int yt, int yb);
void (*pixel)(int x, int y);
void (*aapixel)(int x, int y, int alpha);

#if 0 // C or ASM pixels
#include "pixspan8.c"
#else
void hspan8brush(int xl, int xr, int y);
void vspan8brush(int x, int yt, int yb);
void pixel8brush(int x, int y);
void hspan8(int xl, int xr, int y);
void vspan8(int x, int yt, int yb);
void pixel8(int x, int y);
#endif
#if 0 // C or ASM line?
#include "fastline.c"
#endif
//
// Clamped and shifted Alpha*RGB multiplication tables.
//
unsigned char amulr[8][8] = 
{
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01}, 
	{0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02}, 
	{0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03}, 
	{0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04}, 
	{0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x04, 0x05}, 
	{0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06}, 
	{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07} 
};	
unsigned char amulg[8][8] = // Above table shifted left by 3
{
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08}, 
	{0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10}, 
	{0x00, 0x00, 0x08, 0x08, 0x10, 0x10, 0x18, 0x18}, 
	{0x00, 0x08, 0x08, 0x10, 0x10, 0x18, 0x18, 0x20}, 
	{0x00, 0x08, 0x08, 0x10, 0x18, 0x20, 0x20, 0x28}, 
	{0x00, 0x08, 0x10, 0x18, 0x18, 0x20, 0x28, 0x30}, 
	{0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38}
};	
unsigned char amulb[8][4] = // Above table shifted left by 2
{
	{0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0x00, 0x40, 0x40}, 
	{0x00, 0x00, 0x40, 0x40}, 
	{0x00, 0x40, 0x40, 0x80}, 
	{0x00, 0x40, 0x40, 0x80}, 
	{0x00, 0x40, 0x80, 0xC0}, 
	{0x00, 0x40, 0x80, 0xC0}
};	
void aapixel8(int x, int y, int alpha)
{
    int idx;
    unsigned char far *pix;

    if (!(alpha >>= 5)) return; // Scale for the limited 3-3-2 RGB palette
    if (alpha < 0x07) // Could technically remove last row of mul arrays
    {
        pix    = vidmem + y * 320 + x;
        idx    = *pix;
        *pix   = amulr[alpha][rgb8[RED] >> 5] + amulr[0x07^alpha][ idx       & 0x07]
               | amulg[alpha][rgb8[GRN] >> 5] + amulg[0x07^alpha][(idx >> 3) & 0x07]
               | amulb[alpha][rgb8[BLU] >> 6] + amulb[0x07^alpha][(idx >> 6) & 0x03];
    }
    else
        *(vidmem + y * 320 + x) = idx8;
}

#include "aaline.c"

void setmodex(int modex, unsigned char noise)
{
    union  REGS  regs;
    struct SREGS sregs;
    int red, grn, blu;
    long c;
    unsigned char colors[256*3], far *far_colors;

    if ((dithernoise = noise))
    {
        hspan   = hspan8brush;
        vspan   = vspan8brush;
        pixel   = pixel8brush;
        aapixel = aapixel8;
    }
    else
    {
        hspan   = hspan8;
        vspan   = vspan8;
        pixel   = pixel8;
        aapixel = aapixel8;
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
                colors[c++] = red;
                colors[c++] = grn;
                colors[c++] = blu;
            }
        }
    }
    far_colors = colors;
    regs.x.ax  = 0x1012;
    regs.x.bx  = 0;
    regs.x.cx  = 256;
    regs.x.dx  = FP_OFF(far_colors);
    sregs.es   = FP_SEG(far_colors);
    int86x(0x10, &regs, &regs, &sregs);
}
void restoremode(void)
{
    union REGS regs;

    regs.x.ax = orgmode;
    int86(0x10, &regs, &regs);
}
void brush8rgb(int red, int grn, int blu)
{
    int x, y, dither;
    unsigned char *brush;

    rgb8[RED] = red;
    rgb8[GRN] = grn;
    rgb8[BLU] = blu;
    idx8      = RGB2I(red, grn, blu);
    if (dithernoise)
    {
        brush = brush8;
        for (y = 0; y < 4; y++)
            for (x = 0; x < 4; x++)
            {
                dither = dithmatrix[y][x];
                *brush++ = mapr[red+dither] | mapg[grn+dither] | mapb[blu+dither*2];
            }
    }
}
unsigned char dither8rgb(int x, int y, int red, int grn, int blu)
{
    int dither;

    dither = dithmatrix[y & 3][x & 3];
    return mapr[red+dither] | mapg[grn+dither] | mapb[blu+dither*2];
}
