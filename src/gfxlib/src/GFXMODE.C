#include <bios.h>
#include <dos.h>
#include "gfx.h"

unsigned char display_page, render_page;
unsigned char far *page_addr[2];
unsigned char far *renderbuff;
static int orgmode;
static int leftedge[200], rightedge[200];
static int maxscan = 199;
static int minscan = 0;
void (*fillhspan)(int xl, int xr, int y);
void (*fillvspan)(int x, int yt, int yb);
void (*clear)(void);
void (*flip)(int sync);
void (*render)(int page);
void (*color)(int red, int grn, int blu);
void (*hspan)(int xl, int xr, int y);
void (*vspan)(int x, int yt, int yb);
void (*pixel)(int x, int y);
void (*aapixel)(int x, int y, int alpha);
void (*aahspan)(int xl, int xr, int y);
void (*aavspan)(int x, int yt, int yb);

int gfxmode8(int mono);
int gfxmode4(int mono);
int gfxmode2(int mono);
void gfxrender(int page);

int gfxmode(int modeflags)
{
    union REGS regs;
    /*
     * Get current mode
     */
    regs.x.ax = 0x0F00;
    int86(0x10, &regs, &regs);
    orgmode = regs.h.al;
    /*
     * Initiaze buffers.
     */
    display_page = 0;
    render_page  = 0;
    render       = gfxrender;
    /*
     * Initialize edge fill arrays.
     */
    for (minscan = 0; minscan < 200; minscan++)
    {
        leftedge[minscan]  = 320;
        rightedge[minscan] = -1;
    }
    /*
     * Map best mode.
     */
    if (modeflags & MODE_8BPP)
    {
        if (gfxmode8(modeflags))
            return 8;
        modeflags |= MODE_4BPP;
    }
    if (modeflags & MODE_4BPP)
    {
        if (gfxmode4(modeflags))
            return 4;
        modeflags |= MODE_2BPP;
    }
    if (modeflags & MODE_2BPP)
    {
        if (gfxmode2(modeflags))
            return 2;
    }
    return 0;
}
void restoremode(void)
{
    union REGS regs;

    regs.x.ax = orgmode;
    int86(0x10, &regs, &regs);
}
/*
 * Set render page.
 */
static void gfxrender(int page)
{
    union REGS regs;

    render_page = (page & 1) ^ display_page;
    renderbuff  = page_addr[render_page];
}
/*
 * Fill mode.
 */
static void hfill(int xl, int xr, int y)
{
    if (y >= 0 && y <= 199)
    {
        if (y < minscan) minscan = y;
        if (y > maxscan) maxscan = y;
        if (leftedge[y]  > xl) leftedge[y]  = xl;
        if (rightedge[y] < xr) rightedge[y] = xr;
    }
}
static void vfill(int x, int yt, int yb)
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
void beginfill(void)
{
    /*
     * Override span filling routines.
     */
    fillhspan = hspan;
    fillvspan = vspan;
    hspan     = hfill;
    vspan     = vfill;
    minscan   = 200;
    maxscan   = -1;
}
void endfill(void)
{
    int i;

    /*
     * Fill edge array.
     */
    for (i = minscan; i <= maxscan; i++)
    {
        if (leftedge[i]  < 0)   leftedge[i]  = 0;
        if (rightedge[i] > 319) rightedge[i] = 319;
        if (leftedge[i] <= rightedge[i])
            fillhspan(leftedge[i], rightedge[i], i);
        leftedge[i]  = 320;
        rightedge[i] = -1;
    }
    /*
     * Reset span filling routines.
     */
    hspan   = fillhspan;
    vspan   = fillvspan;
}

