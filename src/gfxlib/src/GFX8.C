
#include <bios.h>
#include <dos.h>
#include "gfx.h"

extern unsigned char display_page, render_page;
extern unsigned char far *page_addr[2];
extern unsigned char far *renderbuff;

#define RGB2I(r,g,b)    (((b)&0xC0)|(((g)&0xE0)>>2)|(((r)&0xE0)>>5))
/*
 * VGA Registers
 */
#define VGA_MISC        0x03C2
#define VGA_SEQ         0x03C4
#define VGA_GC          0x03CE
#define VGA_CRTC        0x03D4
#define VGA_STATUS      0x03DA

unsigned int CRTC_320x200[] =
{
    0x5F00,                  // Horizontal total
    0x4F01,                  // Horizontal displayed
    0x5002,                  // Start horizontal blanking
    0x8203,                  // End horizontal blanking
    0x5404,                  // Start horizontal sync
    0x8005,                  // End horizontal sync
    0x2813,                  // Row address
    0xBF06,                  // Vertical total
    0x1F07,                  // Overflow
    0x9C10,                  // Start vertical sync
    0x8E11,                  // End vertical sync
    0x8F12,                  // Vertical displayed
    0x9615,                  // Start vertical blanking
    0xB916,                  // End vertical blanking
    0x4109,                  // Cell height: 2 scan lines
    0x0014,                  // Double word mode off
    0xE317,                  // Byte mode on
    0
};

signed char colordither[4][4] =
{
    {  0, 25,   6, 31},
    { 17,  8,  23, 14},
    {  4, 29,   2, 27},
    { 21, 12,  19, 10}
};
signed char monodither[4][4] =
{
    { 0, 3, 0, 3},
    { 2, 1, 2, 1},
    { 0, 3, 0, 3},
    { 2, 1, 2, 1}
};
unsigned char mapi[256+32];
unsigned char mapr[256+32];
unsigned char mapg[256+32];
unsigned char mapb[256+64];
unsigned char idx8, rgb8[3], brush8[4][4];
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

void flip8(int sync);
void clear8(void);
void mono8rgb(int red, int grn, int blu);
void color8rgb(int red, int grn, int blu);
void hspan8brush(int xl, int xr, int y);
void vspan8brush(int x, int yt, int yb);
void pixel8brush(int x, int y);
void hspan8(int xl, int xr, int y);
void vspan8(int x, int yt, int yb);
void pixel8(int x, int y);
void pixel8rgb(int x, int y, int red, int grn, int blu);
void pixel8argb(int x, int y, int alpha);
void pixel8amono(int x, int y, int alpha);

int gfxmode8(int modeflags)
{
    union  REGS  regs;
    struct SREGS sregs;
    int red, grn, blu, c;
    unsigned char colors[256*3], far *far_colors;
    unsigned int far *vidmem = (unsigned int far *)0xA0000000L;

    /*
     * Set mode 0x13 (320x200x8) and verify.
     */
    regs.x.ax = 0x0013;
    int86(0x10, &regs, &regs);
    regs.x.ax = 0x0F00;
    int86(0x10, &regs, &regs);
    if (regs.h.al != 0x13)
        return 0;
    outpw(VGA_GC, 0xAA08); // Set pixmask
    if (inp(VGA_GC+1) != 0xAA) // Some EGA cards are stoopid and set mode 0x13
        return 0;
    page_addr[0] = (unsigned char far *)0xA0000000L;
    page_addr[1] = (unsigned char far *)0xA0004000L;
    renderbuff   = page_addr[0];
    /*
     * Program color registers.
     */
    c = 0;
    if (modeflags & MODE_MONO)
    {
        for (blu = 0; blu < 64; blu++)
        {
            colors[c++] = blu;
            colors[c++] = blu;
            colors[c++] = blu;
        }
        while (blu++ < 256)
        {
            colors[c++] = 255;
            colors[c++] = 255;
            colors[c++] = 255;
        }
        /*
         * Fill greyscal mapping arrays
         */
        for (c = 0; c < 256+32; c++)
            mapi[c] = (((unsigned char)(c * 255L / (256+32))) >> 2);
        aapixel = pixel8amono;
        color = mono8rgb;
    }
    else
    {
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
        /*
         * Fill RGB mapping arrays
         */
        for (c = 0; c < 256+32; c++)
        {
            mapr[c] = (((unsigned char)(c * 255L / (256+32))) >> 5);
            mapg[c] = (((unsigned char)(c * 255L / (256+32))) >> 2) & 0x38;
        }
        for (c = 0; c < 256+64; c++)
            mapb[c] = ((unsigned char)(c * 255L / (256+64))) & 0xC0;
        aapixel = pixel8argb;
        color   = color8rgb;
    }
    far_colors = colors;
    regs.x.ax  = 0x1012;
    regs.x.bx  = 0;
    regs.x.cx  = 256;
    regs.x.dx  = FP_OFF(far_colors);
    sregs.es   = FP_SEG(far_colors);
    int86x(0x10, &regs, &regs, &sregs);
    /*
     * Unchain memory (Mode Y)
     */
    outpw(VGA_SEQ, 0x0604);	// Set memory mode disable chain4
    outpw(VGA_SEQ, 0x0100); // Reset
    outp(VGA_MISC, 0x63);   // 400 scan lines, 25 MHz clock
    outpw(VGA_SEQ, 0x0300); // Restart
    outp(VGA_CRTC, 0x11);   // Unlock CRTC
    outp(VGA_CRTC+1, inp(VGA_CRTC+1) & 0x7F);
    for (c = 0; CRTC_320x200[c]; c++)
        outpw(VGA_CRTC, CRTC_320x200[c]);
    outp(VGA_CRTC, 0x11);   // Lock CRTC
    outp(VGA_CRTC+1, inp(VGA_CRTC+1) | 0x80);
    outpw(VGA_GC, 0xFF08); // Set pixmask
    clear8();
    /*
     * Function pointers.
     */
    clear   = clear8;
    flip    = flip8;
    aahspan = hspan8;
    aavspan = vspan8;
    if (modeflags & MODE_NODITHER)
    {
        pixel = pixel8;
        hspan = hspan8;
        vspan = vspan8;
    }
    else
    {
        pixel = pixel8brush;
        hspan = hspan8brush;
        vspan = vspan8brush;
    }
    return 1;
}
/*
 * Flip display page.
 */
static void flip8(int sync)
{
    unsigned int displaybuff;

    display_page ^= 1;
    render_page  ^= 1;
    renderbuff    =               page_addr[render_page];
    displaybuff   = (unsigned int)page_addr[display_page];
    if (sync)
    {
        while(  inp(VGA_STATUS) & 0x08);  // Wait for current retrace
        outpw(VGA_CRTC,(displaybuff & 0xFF00) | 0x0C);
        while(!(inp(VGA_STATUS) & 0x08)); // Wait for next retrace
    }
    else
        outpw(VGA_CRTC,(displaybuff & 0xFF00) | 0x0C);
}
static void mono8rgb(int red, int grn, int blu)
{
    int row, col, i;
    unsigned char *brush;
    unsigned char far *pscan;

    rgb8[RED] = red;
    rgb8[GRN] = grn;
    rgb8[BLU] = blu;
    i         = (red >> 2) + (grn >> 1) + (blu >> 2);
    idx8      = i >> 2;
    brush     = (unsigned char *)brush8;
    for (row = 0; row < 4; row++)
    {
        *brush++ = mapi[i + monodither[row][0]];
        *brush++ = mapi[i + monodither[row][1]];
        *brush++ = mapi[i + monodither[row][2]];
        *brush++ = mapi[i + monodither[row][3]];
    }
    /*
     * Copy brush to off-screen memory.
     */
    pscan = (unsigned char far *)(0xA0000000L + 80*200);
    for (col = 0; col < 4; col++)
    {
        outpw(VGA_SEQ, (0x0100 << col) | 0x02); // Set write plane enable
        pscan[0] = brush8[0][col];
        pscan[1] = brush8[1][col];
        pscan[2] = brush8[2][col];
        pscan[3] = brush8[3][col];
    }
}
static void color8rgb(int red, int grn, int blu)
{
    int row, col, dither;
    unsigned char *brush;
    unsigned char far *pscan;

    rgb8[RED] = red;
    rgb8[GRN] = grn;
    rgb8[BLU] = blu;
    idx8      = RGB2I(red, grn, blu);
    brush     = (unsigned char *)brush8;
    for (row = 0; row < 4; row++)
    {
        dither = colordither[row][0];
        *brush++ = mapr[red+dither] | mapg[grn+dither] | mapb[blu+dither*2];
        dither = colordither[row][1];
        *brush++ = mapr[red+dither] | mapg[grn+dither] | mapb[blu+dither*2];
        dither = colordither[row][2];
        *brush++ = mapr[red+dither] | mapg[grn+dither] | mapb[blu+dither*2];
        dither = colordither[row][3];
        *brush++ = mapr[red+dither] | mapg[grn+dither] | mapb[blu+dither*2];
    }
    /*
     * Copy brush to off-screen memory.
     */
    pscan = (unsigned char far *)(0xA0000000L + 80*200);
    for (col = 0; col < 4; col++)
    {
        outpw(VGA_SEQ, (0x0100 << col) | 0x02); // Set write plane enable
        pscan[0] = brush8[0][col];
        pscan[1] = brush8[1][col];
        pscan[2] = brush8[2][col];
        pscan[3] = brush8[3][col];
    }
}

