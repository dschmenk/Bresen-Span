#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <ctype.h>
#ifdef DOS
#include <dos.h>
#endif

/* CGA high resolution */
#define X_RES              640
#define Y_RES              200
/* According to what I could find out about the NTSC color wheel:
 *   Red maxes at 103.5 degrees
 *   Green maxes at 240.7 degrees
 *   Blue maxes at 347.1 degrees
 */
#define RED_PHASE_NTSC     104
#define GREEN_PHASE_NTSC   241
#define BLUE_PHASE_NTSC    347
/* Ideal phase angles for 4 phase */
#define RED_PHASE_IDEAL    90
#define GREEN_PHASE_IDEAL  270
#define BLUE_PHASE_IDEAL   360
/* Equal 120 deg phase angles */
#define RED_PHASE_EQUAL    120
#define GREEN_PHASE_EQUAL  240
#define BLUE_PHASE_EQUAL   360
/* Simplified phase angles: 90 deg between R and B, 135 between RG and BG */
#define RED_PHASE_SIMPLE   90
#define GREEN_PHASE_SIMPLE 225
#define BLUE_PHASE_SIMPLE  360
/* Flags */
#define DUMP_STATE         2 /* Dump internal state */
/* Handy macros */
#ifndef DOS
#define min(a,b)           ((a)<(b)?(a):(b))
#define max(a,b)           ((a)>(b)?(a):(b))
#endif
#define RED                0
#define GRN                1
#define BLU                2
#define DEG_TO_RAD         0.0174533
#define TRUE               1
#define FALSE              0
unsigned char ntscChroma[4][3];
int prevRed, prevBlu, prevGrn;
unsigned char gammaRed[256]; /* RED gamma correction */
unsigned char gammaGrn[256]; /* GREEN gamma correction */
unsigned char gammaBlu[256]; /* BLUE gamma correction */
int phase[3] = {RED_PHASE_NTSC, GREEN_PHASE_NTSC, BLUE_PHASE_NTSC};
int gammacorrect = 1; /* Gamma correction */
int brightness   = 0;
int saturation   = 255; /* 1.0 */
int tint         = 22;  /* = 45/2 deg */
int orgmode;
int errDiv       = 4;
unsigned char rgbScanline[X_RES * 3]; /* RGB scanline */
int rgbErr[(X_RES + 1) * 3]; /* Running color error array */
char flags     = 0;

long int dist(int dr, int dg, int db)
{
  long int rr, gg, bb;

  rr = dr * dr;
  gg = dg * dg;
  bb = db * db;
  return rr + gg + bb;
}

void calcChroma(int angle)
{
  int r, g, b, i;

  for (i = 0; i < 4; i++)
  {
    /* Calculate RGB for this NTSC subcycle pixel */
    r = saturation + (int)(cos((angle - phase[RED]) * DEG_TO_RAD) * 255);
    g = saturation + (int)(cos((angle - phase[GRN]) * DEG_TO_RAD) * 255);
    b = saturation + (int)(cos((angle - phase[BLU]) * DEG_TO_RAD) * 255);
    /* Make chroma add up to white */
    ntscChroma[i][RED] = (r + 2) / 4;
    ntscChroma[i][GRN] = (g + 2) / 4;
    ntscChroma[i][BLU] = (b + 2) / 4;
    /* Next NTSC chroma subcycle pixel */
    angle = angle - 90;
  }
}

int rgbMatchChroma(int r, int g, int b, int *errptr, int cx)
{
  int currRed, currGrn, currBlu;
  int errRed, errGrn, errBlu;
  int pix;

  /* Apply error propogation */
  if (errDiv)
  {
    r = r + errptr[RED] / errDiv;
    g = g + errptr[GRN] / errDiv;
    b = b + errptr[BLU] / errDiv;
  }
  /* Previous RGB chroma subcycles */
  prevRed = (prevRed * 3) / 4;
  prevGrn = (prevGrn * 3) / 4;
  prevBlu = (prevBlu * 3) / 4;
  /* Current potential RGB subcycle */
  currRed = prevRed + ntscChroma[cx][RED];
  currGrn = prevGrn + ntscChroma[cx][GRN];
  currBlu = prevBlu + ntscChroma[cx][BLU];
  /* Match chroma subcycle */
  pix = 0;
  if (dist(r - currRed, g - currGrn, b - currBlu) < dist(r - prevRed, g - prevGrn, b - prevBlu))
  {
    /* RGB better matched with current chroma subcycle color */
    prevRed = currRed;
    prevGrn = currGrn;
    prevBlu = currBlu;
    pix = 1;
  }
  /* Propogate error down */
  errRed = r - prevRed;
  errGrn = g - prevGrn;
  errBlu = b - prevBlu;
  errptr[RED] = errRed;
  errptr[GRN] = errGrn;
  errptr[BLU] = errBlu;
  /* And forward */
  errptr[RED + 3] = errRed + errptr[RED + 3];
  errptr[GRN + 3] = errGrn + errptr[GRN + 3];
  errptr[BLU + 3] = errBlu + errptr[BLU + 3];
  return pix;
}

void rgbInit(void)
{
  int i;
  long int g32;
#ifdef DOS
  union REGS regs;
#endif

  switch (gammacorrect)
  {
    case -1:
      for (i = 0; i < 256; i++)
      {
        g32 = (255 - i + 255 - (i * i + 127)/255) / 2;
        gammaRed[255 - i] = g32;
        gammaGrn[255 - i] = g32;
        gammaBlu[255 - i] = g32;
      }
      break;
    case -2:
      for (i = 0; i < 256; i++)
      {
        g32 = 255 - (i * i + 127)/255;
        gammaRed[255 - i] = g32;
        gammaGrn[255 - i] = g32;
        gammaBlu[255 - i] = g32;
      }
      break;
    case 2:
      for (i = 0; i < 256; i++)
      {
        g32 = (i * i + 127) / 255;
        gammaRed[i] = g32;
        gammaGrn[i] = g32;
        gammaBlu[i] = g32;
      }
      break;
    case 1:
      for (i = 0; i < 256; i++)
      {
        g32 = (i + (i * i + 127) / 255) / 2;
        gammaRed[i] = g32;
        gammaGrn[i] = g32;
        gammaBlu[i] = g32;
      }
      break;
    default:
      for (i = 0; i < 256; i++)
      {
        gammaRed[i] = i;
        gammaGrn[i] = i;
        gammaBlu[i] = i;
      }
  }
  if (brightness)
    for (i = 0; i < 256; i++)
    {
      gammaRed[i] = max(0, min(255, gammaRed[i] + brightness));
      gammaGrn[i] = max(0, min(255, gammaGrn[i] + brightness));
      gammaBlu[i] = max(0, min(255, gammaBlu[i] + brightness));
    }
  calcChroma(tint);
  if (flags & DUMP_STATE)
  {
    printf("Gamma = %d\n", gammacorrect);
    printf("Err Div = %d\n", errDiv);
    printf("Brightness = %d\n", brightness);
    printf("Tint = %d\n", tint);
    printf("Saturation = %d\n", saturation);
    puts("Chroma cycle RGB =");
    for (i = 0; i < 4; i++)
      printf("    [%3d, %3d %3d]\n", ntscChroma[i][RED],
                                     ntscChroma[i][GRN],
                                     ntscChroma[i][BLU]);
    getchar();
  }
#ifdef DOS
  /* Get current mode */
  regs.x.ax = 0x0F00;
  int86(0x10, &regs, &regs);
  orgmode = regs.h.al;
  /* Set mode 0x06 640x200 */
  regs.x.ax = 0x0006;
  int86(0x10, &regs, &regs);
  /* Enable colorburst */
#endif
}

void rgbExit(void)
{
#ifdef DOS
  union REGS regs;

  regs.x.ax = orgmode;
  int86(0x10, &regs, &regs);
#endif
}

char *pnmReadElement(FILE *fp, char *bufptr)
{
  char *endptr;

  endptr = bufptr + 1;
  while (fread(bufptr, 1, 1, fp) == 1 && *bufptr == '#') /* Comment */
    while (fread(endptr, 1, 1, fp) == 1 && *endptr >= ' ');
  while (fread(endptr, 1, 1, fp) == 1 && *endptr > ' ' && (endptr - bufptr < 80))
    endptr++;
  *endptr = '\0';
  if (flags & DUMP_STATE)
    puts(bufptr);
  return bufptr;
}

int pnmVerifyHeader(FILE *fp)
{
  char buf[128];

  if (flags & DUMP_STATE)
    printf("PNM = ");
  pnmReadElement(fp, buf);
  if (strcmp(buf, "P6"))
  {
    printf("Invalid PNM magic #: %c%c\n", buf[0], buf[1]);
    return FALSE;
  }
  if (atoi(pnmReadElement(fp, buf)) != X_RES)
  {
    printf("Width not 640: %s\n", buf);
    return FALSE;
  }
  if (atoi(pnmReadElement(fp, buf)) != Y_RES)
  {
    printf("Height not 192: %s\n", buf);
    return FALSE;
  }
  if (atoi(pnmReadElement(fp, buf)) != 255)
  {
    printf("Depth not 255: %s\n", buf);
    return FALSE;
  }
  return TRUE;
}

int rgbImportExport(char *pnmfile)
{
  FILE *fp;
#ifdef DOS
  unsigned char far *scanptr;
#else
  unsigned char *scanptr;
#endif
  unsigned char chromaBits, *rgbptr;
  int bit, scanbyte, scan, r, g, b, *errptr;

  if (flags & DUMP_STATE)
    printf("PNM file = %s\n", pnmfile);
  fp = fopen(pnmfile, "rb");
  if (fp)
  {
    if (pnmVerifyHeader(fp))
    {
      rgbInit();
#ifndef DOS
      scanptr = malloc(X_RES / 8);
#endif
      /* Init error propogation array */
      memset(rgbScanline, 0, X_RES * 3);
      memset(rgbErr, 0, X_RES * 3 * sizeof(int));
      for (scan = 0; scan < Y_RES; scan++)
      {
        fread(rgbScanline, X_RES, 3, fp);
        /* Reset prev RGB to neutral color */
        prevRed = 96;
        prevGrn = 96;
        prevBlu = 96;
        /* Reset pointers */
        rgbptr  = rgbScanline;
        errptr  = rgbErr;
#ifdef DOS
        scanptr = (unsigned char far *)(0xB8000000L + (scan >> 1) * 80 + (scan & 1) * 8192);
#endif
        for (scanbyte = 0; scanbyte < X_RES/8; scanbyte++)
        {
          chromaBits = 0;
          for (bit = 0; bit < 8; bit++)
          {
            /* Calc best match */
            r = gammaRed[rgbptr[RED]];
            g = gammaGrn[rgbptr[GRN]];
            b = gammaBlu[rgbptr[BLU]];
            if (rgbMatchChroma(r, g, b, errptr, bit & 3))
              chromaBits |= 0x80 >> bit;
            rgbptr = rgbptr + 3;
            errptr = errptr + 3;
          }
          scanptr[scanbyte] = chromaBits;
        }
        rgbExit();
      }
    }
    fclose(fp);
    return 0;
  }
  printf("Unable to open %s\n", pnmfile);
  return -1;
}

int main(int argc, char **argv)
{
  puts("CGA RGB converter 1.0");
  while (*++argv && (*argv)[0] == '-')
  {
    switch (toupper((*argv)[1]))
    {
      case 'B': /* Set brightness */
        brightness = atoi(*argv + 2);
        break;
      case 'D': /* Dump internal state */
        flags = flags | DUMP_STATE;
        break;
      case 'E': /* Set error strength */
        errDiv = atoi(*argv + 2);
        break;
      case 'G': /* Set gamma amount */
        gammacorrect = atoi(*argv + 2);
        break;
      case 'P': /* RGB phase angle */
        switch (toupper((*argv)[2]))
        {
          case 'I': /* Use ideal 4 sub-phase angles */
            phase[RED] = RED_PHASE_IDEAL;
            phase[GRN] = GREEN_PHASE_IDEAL;
            phase[BLU] = BLUE_PHASE_IDEAL;
            break;
          case 'E': /* Use equal 120 deg phase angles */
            phase[RED] = RED_PHASE_EQUAL;
            phase[GRN] = GREEN_PHASE_EQUAL;
            phase[BLU] = BLUE_PHASE_EQUAL;
            break;
          case 'S': /* Use simplified 90 degree and opposite phase angles */
            phase[RED] = RED_PHASE_SIMPLE;
            phase[GRN] = GREEN_PHASE_SIMPLE;
            phase[BLU] = BLUE_PHASE_SIMPLE;
            break;
          /* case 'N': Use theoretical NTSC phase angles */
          default:
            phase[RED] = RED_PHASE_NTSC;
            phase[GRN] = GREEN_PHASE_NTSC;
            phase[BLU] = BLUE_PHASE_NTSC;
            break;
        }
        break;
      case 'S': /* Adjust saturation */
        saturation = saturation - atoi(*argv + 2);
        break;
      case 'T': /* Adjust tint */
        tint = tint + atoi(*argv + 2);
        break;
      default:
        printf("? option: %c\n", (*argv)[1]);
    }
  }
  if (*argv)
    return rgbImportExport(*argv);
  puts("Usage:");
  puts(" CGARGB");
  puts("  [-B#]  = Brightness: -255..255");
  puts("  [-D]   = Dump state");
  puts("  [-E#]  = Error strength: 1..255");
  puts("                           0 = no err");
  puts("  [-G#]  = Gamma: 2, 1, 0, -1, -2");
  puts("  [-P<I,E,S,N>] = Phase: Ideal, Equal, Simple, NTSC");
  puts("  [-S#]  = Saturation: -255..255");
  puts("  [-T#]  = Tint: -360..360 (in degrees)");
  return 0;
}
