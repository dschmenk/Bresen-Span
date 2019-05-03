#define X_RES           320
#define Y_RES           200
#define BRI             3
#define RED             2
#define GRN             1
#define BLU             0
#define MODE_NODITHER   16
#define MODE_BEST       8
#define MODE_8BPP       8
#define MODE_4BPP       4
#define MODE_2BPP       2
#define MODE_MONO       1
#define FRONT_PAGE      0
#define BACK_PAGE       1
int gfxmode(int modeflags);
void restoremode(void);
void line(int x1, int y1, int x2, int y2);
void aaline(int x1, int y1, int x2, int y2);
void bitblt(int xl, int yt, int width, int height, int xoffset, int yoffset, unsigned char far *src, int pitch);
void beginfill(void);
void endfill(void);
void text(int x, int y, char *string);
extern void (*clear)(void);
extern void (*flip)(void);
extern void (*render)(int page);
extern void (*color)(int red, int grn, int blu);
extern void (*hspan)(int xl, int xr, int y);
extern void (*vspan)(int x, int yt, int yb);
extern void (*pixel)(int x, int y);
extern void (*aahspan)(int xl, int xr, int y);
extern void (*aavspan)(int x, int yt, int yb);
extern void (*aapixel)(int x, int y, int alpha);
