void hspan8brush(int xl, int xr, int y)
{
    unsigned char far *pix;
    unsigned char *brush;
    
    pix = vidmem + y * 320 + xl;
    brush = brush8 + (y & 3) * 4;
    do
    {
        *pix++ = brush[xl & 3];
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
void vspan8brush(int x, int yt, int yb)
{
    unsigned char far *pix;
    unsigned char *brush;

    pix = vidmem + yt * 320 + x;
    brush = brush8 + (x & 3);
    do
    {
        *pix   = brush[(yt & 3) * 4];
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
void pixel8brush(int x, int y)
{
    vidmem[y * 320 + x] = brush8[(y & 3) * 4 + (x & 3)];
}
void pixel8(int x, int y)
{
    vidmem[y * 320 + x] = idx8;
}
void pixel8rgb(int x, int y, int red, int grn, int blu)
{
    int dither;

    dither              = dithmatrix[y & 3][x & 3];
    vidmem[y * 320 + x] = mapr[red+dither] | mapg[grn+dither] | mapb[blu+dither*2];
}

