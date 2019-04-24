#include <stdio.h>
#include <bios.h>
#include "gfx.h"

extern int render_page;
extern unsigned char far *page_addr[2];
extern unsigned char far *renderbuff;
unsigned char idx2, rgb2[3];
unsigned int scanaddr[200];
unsigned long brush2;
unsigned long monodither2[64] = {
    0x00000000, // 0
    0x00000040, // 1
    0x00040040, // 2
    0x00440040, // 3
    0x00440044, // 4
    0x00441044, // 5
    0x01441044, // 6
    0x01441044, // 6
    0x11441044, // 7
    0x11441144, // 8
    0x11445144, // 9
    0x11445144, // 9
    0x15445144, // 10
    0x55445144, // 11
    0x55445544, // 12
    0x55445544, // 12
    0x55445554, // 13
    0x55455554, // 14
    0x55555554, // 15
    0x55555554, // 15
    0x55555555, // 16
    0x55555595, // 17
    0x55595595, // 18
    0x55595595, // 18
    0x55995595, // 19
    0x55995599, // 20
    0x55996599, // 21
    0x55996599, // 21
    0x56996599, // 22
    0x66996599, // 23
    0x66996699, // 24
    0x66996699, // 24
    0x6699A699, // 25
    0x6A99A699, // 26
    0xAA99A699, // 27
    0xAA99A699, // 27
    0xAA99AA99, // 28
    0xAA99AAA9, // 29
    0xAA9AAAA9, // 30
    0xAA9AAAA9, // 30
    0xAAAAAAA9, // 31
    0xAAAAAAAA, // 32
    0xAAAAAAEA, // 33
    0xAAAAAAEA, // 33
    0xAAAEAAEA, // 34
    0xAAEEAAEA, // 35
    0xAAEEAAEE, // 36
    0xAAEEAAEE, // 36
    0xAAEEBAEE, // 37
    0xABEEBAEE, // 38
    0xBBEEBAEE, // 39
    0xBBEEBAEE, // 39
    0xBBEEBBEE, // 40
    0xBBEEFBEE, // 41
    0xBFEEFBEE, // 42
    0xBFEEFBEE, // 42
    0xFFEEFBEE, // 43
    0xFFEEFFEE, // 44
    0xFFEEFFFE, // 45
    0xFFEEFFFE, // 45
    0xFFEFFFFE, // 46
    0xFFFFFFFE, // 47
    0xFFFFFFFF, // 48
    0xFFFFFFFF  // 48
};
unsigned long colordither2[16] = {
    0x00000000, // 0
    0x00000060, // 1
    0x00090060, // 2
    0x00690060, // 3
    0x00690069, // 4
    0x00699069, // 5
    0x06699069, // 6
    0x96699069, // 7
    0x96699669, // 8
    0x966996F9, // 9
    0x966F96F9, // 10
    0x96FF96F9, // 11
    0x96FF96FF, // 12
    0x96FFF6FF, // 13
    0x9FFFF6FF, // 14
//  0xFFFFF6FF, // 15
    0xFFFFFFFF  // 16
};
unsigned char amul2[4][4] =
{
	{0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0x00, 0x55, 0x55}, 
	{0x00, 0x55, 0x55, 0xAA}, 
	{0x00, 0x55, 0xAA, 0xFF}
};

void clear2(void);
void flip2(int sync);
void render2(int page);
void mono2rgb(int red, int grn, int blu);
void color2rgb(int red, int grn, int blu);
void pixel2brush(int x, int y);
void hspan2brush(int xl, int xr, int y);
void vspan2brush(int x, int yt, int yb);
void pixel2(int x, int y);
void hspan2(int xl, int xr, int y);
void vspan2(int x, int yt, int yb);
void pixel2alpha(int x, int y, int alpha);

int gfxmode2(int modeflags)
{
    union REGS regs;
    int scan;

    /*
     * Set mode 0x04 (320x200x2) and verify.
     */
    regs.x.ax = 0x0004;
    int86(0x10, &regs, &regs);
    regs.x.ax = 0x0F00;
    int86(0x10, &regs, &regs);
    if (regs.h.al != 0x04)
        return 0;
    /*
     * Fill in scanline/framebuffer addresses
     */
    for (scan = 0; scan < 200; scan++)
        scanaddr[scan] = (scan >> 1) * 80 + (scan & 1) * 8192;
    renderbuff   = (unsigned char far *)0xB8000000L;
    page_addr[0] = renderbuff;
    /*
     * Allocate back buffer.
     */
    regs.x.ax = 0x4800; // Allocate bx paragraphs
    regs.x.bx = 0x0400; // 16K (1024 paragraphs)
    int86(0x21, &regs, &regs);
    page_addr[1] = (unsigned char far *)((unsigned long)regs.x.ax << 16);
    if (*(unsigned int far *)0xC0000000L == 0xAA55) // On EGA/VGA or CGA?
    {
        /*
         * Reprogram palette to greyscale (EGA/VGA only).
         */
        regs.x.bx = 0x0000;
        regs.x.ax = 0x1000;
        int86(0x10, &regs, &regs);
        regs.x.bx = 0x3801;
        regs.x.ax = 0x1000;
        int86(0x10, &regs, &regs);
        regs.x.bx = 0x0702;
        regs.x.ax = 0x1000;
        int86(0x10, &regs, &regs);
        regs.x.bx = 0x3F03;
        regs.x.ax = 0x1000;
        int86(0x10, &regs, &regs);
        color = mono2rgb;
    }
    else
    {
        /*
         * Set color palette (CGA only).
         */
        outp(0x3D9, 0x30);
        color = modeflags & MODE_MONO ? mono2rgb : color2rgb;
    }
    /*
     * Function pointers.
     */
    clear   = clear2;
    flip    = flip2;
    render  = render2;
    aapixel = pixel2alpha;
    aahspan = hspan2;
    aavspan = vspan2;
    if (modeflags & MODE_NODITHER)
    {
        pixel = pixel2;
        hspan = hspan2;
        vspan = vspan2;
    }
    else
    {
        pixel = pixel2brush;
        hspan = hspan2brush;
        vspan = vspan2brush;
    }
    return 1;
}
/*
 * Set render page.
 */
void render2(int page)
{
    union REGS regs;

    render_page = page & 1;
    renderbuff  = page_addr[render_page];
}
/*
 * Build a dithered brush.
 */
static void mono2rgb(int red, int grn, int blu)
{
    int i;

    rgb2[RED] = red;
    rgb2[GRN] = grn;
    rgb2[BLU] = blu;
    i         = ((red >> 2) + (grn >> 1) + (blu >> 2)) >> 2;
    brush2    = monodither2[i];
    i         = (red >> 1) | (grn) | (blu >> 1);
    idx2      = (i & 0xC0) | ((i >> 2) & 0x30) | ((i >> 4) & 0x0C) | ((i >> 6) & 0x03);
}
static void color2rgb(int red, int grn, int blu)
{
    int i;

    rgb2[RED] = red;
    rgb2[GRN] = grn;
    rgb2[BLU] = blu;
    i         = ((red >> 2) + (grn >> 1) + (blu >> 2)) >> 4;
    brush2    = colordither2[i];
    i         = (red >> 1) | (grn) | (blu >> 1);
    idx2      = (i & 0xC0) | ((i >> 2) & 0x30) | ((i >> 4) & 0x0C) | ((i >> 6) & 0x03);
}
