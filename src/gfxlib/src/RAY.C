/**********************************************\
*                                              *
*                Simple Ray Tracing            *
*                      with                    *
*            Octree Spacial Decompostion       *
*                                              *
*                  Monochrome version          *
*                                              *
*                  Dave Schmenk                *
*                                              *
\**********************************************/

#include <stdio.h>
#include <math.h>
#include <dos.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <malloc.h>
#include "gfx.h"

#define _far            far

#define X_PIXELS        320
#define Y_PIXELS        200
#define PICT_RES        200
#define MAX_WHITE       255

#define	EYE			    1
#define	LIGHT			2
#define	MAXOBJ		    10
#define	TRUE			1
#define	FALSE			0
#define	ZERO			(1e-6)

typedef struct
{
    float x, y, z;
} vector;
	
struct boundbox
{
    float xmin, xmax;
    float ymin, ymax;
    float zmin, zmax;
};

struct vertex
{
    float xvert, yvert, zvert;
};
	
struct poly
{
    int	   color;
    int	   objnum;
    int	   polynum;
    int	   used;
    vector normal;
    int	   numvert;
    struct vertex _far *vertlist;
    struct poly   _far *next;
};

struct polylink
{
	struct poly     _far *boundpoly;
	struct polylink _far *next;
};

struct obj
{
    float kdiff, kspec, kexp;
    int   numpoly;
} object[MAXOBJ];

struct octree
{
    int level;
    struct
    {
        struct boundbox branchbox;
        union
        {
            struct polylink _far *polylist;
            struct octree   _far *next;
        } ptr;
    } branch[8];
} _far *trunk;

vector eye, aim, light;
float  winsize;
int    res, xcenter, ycenter, grey, maxlevel;

/***************************************************************************\
\***************************************************************************/

vector crossprod(vector *p1, vector *p2)
{
    vector result;
    
    result.x = p1->y * p2->z - p2->y * p1->z;
    result.y = p2->x * p1->z - p1->x * p2->z;
    result.z = p1->x * p2->y - p2->x * p1->y;
    return result;
}

float dotprod(vector *p1, vector *p2)
{
    return p1->x * p2->x + p1->y * p2->y + p1->z * p2->z;
}

void normalize(vector *vec)
{
    float magnitude;
    
    magnitude = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    if (magnitude > 0.0)
    {
        vec->x /= magnitude;
        vec->y /= magnitude;
        vec->z /= magnitude;
    }
}

void fov(int angle)
{
    float  focal;
    vector v;
    
    v.x     = eye.x - aim.x;
    v.y     = eye.y - aim.y;
    v.z     = eye.z - aim.z;
    focal   = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    winsize = focal * tan((double)angle / 114.5916);
}

/*
 * These routines insert a polygon into the octree
 */

int intersectboxbox(struct boundbox _far *box1, struct boundbox _far *box2)
{
    if (box2->xmax < box1->xmin || box1->xmax < box2->xmin
     || box2->ymax < box1->ymin || box1->ymax < box2->ymin
     || box2->zmax < box1->zmin || box1->zmax < box2->zmin)
        return FALSE;
    return TRUE;
}

struct octree _far *newbranch(int level, struct boundbox _far *boxptr)
{
    struct octree   _far *branchptr;
    struct boundbox _far *newbox;
    float  xhalf, yhalf, zhalf;
    int    i;

    if (!(branchptr = _fmalloc(sizeof(struct octree))))
    {
        fprintf(stderr, "Out of memory in newbranch()\n");
        exit(-1);
    }
    branchptr->level = level;
    xhalf = (boxptr->xmax + boxptr->xmin) / 2.0;
    yhalf = (boxptr->ymax + boxptr->ymin) / 2.0;
    zhalf = (boxptr->zmax + boxptr->zmin) / 2.0;
    for (i = 0; i < 8; i++)
    {
        branchptr->branch[i].ptr.next = NULL;
        newbox       = &branchptr->branch[i].branchbox;
        newbox->xmin = boxptr->xmin;
        newbox->xmax = boxptr->xmax;
        newbox->ymin = boxptr->ymin;
        newbox->ymax = boxptr->ymax;
        newbox->zmin = boxptr->zmin;
        newbox->zmax = boxptr->zmax;
        switch (i) // break up space into octants
        {
            case 0:
                newbox->xmin = xhalf;
                newbox->ymin = yhalf;
                newbox->zmin = zhalf;
                break;
            case 1:
                newbox->xmin = xhalf;
                newbox->ymax = yhalf;
                newbox->zmin = zhalf;
                break;
            case 2:
                newbox->xmin = xhalf;
                newbox->ymin = yhalf;
                newbox->zmax = zhalf;
                break;
            case 3:
                newbox->xmin = xhalf;
                newbox->ymax = yhalf;
                newbox->zmax = zhalf;
                break;
            case 4:
                newbox->xmax = xhalf;
                newbox->ymin = yhalf;
                newbox->zmin = zhalf;
                break;
            case 5:
                newbox->xmax = xhalf;
                newbox->ymax = yhalf;
                newbox->zmin = zhalf;
                break;
            case 6:
                newbox->xmax = xhalf;
                newbox->ymin = yhalf;
                newbox->zmax = zhalf;
                break;
            case 7:
                newbox->xmax = xhalf;
                newbox->ymax = yhalf;
                newbox->zmax = zhalf;
                break;
        }
    }
    return branchptr;
}

/*
 * Check branches to insert a bounded polygon recursively
 */
void insertbranch(struct octree _far *branchptr, struct poly _far *polyptr, struct boundbox _far *boxptr)
{
    struct polylink _far *newlink, _far *linkptr;
    int    i;

    for (i = 0; i < 8; i++)
    {
        if (intersectboxbox(boxptr, &(branchptr->branch[i].branchbox)))
        {
            if (branchptr->level < maxlevel)
            {
                if (!(branchptr->branch[i].ptr.next))
                    branchptr->branch[i].ptr.next = newbranch(branchptr->level + 1, &branchptr->branch[i].branchbox);
                insertbranch(branchptr->branch[i].ptr.next, polyptr, boxptr);
            }
            else
            {
                if (!(newlink = _fmalloc(sizeof(struct polylink))))
                {
                    fprintf(stderr, "Out of memory in insertbranch()\n");
                    exit(-1);
                }
                newlink->next      = NULL;
                newlink->boundpoly = polyptr;
                if (!(linkptr = branchptr->branch[i].ptr.polylist))
                    branchptr->branch[i].ptr.polylist = newlink;
                else
                {
                    while (linkptr->next)
                        linkptr = linkptr->next;
                    linkptr->next = newlink;
                }
                putchar('.');
            }
        }
    }
}

void insertpoly(struct poly _far *polyptr)
{
    int                 i;
    struct boundbox     polybox;
    struct vertex _far *verts;

    //
    // Find bounding box for polygon
    //
    i            = polyptr->numvert - 1;
    verts        = polyptr->vertlist;
    polybox.xmin = polybox.xmax = verts[i].xvert;
    polybox.ymin = polybox.ymax = verts[i].yvert;
    polybox.zmin = polybox.zmax = verts[i].zvert;
    while (--i >= 0)
    {
        if (verts[i].xvert > polybox.xmax) polybox.xmax = verts[i].xvert;
        if (verts[i].xvert < polybox.xmin) polybox.xmin = verts[i].xvert;
        if (verts[i].yvert > polybox.ymax) polybox.ymax = verts[i].yvert;
        if (verts[i].yvert < polybox.ymin) polybox.ymin = verts[i].yvert;
        if (verts[i].zvert > polybox.zmax) polybox.zmax = verts[i].zvert;
        if (verts[i].zvert < polybox.zmin) polybox.zmin = verts[i].zvert;
    }
    insertbranch(trunk, polyptr, &polybox);
}


/*
 * These routines search the octree looking for possible intersections
 * and builds a list of the polygons
 */
int intersectraybox(vector *p, vector *v, struct boundbox _far *box, float *t1, float *t2)
{
    float t, x, y, z;
    int   i;

    i = 0;
    if (fabs(v->x) > ZERO)
    {
        /* Xmin */
        t = (box->xmin - p->x) / v->x;
        y = p->y + t * v->y;
        if (y >= box->ymin && y <= box->ymax)
        {
            z = p->z + t * v->z;
            if (z >=box->zmin && z <= box->zmax)
                *t1 = t;
        }
        /* Xmax */
        t = (box->xmax - p->x) / v->x;
        y = p->y + t * v->y;
        if (y >= box->ymin && y <= box->ymax)
        {
            z = p->z + t * v->z;
            if (z >=box->zmin && z <= box->zmax)
                if (i++)
                {
                    *t2 = t;
                    return i;
                }
                else
                    *t1 = t;
        }
    }
    if (fabs(v->y) > ZERO)
    {
        /* Ymin */
        t = (box->ymin - p->y) / v->y;
        x = p->x + t * v->x;
        if (x >= box->xmin && x <= box->xmax)
        {
            z = p->z + t * v->z;
            if (z >=box->zmin && z <= box->zmax)
                if (i++)
                {
                    *t2 = t;
                    return i;
                }
                else
                    *t1 = t;
        }
        /* Ymax */
        t = (box->ymax - p->y) / v->y;
        x = p->x + t * v->x;
        if (x >= box->xmin && x <= box->xmax)
        {
            z = p->z + t * v->z;
            if (z >=box->zmin && z <= box->zmax)
                if (i++)
                {
                    *t2 = t;
                    return i;
                }
                else
                    *t1 = t;
        }
    }
    if (fabs(v->z) > ZERO)
    {
        /* Zmin */
        t = (box->zmin - p->z) / v->z;
        y = p->y + t * v->y;
        if (y >= box->ymin && y <= box->ymax)
        {
            x = p->x + t * v->x;
            if (x >=box->xmin && x <= box->xmax)
                if (i++)
                {
                    *t2 = t;
                    return i;
                }
                else
                    *t1 = t;
        }
        /* Zmax */
        t = (box->zmax - p->z) / v->z;
        y = p->y + t * v->y;
        if (y >= box->ymin && y <= box->ymax)
        {
            x = p->x + t * v->x;
            if (x >=box->xmin && x <= box->xmax)
                if (i++)
                    *t2 = t;
                else
                    *t1 = t;
        }
    }
    if (i) // one intersection was found, fake the second  hackety, hackety, hack
        *t2 = 0.5;
    return i;
}

void searchbranch(vector *p, vector *v, struct poly _far * _far *listptr, struct octree _far *branchptr, int type)
{
    struct polylink _far *linkptr;
    struct poly     _far *newptr;
    float  t1, t2;
    int    i;

    for (i = 0; i < 8; i++)
        if (branchptr->branch[i].ptr.next)
            if (intersectraybox(p, v, &(branchptr->branch[i].branchbox), &t1, &t2))
                if ((t1 > ZERO) && (t2 > ZERO))
                    if ((type == EYE) || ((type == LIGHT) && (t2 < 1.0) && (t1 < 1.0)))
                        if (branchptr->level != maxlevel)
                            searchbranch(p, v, listptr, branchptr->branch[i].ptr.next, type);
                        else
                        {
                            linkptr = branchptr->branch[i].ptr.polylist;
                            if ((newptr = *listptr)) // go to end of of list
                                while (newptr->next)
                                    newptr = newptr->next;
                            while (linkptr) // include octant's polygon list of possible intersection
                            {
                                if (!linkptr->boundpoly->used) // don't include ones already in list
                                {
                                    if (*listptr) // add to current list
                                    {
                                        newptr->next = linkptr->boundpoly;
                                        newptr       = newptr->next;
                                    }
                                    else // start new list
                                        *listptr = newptr = linkptr->boundpoly;
                                    newptr->next = NULL; /* terminate list */
                                    newptr->used = TRUE; /* flag polygon as used */
                                }
                                linkptr = linkptr->next;
                            }
                        }
}

struct poly _far *buildlist(vector *p, vector *v, int type)
{
    struct poly _far *listptr;

    listptr = NULL;
    searchbranch(p, v, &listptr, trunk, type);
    return listptr;
}

/*
 *    These routines do all the actual ray tracing.
 */
int reflect(struct poly _far *polyptr, vector point)
{
    vector l, n, r, v;
    float  twoldotn, cosphi, costheta;
    struct obj *objptr;
    
    n   = polyptr->normal;
    v.x = eye.x - point.x;
    v.y = eye.y - point.y;
    v.z = eye.z - point.z;
    normalize(&v);
    if (dotprod(&v, &n) < 0.0)
    {
        n.x = -n.x;
        n.y = -n.y;
        n.z = -n.z;
    }
    l.x = light.x - point.x;
    l.y = light.y - point.y;
    l.z = light.z - point.z;
    normalize(&l);
    if ((costheta = dotprod(&l, &n)) < 0.0)
        return 0; // light hidden by surface 
    twoldotn = 2.0 * costheta;
    r.x      = twoldotn * n.x - l.x;
    r.y      = twoldotn * n.y - l.y;
    r.z      = twoldotn * n.z - l.z;
    cosphi   = dotprod(&r, &v);
    objptr   = &object[polyptr->objnum];
    return (int)((objptr->kdiff * costheta + objptr->kspec * pow(cosphi, objptr->kexp)) * MAX_WHITE + 0.5);
}

int intersectraypoly(vector p1, vector p2, struct poly _far * _far *plyptr, vector *ptptr, int type)
{
    struct poly   _far *polyptr, _far *tmpptr;
    struct vertex _far *verts;
    vector ipt, tmppt;
    vector v, n, p, r, q;
    float  vdotn, t, tmpt;
    int    i, j, k, intersect;

    intersect = FALSE;
    v.x       = p2.x - p1.x;
    v.y       = p2.y - p1.y;
    v.z       = p2.z - p1.z;
    polyptr   = buildlist(&p1, &v, type);    /* get list of possible intersections */
    while (polyptr)
    {
        //
        // Backface cull
        //
        n     = polyptr->normal;
        vdotn = dotprod(&v, &n);
        if (fabs(vdotn) > 0.0)
        {
            //
            // Calculate intersection
            //
            verts = polyptr->vertlist;
            ipt.x = p1.x - verts[0].xvert; // use ipt temporarily
            ipt.y = p1.y - verts[0].yvert;
            ipt.z = p1.z - verts[0].zvert;
            if ((t = -dotprod(&ipt, &n) / vdotn) > 0.0)
            {
                //
                // Check validity if eye or light ray
                //
                if ((type == EYE) || ((type == LIGHT) && (t < 1.0)))
                {
                    //
                    // Calculate intersect point
                    //
                    ipt.x = p1.x + t * v.x;
                    ipt.y = p1.y + t * v.y;
                    ipt.z = p1.z + t * v.z;
                    //
                    // Inside outside test
                    //
                    i = polyptr->numvert;
                    k = i - 1;
                    for (j = 0; j < i; j++)
                    {
                        q.x = verts[j].xvert - verts[k].xvert;
                        q.y = verts[j].yvert - verts[k].yvert;
                        q.z = verts[j].zvert - verts[k].zvert;
                        p.x = ipt.x          - verts[k].xvert;
                        p.y = ipt.y          - verts[k].yvert;
                        p.z = ipt.z          - verts[k].zvert;
                        r   = crossprod(&q, &p);
                        if (dotprod(&r, &n) < 0.0) // quick reject
                            break;
                        k   = j;
                    }
                    if (j == i)
                        //
                        // Inside test passed
                        //
                        if (!intersect || (intersect && t < tmpt))
                        {
                            if (type == LIGHT)
                            {
                                //
                                // Quick exit if shaded
                                //
                                do
                                {
                                    polyptr->used = FALSE;
                                    polyptr       = polyptr->next;
                                } while (polyptr);
                                return TRUE;
                            }
                            tmpt      = t;
                            tmpptr    = polyptr;
                            tmppt     = ipt;
                            intersect = TRUE;
                        }
                }
            }
        }
        polyptr->used = FALSE; // clear flag for next call to buildlist
        polyptr       = polyptr->next;
    }
    if (intersect)
    {
        *plyptr = tmpptr;
        *ptptr  = tmppt;
    }
    return intersect;
}

void image(FILE *fp)
{
    char  *imagescan;
    int    x, halfres, numpixels, rgb, xscreen, yscreen, sumpix, prevrgb = -1;
    int    scanline;
    float  fullwin, winvert, hyp, focal, cosa, cosb, sina, sinb, win2view;
    float  xp, yp, zp, persp, viewalpha, viewbeta;
    struct poly   _far *polyptr, _far *dummyptr;
    vector xscan, delta, point, dummy, xeye, yeye, zeye, up;

    if (fp)
    {
        fprintf(fp, "P5\n200 200 8\n");
        imagescan = malloc(X_PIXELS*3);
    }
    fullwin   = winsize * 2;
    halfres   = res / 2;
    numpixels = res - 1;
    up.x      = 0.0;
    up.y      = 1.0;
    up.z      = 0.0;
    zeye.x    = eye.x - aim.x;
    zeye.y    = eye.y - aim.y;
    zeye.z    = eye.z - aim.z;
    normalize(&zeye);
    xeye = crossprod(&up, &zeye);
    if (fabs(xeye.x) < ZERO && fabs(xeye.y) < ZERO && fabs(xeye.z) < ZERO)
    {
        up.y = 0.0;
        up.z = -1.0;
        xeye = crossprod(&up, &zeye);
    }
    normalize(&xeye);
    yeye = crossprod(&zeye, &xeye);
    normalize(&yeye);
    color(0, 0, 255);
    for (scanline = 0; scanline < res; scanline++)
    {
        winvert = (float)(halfres - scanline) / (float)halfres * winsize;
        xscan.x = aim.x - winsize * xeye.x + winvert * yeye.x;
        xscan.y = aim.y - winsize * xeye.y + winvert * yeye.y;
        xscan.z = aim.z - winsize * xeye.z + winvert * yeye.z;
        delta.x = fullwin / numpixels * xeye.x;
        delta.y = fullwin / numpixels * xeye.y;
        delta.z = fullwin / numpixels * xeye.z;
        for (x = 0; x < res; x++)
        {
            if (intersectraypoly(eye, xscan, &polyptr, &point, EYE))
            {
                rgb           = polyptr->color;
                polyptr->used = TRUE;
                if (!intersectraypoly(point, light, &dummyptr, &dummy, LIGHT))
                    rgb += reflect(polyptr, point);
                polyptr->used = FALSE;
                imagescan[x]  = rgb;
                if (prevrgb != rgb)
                {
                    color(rgb, rgb, rgb);
                    prevrgb = rgb;
                }
            }
            else
            {
                imagescan[x] = 0;
                if (prevrgb != -1)
                    color(0, 0, 255);
                prevrgb = -1;
            }
            pixel(x + 60, scanline);
            xscan.x += delta.x;
            xscan.y += delta.y;
            xscan.z += delta.z;
        }
        if (fp)
            fwrite(imagescan, 200, 1, fp);
    }
    if (fp)
        free(imagescan);
}

void init(char *viewfile, char *polyfile)
{
    struct poly   _far *polyptr, _far *newpoly, _far *polylist;
    struct vertex _far *verts;
    struct obj         *objptr;
    struct boundbox     space;
    FILE  *fp;
    int    i, j, onum, pnum, base;
    vector r, v, n;
    float  hue, sat, amb;
    float  tmpx, tmpy, tmpz;

    //
    // View file
    //
    puts(viewfile);
    space.xmin = space.xmax = 0.0;
    space.ymin = space.ymax = 0.0;
    space.zmin = space.zmax = 0.0;
    xcenter    = X_PIXELS / 2;
    ycenter    = Y_PIXELS / 2;
    grey       = 2;
    res        = PICT_RES;
    fp = fopen(viewfile, "r");
    fscanf(fp, "%f %f %f", &light.x, &light.y, &light.z);
    fscanf(fp, "%f %f %f", &eye.x,   &eye.y,   &eye.z);
    fscanf(fp, "%f %f %f", &aim.x,   &aim.y,   &aim.z);
    fscanf(fp, "%d", &i);
    printf("LIGHT: %f %f %f\n", light.x, light.y, light.z);
    printf("EYE:   %f %f %f\n", eye.x, eye.y, eye.z);
    printf("AIM:   %f %f %f\n", aim.x, aim.y, aim.z);
    printf("FOV:   %d\n", i);
    fclose(fp);
    fov(i);
    //
    // Object polygon file
    //
    puts(polyfile);
    objptr     = object;
    polylist   = NULL;
    pnum       =
    onum       = 0;
    fp = fopen(polyfile, "r");
    while(fscanf(fp, "%d", &objptr->numpoly) != EOF)
    {
        hue = sat = 0.0;
        fscanf(fp, "%f %f %f %f", &amb, &objptr->kdiff, &objptr->kspec, &objptr->kexp);
        base = 0;
        for (j = 0; j < objptr->numpoly; j++)
        {
            newpoly = _fmalloc(sizeof(struct poly));
            if (!newpoly)
            {
                fprintf(stderr, "Out of memory in init() newpoly\n");
                exit(-1);
            }
            if (!polylist)
                polylist = newpoly;
            else
                polyptr->next = newpoly;
            polyptr           = newpoly;
            polyptr->next     = NULL;
            polyptr->vertlist = NULL;
            polyptr->used     = FALSE;
            polyptr->polynum  = pnum++;
            polyptr->objnum   = onum;
            polyptr->color    = amb * 127.0 + base;
            fscanf(fp, "%d", &pnum);
            polyptr->numvert  = pnum;
            polyptr->vertlist =
            verts             = _fmalloc(sizeof(struct vertex) * pnum);
            if (!verts)
            {
                fprintf(stderr, "Out of memory in init() newvert\n");
                exit(-1);
            }
            for (i = 0; i < pnum; i++)
            {
                fscanf(fp, "%f %f %f", &tmpx, &tmpy, &tmpz);
                if (tmpx < space.xmin) space.xmin = tmpx;
                if (tmpx > space.xmax) space.xmax = tmpx;
                if (tmpy < space.ymin) space.ymin = tmpy;
                if (tmpy > space.ymax) space.ymax = tmpy;
                if (tmpz < space.zmin) space.zmin = tmpz;
                if (tmpz > space.zmax) space.zmax = tmpz;
                verts[i].xvert = tmpx;
                verts[i].yvert = tmpy;
                verts[i].zvert = tmpz;
            }
            putchar('.');
            v.x     = verts[0].xvert - verts[1].xvert;
            v.y     = verts[0].yvert - verts[1].yvert;
            v.z     = verts[0].zvert - verts[1].zvert;
            r.x     = verts[1].xvert - verts[2].xvert;
            r.y     = verts[1].yvert - verts[2].yvert;
            r.z     = verts[1].zvert - verts[2].zvert;
            n       = crossprod(&v, &r);
            normalize(&n);
            polyptr->normal = n;
        }
        puts("\nEND OF OBJECT DEFINITION");
        objptr++;
        onum++;
    }
    fclose(fp);
    trunk   = newbranch(1, &space); // grow trunk of octree 
    polyptr = polylist;             // insert all the polygons into the octree
    while (polyptr)
    {
        insertpoly(polyptr);
        polyptr = polyptr->next;
    }
    puts("\nDONE INSERTING POLYGONS INTO OCTREE");
}

unsigned long clock(void)
{
    union REGS regs;

    regs.x.ax = 0;
    int86(0x1A, &regs, &regs);
    return((unsigned long) regs.x.dx | ((unsigned long) regs.x.cx << 16));
}

int main(int argc, char **argv)
{
    FILE         *pbmfile;
    int           mode;
    unsigned long start, finish;

    mode = MODE_BEST;
    while (argc > 1 && argv[1][0] == '-')
    {
        if (argv[1][1] == 'd')
            switch (argv[1][2] - '0') {
                case 2:
                    mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_2BPP;
                    break;
                case 4:
                    mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_4BPP;
                    break;
                case 8:
                    mode = (mode & (MODE_MONO|MODE_NODITHER)) | MODE_8BPP;
                    break;
            }
        else if (argv[1][1] == 'm')
            mode |= MODE_MONO;
        else if (argv[1][1] == 'n')
            mode |= MODE_NODITHER;
        argc--;
        argv++;
    }
    if (argc < 4)
    {
        fprintf(stderr, "ERROR: ray <octreelevel> <viewfile> <polygonfile> [imagefile]\n");
        exit(1);
    }
    maxlevel = atoi(argv[1]);
    printf("Max octree depth = %d\n", maxlevel);
    init(argv[2], argv[3]);
#ifdef DEBUG
    puts("Press any key to start rendering...");
    getch();
#endif
    if (argc == 5)
    {
        if ((pbmfile = fopen(argv[4], "wb")) == NULL)
        {
            fprintf(stderr, "Can't open %s\n", argv[1]);
            return -1;
        }
    }
    else pbmfile = NULL;
    if (!gfxmode(mode))
    {
        fprintf(stderr, "Unable to set graphics mode.\n");
        exit(-1);
    }
    start = clock();
    image(pbmfile);
    finish = clock();
    if (pbmfile) fclose(pbmfile);
    restoremode();
    printf("Elapsed render time = %6.1f seconds.\n", (float)(finish - start) / 18.2);
    return 0;
}



