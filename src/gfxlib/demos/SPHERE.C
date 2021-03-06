#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include "gfx.h"

/*
 * Object coordinates and polygon definitions
 */
#define X_PIXELS        320
#define Y_PIXELS        200
#define X_FOV           200.0
#define Y_FOV           200.0
#define Z_FOV           100.0
#define X_ORG           X_PIXELS/2.0
#define Y_ORG           Y_PIXELS/2.0

#define	DIST            100.0
#define SCALE           75.0

#define MAX_VERTS       1100
#define MAX_TRIS        2100

typedef struct
{
    float xWorld;
    float yWorld;
    float zWorld;
    int   xScreen;
    int   yScreen;
    int   zScreen;
} vertex, * pvertex;

typedef struct
{
    float X;
    float Y;
    float Z;
} coord, * pcoord;

typedef struct
{
    unsigned Index[3];
} triangle, * ptriangle;

vertex   *Vertex;
triangle *Triangle;
/*
 * The Platonic Solids
 */
#define PHI 1.618034
coord OctaVerts[] =
{
    { 1.0,  0.0,  0.0},
    {-1.0,  0.0,  0.0},
    { 0.0,  1.0,  0.0},
    { 0.0, -1.0,  0.0},
    { 0.0,  0.0,  1.0},
    { 0.0,  0.0, -1.0}
};
triangle OctaTris[] =
{
    {0, 2, 4},
    {2, 0, 5},
    {3, 0, 4},
    {0, 3, 5},
    {2, 1, 4},
    {1, 2, 5},
    {1, 3, 4},
    {3, 1, 5}
};
coord TetraVerts[] =
{
    { 1.0,  1.0,  1.0},
    { 1.0, -1.0, -1.0},
    {-1.0,  1.0, -1.0},
    {-1.0, -1.0,  1.0}
};
triangle TetraTris[] =
{
    {3, 2, 1},
    {2, 3, 0},
    {1, 0, 3},
    {0, 1, 2}
};
coord IcosaVerts[] =
{
    { PHI,  1.0,  0.0},
    {-PHI,  1.0,  0.0},
    { PHI, -1.0,  0.0},
    {-PHI, -1.0,  0.0},
    { 1.0,  0.0,  PHI},
    { 1.0,  0.0, -PHI},
    {-1.0,  0.0,  PHI},
    {-1.0,  0.0, -PHI},
    { 0.0,  PHI,  1.0},
    { 0.0, -PHI,  1.0},
    { 0.0,  PHI, -1.0},
    { 0.0, -PHI, -1.0}
};
triangle IcosaTris[] =
{
    { 0,  8,  4},
    { 0,  5, 10},
    { 2,  4,  9},
    { 2, 11,  5},
    { 1,  6,  8},
    { 1, 10,  7},
    { 3,  9,  6},
    { 3,  7, 11},
    { 0, 10,  8},
    { 1,  8, 10},
    { 2,  9, 11},
    { 3, 11,  9},
    { 4,  2,  0},
    { 5,  0,  2},
    { 6,  1,  3},
    { 7,  3,  1},
    { 8,  6,  4},
    { 9,  4,  6},
    {10,  5,  7},
    {11,  7,  5}
};
#define NUM_PLATONICS   3
struct platonic
{
    int PlatNumVerts, PlatNumTris;
    coord    *PlatVerts;
    triangle *PlatTris;
} Platonic[NUM_PLATONICS] =
{
    { 6,  8, OctaVerts,  OctaTris},
    { 4,  4, TetraVerts, TetraTris},
    {12, 20, IcosaVerts, IcosaTris}
};

int fill, aa, hidden;
void (*outline)(int x1, int y1, int x2, int y2);
/*
 * Viewing angle and position
 */
int tilt;
int  rot;
float dist;
float xEye;
float yEye;
float zEye;

/*
 * Light position
 */
float xLight;
float yLight;
float zLight;

/*
 * Ambient & diffuse light
 */
float Ambient;
float Diffuse;

/*
 * Object parameters
 */

unsigned NumVerts;
unsigned NumTris;

/*
 * sine and cosine lookup tables
 */
float sine[361];
float cosine[361];

/*
 * Normalize a vector
 */
void Normalize(float *x, float *y, float *z, float scale)
{
	float Magnitude;

    if (scale != 0.0)
    	Magnitude = sqrt((*x) * (*x) + (*y) * (*y) + (*z) * (*z)) / scale;
	if (Magnitude != 0.0)
    {
		*x /= Magnitude;
		*y /= Magnitude;
		*z /= Magnitude;
	}
}
/*
 * Search existing vertices for match
 */
int AddVert(float x, float y, float z)
{
    int i;

    for (i = NumVerts-1; i > 8; i--)
    {
        if (Vertex[i].xWorld == x && Vertex[i].yWorld == y && Vertex[i].zWorld == z)
            return i;
    }
    if (NumVerts + 1 >= MAX_VERTS)
    {
        fprintf(stderr, "Out of Vertices!\n");
        exit(-1);
    }
    Vertex[NumVerts].xWorld = x;
    Vertex[NumVerts].yWorld = y;
    Vertex[NumVerts].zWorld = z;
    NumVerts++;
    return NumVerts - 1;
}
/*
 * Init render tables and values
 */
void InitRender(void)
{
    unsigned Circum;
    double   Rad;

    /*
     * Fill in trig lookup tables
     */
    for (Circum = 0; Circum < 361; Circum++)
        sine[Circum] = (float)sin((double)Circum / 57.29578);
    for (Circum = 0; Circum < 361; Circum++)
        cosine[Circum] = sine[(Circum + 90) % 360];
    /*
     * Set initial view parameters
     */
	tilt = 0;
	rot  = 0;
	xEye = 0.0;
	yEye = 0.0;
	zEye = -DIST;
    dist = DIST;
}
void Tesselate(int t, int subdiv)
{
    int j, p, n;
    int v[3];
    float x, y, z;
    /*
     * Add intermediate vertices along edges
     */
    for (j = 0; j < 3; j++)
    {
        p = Triangle[t].Index[j];
        n = Triangle[t].Index[j == 2 ? 0 : j+1];
        x = (Vertex[p].xWorld + Vertex[n].xWorld) / 2.0;
        y = (Vertex[p].yWorld + Vertex[n].yWorld) / 2.0;
        z = (Vertex[p].zWorld + Vertex[n].zWorld) / 2.0;
        Normalize(&x, &y, &z, SCALE);
        v[j] = AddVert(x, y, z);
    }
    /*
     * Add new triangles
     */
    if (NumTris + 3 >= MAX_TRIS)
    {
        fprintf(stderr, "Out of Triangles!\n");
        exit(-1);
    }
    Triangle[NumTris+0].Index[0] = Triangle[t].Index[1];
    Triangle[NumTris+0].Index[1] = v[1];
    Triangle[NumTris+0].Index[2] = v[0];
    Triangle[NumTris+1].Index[0] = Triangle[t].Index[2];
    Triangle[NumTris+1].Index[1] = v[2];
    Triangle[NumTris+1].Index[2] = v[1];
    Triangle[NumTris+2].Index[0] = v[0];
    Triangle[NumTris+2].Index[1] = v[1];
    Triangle[NumTris+2].Index[2] = v[2];
    /*
     * Shrink original triangle
     */
    Triangle[t].Index[1] = v[0];
    Triangle[t].Index[2] = v[2];
    n         = NumTris;
    NumTris  += 3;
    if (--subdiv)
    {
        Tesselate(t,   subdiv);
        Tesselate(n+0, subdiv);
        Tesselate(n+1, subdiv);
        Tesselate(n+2, subdiv);
    }
}
void BuildSphere(int obj, int subdiv)
{
    int i, nt;

    if (obj > 2)    obj    = 2;
    if (subdiv > 4) subdiv = 4;
    Vertex = malloc(sizeof(vertex) * MAX_VERTS);
    if (!Vertex)
    {
        fprintf(stderr, "Unable to allocate Vertex array size: %u\n", sizeof(vertex) * MAX_VERTS);
        exit(-1);
    }
    Triangle = malloc(sizeof(triangle) * MAX_TRIS);
    if (!Triangle)
    {
        fprintf(stderr, "Unable to allocate Triangle array size: %u\n", sizeof(triangle) * MAX_TRIS);
        exit(-1);
    }
    NumVerts = Platonic[obj].PlatNumVerts;
    for (i = 0; i < NumVerts; i++)
    {
        Vertex[i].xWorld = Platonic[obj].PlatVerts[i].X;
        Vertex[i].yWorld = Platonic[obj].PlatVerts[i].Y;
        Vertex[i].zWorld = Platonic[obj].PlatVerts[i].Z;
        Normalize(&Vertex[i].xWorld, &Vertex[i].yWorld, &Vertex[i].zWorld, SCALE);
    }
    NumTris = Platonic[obj].PlatNumTris;
    for (i = 0; i < NumTris; i++)
    {
        Triangle[i].Index[0] = Platonic[obj].PlatTris[i].Index[0];
        Triangle[i].Index[1] = Platonic[obj].PlatTris[i].Index[1];
        Triangle[i].Index[2] = Platonic[obj].PlatTris[i].Index[2];
    }
    if (subdiv > 0)
    {
        nt = NumTris;
        for (i = 0; i < nt; i++)
            Tesselate(i, subdiv);
    }
    printf("Number Triangles: %d\nNumber of vertices: %d\n", NumTris, NumVerts);
    printf("Press any key to display...\n");
    getch();
}                 
/*
 * Convert world coordinates to screen coordinates
 */
void WorldToScreen(void)
{
    unsigned nv;
    float    s1, s2;
    float    c1, c2;
    float    x, y, z;
    float    ye, xe, ze, dc;

	s1 = sine[rot];
	s2 = sine[tilt];
	c1 = cosine[rot];
	c2 = cosine[tilt];
    for (nv = 0; nv < NumVerts; nv++)
    {
		x = Vertex[nv].xWorld;
		y = Vertex[nv].yWorld;
		z = Vertex[nv].zWorld;
		xe =   x * c1 + z * s1;
		ye = ( x * s1 - z * c1) * s2 + y * c2;
		ze = (-x * s1 + z * c1) * c2 + y * s2 + dist;
		if (ze > 0.0)
        {
			Vertex[nv].xScreen = X_ORG + X_FOV * xe / ze;
			Vertex[nv].yScreen = Y_ORG - Y_FOV * ye / ze;
			Vertex[nv].zScreen = ze;
		}                             
		else
        {
            ze -= 1.0;
			Vertex[nv].xScreen = X_ORG + X_FOV * xe * (-ze);
			Vertex[nv].yScreen = Y_ORG - Y_FOV * ye * (-ze);
			Vertex[nv].zScreen = ze;
	    }
	}
}

void UpdateRender(void)
{
    int   nt, i, j, p, Cull;
    float x1, y1, x2, y2;

    /*
     * Update view coordinates
     */
    WorldToScreen();
    /*
     * Rasterize each polygon
     */
    color(0, 0, 0);
    clear();
    Cull = 0;
    for (nt = 0; nt < NumTris; nt++)
    {
        if (hidden)
        {
            p  = Triangle[nt].Index[0];
            i  = Triangle[nt].Index[1];
            j  = Triangle[nt].Index[2];
            x1 = Vertex[p].xScreen - Vertex[i].xScreen;
            y1 = Vertex[p].yScreen - Vertex[i].yScreen;
            x2 = Vertex[p].xScreen - Vertex[j].xScreen;
            y2 = Vertex[p].yScreen - Vertex[j].yScreen;
            Cull = x1 * y2 - x2 * y1 <= 0;
        }
        /*
         * Cross product for backface culling
         */
        if (!Cull)
        {
            color(255, 255, 255);
            p = Triangle[nt].Index[2];
            i = Triangle[nt].Index[0];
            outline(Vertex[p].xScreen, Vertex[p].yScreen, Vertex[i].xScreen, Vertex[i].yScreen);
            p = Triangle[nt].Index[1];
            outline(Vertex[p].xScreen, Vertex[p].yScreen, Vertex[i].xScreen, Vertex[i].yScreen);
            i = Triangle[nt].Index[2];
            outline(Vertex[p].xScreen, Vertex[p].yScreen, Vertex[i].xScreen, Vertex[i].yScreen);
        }
    }
    /*
     * Update the display with the off screen buffer
     */
    flip();
}

int main(int argc, char *argv[])
{
    int plat, subdiv, mode;

    fill   = 1;
    aa     = 0;
    hidden = 1;
    subdiv = 2;
    plat   = 0;
    mode   = MODE_BEST;
    while (argc > 1 && argv[1][0] == '-')
    {
        if (argv[1][1] == 'f')
            fill = argv[1][2] - '0';
        else if (argv[1][1] == 'a')
            aa = argv[1][2] - '0';
        else if (argv[1][1] == 'h')
            hidden = argv[1][2] - '0';
        else if (argv[1][1] == 'p')
            plat = argv[1][2] - '0';
        else if (argv[1][1] == 's')
            subdiv = argv[1][2] - '0';
        else if (argv[1][1] == 'd')
            switch (argv[1][2] - '0')
            {
                case 2: mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_2BPP;
                    break;
                case 4: mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_4BPP;
                    break;
                case 8: mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_8BPP;
                    break;
            }
        else if (argv[1][1] == 'm')
            mode |= MODE_MONO;
        else if (argv[1][1] == 'n')
            mode |= MODE_NODITHER;
        argc--;
        argv++;
    }
    InitRender();
    BuildSphere(plat, subdiv);
    gfxmode(mode);
    render(BACK_PAGE);
    outline = aa ? aaline : line;
    dist = 200;
    tilt = 340;
    rot  = 0;
    do
    {
        UpdateRender();
        rot += 5;
        if (rot >= 360)
            rot = 0;

    } while (!kbhit());
    getch();
    restoremode();
    return 0;
}


