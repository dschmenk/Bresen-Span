void hspan8brush(int xl, int xr, int y);
void vspan8brush(int x, int yt, int yb);
void pixel8brush(int x, int y);
void hspan8(int xl, int xr, int y);
void vspan8(int x, int yt, int yb);
void pixel8(int x, int y);
void line(int x1, int y1, int x2, int y2);
void setmodex(int modex, unsigned char noise);
void restoremode(void);
void brush8rgb(int red, int grn, int blu);
void pixel8rgb(int x, int y, int red, int grn, int blu);
unsigned char dither8rgb(int x, int y, int red, int grn, int blu);
extern void (*hspan)(int xl, int xr, int y);
extern void (*vspan)(int x, int yt, int yb);
extern void (*pixel)(int x, int y);
extern void (*aapixel)(int x, int y, int alpha);
#define RGB2I(r,g,b)    (((b)&0xC0)|(((g)&0xE0)>>2)|(((r)&0xE0)>>5))


