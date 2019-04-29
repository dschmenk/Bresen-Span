#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include "gfx.h"

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
unsigned NumPolys;
unsigned NumSorted;

/*
 * Object coordinates and polygon definitions
 */
#define X_PIXELS        320
#define Y_PIXELS        200
#define X_FOV           300.0
#define Y_FOV           300.0
#define Z_FOV           25.0
#define X_ORG           X_PIXELS/2.0
#define Y_ORG           Y_PIXELS/2.0

#define SHADE           255.0

#define BLACK           0

#define	DIST            150.0
#define MAX_DIST        600.0
#define MIN_DIST        0.0

#define MAX_VERTEX      10
#define MIN_VERTEX      3

typedef struct
{
    float xWorld;
    float yWorld;
    float zWorld;
    float xScreen;
    float yScreen;
    float zScreen;
    float Shade;
} vertex, * pvertex;

typedef struct
{
    unsigned Count;
    unsigned Base;
    unsigned Index[MAX_VERTEX];
} polygon, * ppolygon;

pvertex   Vertex;
ppolygon  Polygon;
polygon **SortedPoly;
float    *SortedDepth;

/*
 * sine and cosine lookup tables
 */
float sine[361];
float cosine[361];

/*
 * Normalize a vector
 */
void Normalize(float *x, float *y, float *z)
{
	float Magnitude;

	Magnitude = sqrt((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
	if (Magnitude > 0.0)
    {
		*x /= Magnitude;
		*y /= Magnitude;
		*z /= Magnitude;
	}
}

/*
 * Calc amount of light at vertex
 */
float Shade(float px, float py, float pz, float nx, float ny, float nz)
{
    float lx, ly, lz, CosTheta;

    lx = xLight - px;
    ly = yLight - py;
    lz = zLight - pz;
    Normalize(&lx, &ly, &lz);
    /*
     * Dot product
     */
    CosTheta = lx * nx + ly * ny + lz * nz;
    if (CosTheta < 0.0)
        CosTheta = -CosTheta;
    return((Ambient + Diffuse * CosTheta) * SHADE + 0.5);
}

/*
 * Read in Object File
 */
void InitRender(char * ObjFile)
{
    FILE    *fp;
    unsigned i, j;
    unsigned Circum, *UseCount;
    double   Rad;
    float    x1, x2, y1, y2, z1, z2;
    float    xNorm, yNorm, zNorm;
    float    shade;

    /*
     * Fill in trig lookup tables
     */
    for (Circum = 0; Circum < 361; Circum++)
    {
        Rad = (double)Circum / 57.29578;
        sine[Circum] = (float)sin(Rad);
    }
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
    /*
     * Open polygon database
     */
    if ((fp = fopen(ObjFile, "r")) == NULL)
    {
        fprintf(stderr, "Error. File %s not found.\n", ObjFile);
        exit(1);
    }
    else
    {
        printf("Reading file: %s\n", ObjFile);
        /*
         * Read lighting parameters and vertex/polygon counts
         */
        fscanf(fp, "%f %f %f", &xLight, &yLight, &zLight);
        fscanf(fp, "%f %f", &Ambient, &Diffuse);
        fscanf(fp, "%d %d", &NumVerts, &NumPolys);
        /*
         * Allocate coordinate arrays
         */
        Vertex  = malloc(sizeof(vertex) * NumVerts);
        if (Vertex == NULL)
        {
            fprintf(stderr, "Couldn't malloc vertex array\n");
            exit(1);
        }
        /*
         * Allocate polygon structures
         */
        Polygon = malloc(sizeof(polygon) * NumPolys);
        if (Polygon == NULL)
        {
            fprintf(stderr, "Couldn't malloc polygon array\n");
            exit(1);
        }
        SortedPoly = (polygon **)malloc(sizeof(polygon *) * (NumPolys+1));
        SortedDepth = malloc(sizeof(float) * (NumPolys+1));
        /*
         * Read in the verteces
         */
        printf("Reading the vertex coordinates\n");
        for (i = 0; i < NumVerts; i++)
            fscanf(fp, "%f %f %f", &Vertex[i].xWorld, &Vertex[i].yWorld, &Vertex[i].zWorld);
        /*
         * Read in the polygon vertex lists
         */
        printf("Reading the polygon vertex lists\n");
        for (i = 0; i < NumPolys; i++)
        {
            /*
             * Get vertex count
             */
            fscanf(fp, "%d", &Polygon[i].Count);
            /*
             * Little info on structure size
             */
            if (Polygon[i].Count > MAX_VERTEX)
            {
                fprintf(stderr, "Polygon[%d] exceeded max vertex count of %d\n", i, MAX_VERTEX);
                Polygon[i].Count = MAX_VERTEX;
            }
            if (Polygon[i].Count < MIN_VERTEX)
            {
                fprintf(stderr, "Polygon[%d] doesn't have minimum vertex count of %d\n", i, MIN_VERTEX);
                exit(1);
            }
            for (j = 0; j < Polygon[i].Count; j++)
                fscanf(fp, "%d", &Polygon[i].Index[j]);
            /*
             * Get base color index
             */
            fscanf(fp, "%d", &Polygon[i].Base);
        }
        /*
         * Done with the database
         */
        printf("Closing the file\n");
        fclose(fp);
        /*
         * Free up use count array
         */
        free(UseCount);
    }
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
    int   np, i, j, p, c, SkipPoly;
    float x1, y1, x2, y2, zMin;

    /*
     * Update view coordinates
     */
    WorldToScreen();
    /*
     * Rasterize each polygon
     */
    color(0, 0, 0);
    clear();
    NumSorted = 0;
    SortedDepth[0] = 0.0;
    for (np = 0; np < NumPolys; np++)
    {
        SkipPoly = 0;
        zMin     = MAX_DIST;
        for (j = 0; j < Polygon[np].Count; j++)
        {
            if (Vertex[Polygon[np].Index[j]].zScreen < 0.0)
            {
                SkipPoly = 1;
                break;
            }
            if (Vertex[Polygon[np].Index[j]].zScreen < zMin)
                zMin = Vertex[Polygon[np].Index[j]].zScreen;
        }
        if (!SkipPoly)
        {
            /*
             * Insert polygon into sorted list
             */
            j = NumSorted / 2;
            p = j / 2;
            while (p)
            {
                if (zMin < SortedDepth[j])
                    j -= p;
                else
                    j += p;
                p /= 2;
            }
            while (j < NumSorted && zMin > SortedDepth[j]) j++;
            while (j && zMin < SortedDepth[j-1]) j--;
            for (i = NumSorted; i > j; i--)
            {
                SortedPoly[i]  = SortedPoly[i-1];
                SortedDepth[i] = SortedDepth[i-1];
            }
            SortedPoly[j]  = &Polygon[np];
            SortedDepth[j] = zMin;
            NumSorted++;
        }
    }
    for (np = NumSorted - 1; np >= 0; np--)
    {
        c = DIST * 3.0 - SortedDepth[np] * 1.4;
        if (c > 255) c = 255;
        if (c < 0)   c = 0;
        if (fill)
        {
            hidden ? color(0, 0, 0) : color(c, c, c);
            beginfill();
            p = SortedPoly[np]->Index[SortedPoly[np]->Count - 1];
            for (j = 0; j < SortedPoly[np]->Count; j++)
            {
                i = SortedPoly[np]->Index[j];
                line(Vertex[p].xScreen, Vertex[p].yScreen, Vertex[i].xScreen, Vertex[i].yScreen);
                p = i;
            }
            endfill();
        }
        if (!fill || hidden)
        {
            color(c, c, c);
            p = SortedPoly[np]->Index[SortedPoly[np]->Count - 1];
            for (j = 0; j < SortedPoly[np]->Count; j++)
            {
                i = SortedPoly[np]->Index[j];
                outline(Vertex[p].xScreen, Vertex[p].yScreen, Vertex[i].xScreen, Vertex[i].yScreen);
                p = i;
            }
        }
    }
    /*
     * Update the display with the off screen buffer
     */
    flip();
}

int main(int argc, char *argv[])
{
    int mode;

    fill   = 1;
    aa     = 0;
    hidden = 1;
    mode   = MODE_BEST;
    while (argc > 1 && argv[1][0] == '-')
    {
        if (argv[1][1] == 'f')
            fill = argv[1][2] - '0';
        else if (argv[1][1] == 'a')
            aa = argv[1][2] - '0';
        else if (argv[1][1] == 'h')
            hidden = argv[1][2] - '0';
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
    InitRender(argv[1]);
    gfxmode(mode);
    render(BACK_PAGE);
    outline = aa ? aaline : line;
    dist = 200;
    tilt = 330;
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