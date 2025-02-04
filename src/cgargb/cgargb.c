#include <stdio.h>
#include <math.h>

#define X_RES 640
#define Y_RES 200

#define RED 0
#define GRN 1
#define BLU 2
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
#define DUMP_STATE          2 /* Dump internal state */

unsigned char ntscChroma[4][3];
int prevRed, prevBlu, prevGrn;
unsigned char gammaRed[256]; /* RED gamma correction */
unsigned char gammaGrn[256]; /* GREEN gamma correction */
unsigned char gammaBlu[256]; /* BLUE gamma correction */
int phase[3] = RED_PHASE_NTSC, GREEN_PHASE_NTSC, BLUE_PHASE_NTSC;
unsigned char gamut[3]  = 128, 128, 128; /* Gamut */
int gamma      = 1 ;/* Gamma correction */
int brightness = 0;
int saturation = 255; /* 1.0 */
int tint       = 22;  /* = 45/2 deg */
int errDiv     = 4;
int rgbErr[X_RES + 1] /* Running color error array */
unsigned char rgbScan[X_RES * 3]; /* RGB scanline */
char flags     = 0;

long int dist(int dr, int dg, int db)
  long int rr, gg, bb;

  rr = dr * dr;
  gg = dg * dg;
  bb = db * db;
  return rr + gg + bb;
end

void calcChroma(int angle)
{
  int r, g, b, i;

  for (i = 0; i < 4; i++)
  {
    /* Calculate RGB for this DHGR pixel */
    r = saturation + (cos(angle - phase[RED]) >> 7);
    g = saturation + (cos(angle - phase[GRN]) >> 7);
    b = saturation + (cos(angle - phase[BLU]) >> 7);
    /* Make chroma add up to white */
    ntscChroma[i][RED] = (r + 2) / 4;
    ntscChroma[i][GRN] = (g + 2) / 4;
    ntscChroma[i][BLU] = (b + 2) / 4;
    /* Next NTSC chroma pixel */
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

  if (flags & DUMP_STATE)
    printf("Gamma = %d\n", gamma);
  switch (gamma)
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
        gammaRed[255 - i] = g32
        gammaGrn[255 - i] = g32
        gammaBlu[255 - i] = g32
      }
      break;
    case 2:
      for (i = 0; i < 256; i++)
      {
        g32 = (i * i + 127) / 255;
        gammaRed[i] = g32
        gammaGrn[i] = g32
        gammaBlu[i] = g32
      }
      break;
    case 1:
      for (i = 0; i < 256; i++)
      {
        g32 = (i + (i * i + 127) / 255) / 2;
        gammaRed[i] = g32
        gammaGrn[i] = g32
        gammaBlu[i] = g32
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
  if (gamut[RED] != 128)
    for (i = 0; i < 256; i++)
      gammaRed[i] = max(0, min(255, (gammaRed[i] * 128) / gamut[RED]));
  }
  if (gamut[GRN] != 128)
  {
    for (i = 0; i < 256; i++)
      gammaGrn[i] = max(0, min(255, (gammaGrn[i] * 128) / gamut[GRN]));
  }
  if (gamut[BLU] != 128)
  {
    for (i = 0; i < 256; i++)
      gammaBlu[i] = max(0, min(255, (gammaBlu[i] * 128) / gamut[BLU]));
  }
  if (brightness)
  {
    for (i = 0; i < 256; i++)
    {
      gammaRed[i] = max(0, min(255, gammaRed[i] + brightness));
      gammaGrn[i] = max(0, min(255, gammaGrn[i] + brightness));
      gammaBlu[i] = max(0, min(255, gammaBlu[i] + brightness));
    }
  }
  calcChroma(tint);
  if (flags & DUMP_STATE)
  {
    puts("Err Div = "); puti(errDiv); putln
    puts("Brightness = "); puti(brightness); putln
    puts("Tint = "); puti(tint); putln
    puts("Saturation = "); puti(saturation); putln
    puts("Gamut = ["); puti(gamut[RED]); putc(',')
    puti(gamut[GRN]); putc(','); puti(gamut[BLU]); puts("]\n")
    puts("Match = ")
    if flags & MATCH_NEXT; puts("Next\n")
    elsif flags & MATCH_CYCLE; puts("Current\n")
    else; puts("Prev\n"); fin
    puts("Chroma cycle RGB =\n")
    for i = 0 to 3
      putc('[')
      puti(ntscChroma[i*3 + RED]); putc(',')
      puti(ntscChroma[i*3 + GRN]); putc(',')
      puti(ntscChroma[i*3 + BLU]); puts("]\n")
    next
  }
  dhgrMode(DHGR_COLOR_MODE)
}

void rgbExit(void)
{
  dhgrMode(DHGR_TEXT_MODE)
}

char *pnmReadElement(int refnum, char *bufptr)
{
  char *lenptr;

  lenptr = bufptr;
  do
  {
    ^lenptr = 0
    bufptr  = lenptr + 1
    if fileio:read(refnum, bufptr, 1) == 1 and ^bufptr == '#' /* Comment */
      ^lenptr++
      bufptr++
      while fileio:read(refnum, bufptr, 1) == 1 and ^bufptr >= ' '
      loop
    else
      repeat /* Read white space seperated element */
        ^lenptr++
        bufptr++
      until fileio:read(refnum, bufptr, 1) <> 1 or ^bufptr <= ' ' or ^lenptr > 32
    fin
  } while (^lenptr and ^(lenptr + 1) <> '#'); /* Repeat until not comment */
  if (flags & DUMP_STATE)
    printf("%s ", lenptr);
  return lenptr;
}

int pnmVerifyHeader(int refnum)
{
  char buf[128];
  
  if (flags & DUMP_STATE) printf("PNM = ");
  pnmReadElement(refnum, buf);
  if (buf[0] != 2 && buf[1] != 'P' && buf[2] != '6')
  {
    printf("Invalid PNM magic #: %c%c", buf[1], buf[2]);
    return FALSE;
  }
  if (atoi(pnmReadElement(refnum, buf)) != X_RES)
  {
    printf("Width not 640: %s\n", buf);
    return FALSE;
  }
  if (atoi(pnmReadElement(refnum, buf)) != Y_RES)
  {
    printf("Height not 192: %s\n", buf);
    return FALSE;
  }
  if (atoi(pnmReadElement(refnum, buf)) != 255)
  {
    printf("Depth not 255: %s\n", buf);
    return FALSE;
  }
  if (flags & DUMP_STATE) puts("");
  return TRUE;
}

void rgbImportExport(char *rgbfile)
{
  unsigned char refnum, chromaBits;
  int i, j, r, g, b;
  int *rgbScanline, rgbptr, errptr;

  refnum = open(rgbfile);
  if (refnum)
  {
    if (!pnmVerifyHeader(refnum))
    {
      close(refnum);
      return;
    }
    rgbInit();
    rgbScanline = malloc(X_RES * 3);
    rgbErr      = malloc((X_RES+1) * 3 * sizof(int));
    if (rgbErr && rgbScanline)
    {
      /* Init error propogation array */
      memset(rgbScanline, 0, X_RES * 3);
      memset(rgbErr, 0, X_RES * 3 * sizeof(int));
      for (scanline = 0; scanline < Y_RES; scanline++)
      {
        fileio:read(refnum, rgbScanline,  640 * 3)
        // Reset prev RGB to neutral color
        prevRed = 96;
        prevGrn = 96;
        prevBLu = 96;
        /* Reset pointers */
        rgbptr  = rgbScanline
        errptr  = rgbErr
        scanptr = MK_FP(0xC000:
        for (scanbyte = 0; scanbyte < X_RES/8; scanbyte++)
        {
          chromaBits = 0;
          for (bit = 0; bit < 8, bit++)
          {
            /* Calc best match */
            r = gammaRed[rgbptr->[RED]];
            g = gammaGrn[rgbptr->[GRN]];
            b = gammaBlu[rgbptr->[BLU]];
            if rgbMatchChroma(r, g, b, errptr, bit & 3)
              chromaBits |= 0x80 >> bit;
            rgbptr = rgbptr + 3;
            errptr = errptr + 3;
          }
          scanptr[scanbyte] = chromaBits;
        }
      }
      fileio:close(refnum);
      rgbExit();
    }
  }
  else
    printf("Unable to open %s\n", rgbfile);
}

int main(int argc, char **argv)
{
  puts("CGA RGB converter 1.1");
  arg = argNext(argFirst);
  if (^arg)
  {
    while (^(arg + 1) == '-')
    {
      switch (toupper(^(arg + 2)))
      {
        case 'B': // Set brightness
          if (^arg > 2)
            ^(arg + 2) = ^arg - 2;
            brightness = atoi(arg + 2);
          }
          break;
        case 'D': // Dump internal staet
          flags = flags | DUMP_STATE;
          break;
        case 'E': // Set error strength
          if (^arg > 2)
          {
            errDiv = ^(arg + 3) - '0';
            if (^arg > 3)
              errDiv = errDiv * 10 + ^(arg + 4) - '0';
          }
          break;
        case 'G': // Set gamma amount
          if (^arg > 2)
          {
            ^(arg + 2) = ^arg - 2;
            gamma = atoi(arg + 2);
          }
          break;
        case 'P': // RGB phase angle
          switch (toupper(^(arg + 3)))
          {
            case 'I': // Use ideal 4 sub-phase angles
              phase[RED] = RED_PHASE_IDEAL;
              phase[GRN] = GREEN_PHASE_IDEAL;
              phase[BLU] = BLUE_PHASE_IDEAL;
              break;
            case 'E': // Use equal 120 deg phase angles
              phase[RED] = RED_PHASE_EQUAL;
              phase[GRN] = GREEN_PHASE_EQUAL;
              phase[BLU] = BLUE_PHASE_EQUAL;
              break;
            case 'S': // Use simplified 90 degree and opposite phase angles
              phase[RED] = RED_PHASE_SIMPLE;
              phase[GRN] = GREEN_PHASE_SIMPLE;
              phase[BLU] = BLUE_PHASE_SIMPLE;
              break;
            //is 'N' // Use theoretical NTSC phase angles
            default:
              phase[RED] = RED_PHASE_NTSC;
              phase[GRN] = GREEN_PHASE_NTSC;
              phase[BLU] = BLUE_PHASE_NTSC;
              break;
          }
          break;
        case 'S': // Adjust saturation
          if (^arg > 2)
          {
            ^(arg + 2) = ^arg - 2;
            saturation = saturation - atoi(arg + 2);
          }
          break;
        case 'T': // Adjust tint
          if (^arg > 2)
          {
            ^(arg + 2) = ^arg - 2;
            tint = tint + atoi(arg + 2);
          }
          break;
        case 'U': // Adjust gamut
          if (^arg > 3)
            switch (toupper(^(arg + 3)))
              case 'R':
                ^(arg + 1) = RED;
                break;
              case 'G':
                ^(arg + 1) = GRN;
                break;
              default: // B
                ^(arg + 1) = BLU;
                break;
            }
            ^(arg + 3) = ^arg - 3;
            gamut[^(arg + 1)] = gamut[^(arg + 1)] - atoi(arg + 3);
          }
          break;
        case 'V': // No video output, memory mode only (for portable VM)
          flags = flags | MEM_MODE;
          break;
        default:
          printf("? option: %c\n", ^(arg + 2));
      }
      arg = argNext(arg)
    }
    if (^arg)
    {
      rgbImportExport(arg, argNext(arg))
    )
    return 0;
  }
  puts("Usage:");
  puts(" DHGRRGB");
  puts("  [-B#]  = Brightness: -255..255");
  puts("  [-D]   = Dump state");
  puts("  [-E#]  = Error strength: 1..255");
  puts("                           0 = no err");
  puts("  [-G#]  = Gamma: 2, 1, 0, -1, -2");
  puts("  [-P<I,E,S,N>] = Phase: Ideal, Equal");
  puts("                         Simple, NTSC");
  puts("  [-S#]  = Saturation: -255..255");
  puts("  [-T#]  = Tint: -360..360 (in degrees)");
  puts("  [-U<R,G,B>#]  = gammUt: Red, Grn, Blu");
  puts("                          -255..255");
  puts("  [-V]   = no Video output, mem only");
  puts(" IMAGEFILE [DHGRFILE]");
  return 0; 
}
