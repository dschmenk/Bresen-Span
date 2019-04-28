
#include <stdio.h>
#include <bios.h>
#include "gfx.h"

/*
 * VGA Registers
 */
#define VGA_MISC        0x03C2
#define VGA_SEQ         0x03C4
#define VGA_GC          0x03CE
#define VGA_CRTC        0x03D4
#define VGA_STATUS      0x03DA

extern unsigned char display_page, render_page;
extern unsigned char far *page_addr[2];
extern unsigned char far *renderbuff;
unsigned char idx4, rgb4[3], packbrush4[4][4];
unsigned long brush4[4];
/*
 * 8x4 dither matrix (4x4 replicated twice horizontally to fill byte).
 */
static unsigned long ddithmask[16] = // Color dither
{
    0x00000000L,
    0x88000000L,
    0x88002200L,
    0x8800AA00L,
    0xAA00AA00L,
    0xAA44AA00L,
    0xAA44AA11L,
    0xAA44AA55L,
    0xAA55AA55L,
    0xAADDAA55L,
    0xAADDAA77L,
    0xAADDAAFFL,
    0xAAFFAAFFL,
    0xEEFFAAFFL,
    0xEEFFBBFFL,
    0xEEFFFFFFL,
};
static unsigned long bdithmask[16] = // Color dither
{
    0x00000000L,
    0x88000000L,
    0x88002200L,
    0x8800AA00L,
    0xAA00AA00L,
    0xAA44AA00L,
    0xAA44AA11L,
    0xAA44AA55L,
    0xAA55AA55L,
    0xAADDAA55L,
    0xAADDAA77L,
    0xAADDAAFFL,
    0xAAFFAAFFL,
    0xEEFFAAFFL,
    0xEEFFBBFFL,
    0xFFFFFFFFL
};
static unsigned long idithmask[16] = // Brightness dither
{
    0x00000000L,
    0x11000000L,
    0x11004400L,
    0x11005500L,
    0x55005500L,
    0x55225500L,
    0x55225588L,
    0x552255AAL,
    0x55AA55AAL,
    0x55BB55AAL,
    0x55BB55EEL,
    0x55BB55FFL,
    0x55FF55FFL,
    0x77FF55FFL,
    0x77FFDDFFL,
    0xFFFFFFFFL,
};
static signed char dithmatrix[4][4] =
{
    {  0, 12,  3, 15},
    {  8,  4, 11,  7},
    {  2, 14,  1, 13},
    { 10,  6,  9,  5}
};
static unsigned char greyramp[] = 
{
    0, 8, 1, 2, 3, 9, 4, 5, 10, 11, 6, 7, 12, 13, 14, 15
};
static unsigned char mapgrey[256+16];
unsigned int pixmask[] =
{
    0x8008, 0x4008, 0x2008, 0x1008, 0x0808, 0x0408, 0x0208, 0x0108
};
unsigned char amul4[4][16] =
{
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
	{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
	{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}
};

void clear4(void);
void flip4(void);
void mono4rgb(int red, int grn, int blu);
void color4rgb(int red, int grn, int blu);
void pixel4brush(int x, int y);
void hspan4brush(int xl, int xr, int y);
void vspan4brush(int x, int yt, int yb);
void pixel4(int x, int y);
void hspan4(int xl, int xr, int y);
void vspan4(int x, int yt, int yb);
void pixel4alpha(int x, int y, int alpha);

int gfxmode4(int modeflags)
{
    union REGS regs;
    int c;

    /*
     * Set mode 0x0D (320x200x4) and verify.
     */
    regs.x.ax = 0x000D;
    int86(0x10, &regs, &regs);
    regs.x.ax = 0x0F00;
    int86(0x10, &regs, &regs);
    if (regs.h.al != 0x0D)
        return 0;
    page_addr[0] = (unsigned char far *)0xA0000000L;
    page_addr[1] = (unsigned char far *)0xA0002000L;
    renderbuff   = page_addr[0];
    if (modeflags & MODE_MONO)
    {
	    /*
	     * Fill greyscale mapping arrays.
	     */
	    for (c = 0; c < 256+16; c++)
		    mapgrey[c] = greyramp[((unsigned char)(c * 16 / (256+16)))];
        color = mono4rgb;
    }
    else
    {
        /*
         * Reprogram palette to better match RGB.
         */
#if 0
        for (c = 0; c < 16; c++)
        {
            if (c == 8)
            {
                regs.h.bh = 0x38;
            }
            else
            {
#if 0
                regs.h.bh  = c & 1 ? (c & 8 ? 0x09 : 0x01) : 0;
                regs.h.bh |= c & 2 ? (c & 8 ? 0x12 : 0x02) : 0;
                regs.h.bh |= c & 4 ? (c & 8 ? 0x24 : 0x04) : 0;
#else
                regs.h.bh  = ((c & 7) << 3) | ((c & 8) ? c & 7 : 0);
#endif
            }
            regs.h.bl  = c;
            regs.x.ax  = 0x1000;
            int86(0x10, &regs, &regs);
        }
#endif
        color = color4rgb;
    }
    /*
     * Function pointers.
     */
    clear   = clear4;
    flip    = flip4;
    aapixel = pixel4alpha;
    aahspan = hspan4;
    aavspan = vspan4;
    if (modeflags & MODE_NODITHER)
    {
        pixel = pixel4;
        hspan = hspan4;
        vspan = vspan4;
    }
    else
    {
        pixel = pixel4brush;
        hspan = hspan4brush;
        vspan = vspan4brush;
    }
    return 1;
}
/*
 * Flip display page.
 */
static void flip4(void)
{
    unsigned int displaybuff;

    display_page ^= 1;
    render_page  ^= 1;
    renderbuff    =               page_addr[render_page];
    displaybuff   = (unsigned int)page_addr[display_page];
    while(  inp(VGA_STATUS) & 0x08);  // Wait for current retrace
    outpw(VGA_CRTC,(displaybuff & 0xFF00) | 0x0C);
    while(!(inp(VGA_STATUS) & 0x08)); // Wait for next retrace
}
/*
 * Build a dithered brush.
 */
static void mono4rgb(int red, int grn, int blu)
{
    int i, row, plane;
    unsigned char pix;
    unsigned long far *pscan;

    rgb4[RED] = red;
    rgb4[GRN] = grn;
    rgb4[BLU] = blu;
    i = (red >> 2) + (grn >> 1) + (blu >> 2);
    idx4      = mapgrey[i];
    /*
     * Dither grey map to 16 levels.
     */
    for (row = 0; row < 4; row++)
    {
        packbrush4[row][0] = mapgrey[i+dithmatrix[row][0]];
        packbrush4[row][1] = mapgrey[i+dithmatrix[row][1]];
        packbrush4[row][2] = mapgrey[i+dithmatrix[row][2]];
        packbrush4[row][3] = mapgrey[i+dithmatrix[row][3]];
    }
    /*
     * Convert packed brush to planar and copy to off-screen memory.
     */
    pscan = (unsigned long far *)(0xA0000000L + 200*40);
    outpw(VGA_GC, 0x0005); // Set write mode 0
    outpw(VGA_GC, 0xFF08); // Set pixmask
    for (plane = 0; plane < 4; plane++)
    {
        outpw(VGA_SEQ, (0x0100 << plane) | 0x02); // Set write plane enable
        pix           = ((packbrush4[0][0] >> plane) & 1) << 3;
        pix          |= ((packbrush4[0][1] >> plane) & 1) << 2;
        pix          |= ((packbrush4[0][2] >> plane) & 1) << 1;
        pix          |= ((packbrush4[0][3] >> plane) & 1) << 0;
        brush4[plane] = (unsigned long)(pix | (pix << 4)) << 0;
        pix           = ((packbrush4[1][0] >> plane) & 1) << 3;
        pix          |= ((packbrush4[1][1] >> plane) & 1) << 2;
        pix          |= ((packbrush4[1][2] >> plane) & 1) << 1;
        pix          |= ((packbrush4[1][3] >> plane) & 1) << 0;
        brush4[plane]|= (unsigned long)(pix | (pix << 4)) << 8;
        pix           = ((packbrush4[2][0] >> plane) & 1) << 3;
        pix          |= ((packbrush4[2][1] >> plane) & 1) << 2;
        pix          |= ((packbrush4[2][2] >> plane) & 1) << 1;
        pix          |= ((packbrush4[2][3] >> plane) & 1) << 0;
        brush4[plane]|= (unsigned long)(pix | (pix << 4)) << 16;
        pix           = ((packbrush4[3][0] >> plane) & 1) << 3;
        pix          |= ((packbrush4[3][1] >> plane) & 1) << 2;
        pix          |= ((packbrush4[3][2] >> plane) & 1) << 1;
        pix          |= ((packbrush4[3][3] >> plane) & 1) << 0;
        brush4[plane]|= (unsigned long)(pix | (pix << 4)) << 24;
        *pscan = brush4[plane];
    }
}
static void color4rgb(int red, int grn, int blu)
{
    unsigned char v, pix, *pbrush;
    int row, plane;
    unsigned long far *pscan;

    rgb4[RED] = red;
    rgb4[GRN] = grn;
    rgb4[BLU] = blu;
    /*
     * Find MAX(R,G,B)
     */
    if (red >= grn && red >= blu)
        v = red;
    else if (grn >= red && grn >= blu)
        v = grn;
    else // if (blue >= grn && blu >= red)
        v = blu;
    if (v > 127) // 50%-100% brightness
    {
        /*
         * Fill brush based on scaled RGB values (brightest -> 100% -> 0x0F).
         */
        brush4[BRI] = idithmask[(v >> 3) & 0x0F]; //  Bright dither is opposit color dither
        brush4[RED] = bdithmask[(red << 4) / (v + 8)];
        brush4[GRN] = bdithmask[(grn << 4) / (v + 8)];
        brush4[BLU] = bdithmask[(blu << 4) / (v + 8)];
        idx4        = 0x08
                    | ((red & 0x80) >> 5)
                    | ((grn & 0x80) >> 6)
                    | ((blu & 0x80) >> 7);
    }
    else // 0%-50% brightness
    {
        /*
         * Fill brush based on RGB values.
         */
        //if (v < 72 && v > 7 && ((v - red) + (v - grn) + (v - blu) < 12))
        if (v < 68 && v > 7 && ((v - red) + (v - grn) + (v - blu) < 12))
        {
            brush4[BRI] = v > 63 ? 0xFFFFFFFFL : bdithmask[(v >> 2) - 1];
            brush4[RED] = 0;
            brush4[GRN] = 0;
            brush4[BLU] = 0;
            idx4       = 0x08;
        }
        else
        {
            brush4[BRI] = 0;
            brush4[RED] = ddithmask[red >> 3];
            brush4[GRN] = ddithmask[grn >> 3];
            brush4[BLU] = ddithmask[blu >> 3];
            idx4       = ((red & 0x40) >> 4)
                       | ((grn & 0x40) >> 5)
                       | ((blu & 0x40) >> 6);
        }
    }
    /*
     * Convert planar brush to packed pixels.
     */
    for (row = 0; row < 4; row++)
    {
        pbrush = (unsigned char *)brush4 + row;
        /*
         * Extract pixel value from IRGB planes.
         */
        pix  = (pbrush[BRI*4] & 0x08) >> 0;
        pix |= (pbrush[RED*4] & 0x08) >> 1;
        pix |= (pbrush[GRN*4] & 0x08) >> 2;
        pix |= (pbrush[BLU*4] & 0x08) >> 3;
        packbrush4[row][0] = pix;
        pix  = (pbrush[BRI*4] & 0x04) << 1;
        pix |= (pbrush[RED*4] & 0x04) << 0;
        pix |= (pbrush[GRN*4] & 0x04) >> 1;
        pix |= (pbrush[BLU*4] & 0x04) >> 2;
        packbrush4[row][1] = pix;
        pix  = (pbrush[BRI*4] & 0x02) << 2;
        pix |= (pbrush[RED*4] & 0x02) << 1;
        pix |= (pbrush[GRN*4] & 0x02) << 0;
        pix |= (pbrush[BLU*4] & 0x02) >> 1;
        packbrush4[row][2] = pix;
        pix  = (pbrush[BRI*4] & 0x01) << 3;
        pix |= (pbrush[RED*4] & 0x01) << 2;
        pix |= (pbrush[GRN*4] & 0x01) << 1;
        pix |= (pbrush[BLU*4] & 0x01) << 0;
        packbrush4[row][3] = pix;
    }
    /*
     * Copy brush to off-screen memory.
     */
    pscan = (unsigned long far *)(0xA0000000L + 200*40);
    outpw(VGA_GC, 0x0005); // Set write mode 0
    outpw(VGA_GC, 0xFF08); // Set pixmask
    for (plane = 0; plane < 4; plane++)
    {
        outpw(VGA_SEQ, (0x0100 << plane) | 0x02); // Set write plane enable
        *pscan = brush4[plane];
    }
}
