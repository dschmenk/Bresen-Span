#include <stdio.h>
#include <math.h>

#define X_RES 640
#define Y_RES 200

#define RED 0
#define GRN 1
#define BLU 2
// According to what I could find out about the NTSC color wheel:
//   Red maxes at 103.5 degrees
//   Green maxes at 240.7 degrees
//   Blue maxes at 347.1 degrees
#define RED_PHASE_NTSC     104
#define GREEN_PHASE_NTSC   241
#define BLUE_PHASE_NTSC    347
// Ideal phase angles for 4 phase
#define RED_PHASE_IDEAL    90
#define GREEN_PHASE_IDEAL  270
#define BLUE_PHASE_IDEAL   360
// Equal 120 deg phase angles
#define RED_PHASE_EQUAL    120
#define GREEN_PHASE_EQUAL  240
#define BLUE_PHASE_EQUAL   360
// Simplified phase angles: 90 deg between R and B, 135 between RG and BG
#define RED_PHASE_SIMPLE   90
#define GREEN_PHASE_SIMPLE 225
#define BLUE_PHASE_SIMPLE  360
#define GREY_CHROMA        (32 * 4 / 3)
// Flags
#define MEM_MODE    $01 // Render to memory surface
#define DUMP_STATE  $02 // Dump internal state

unsigned char ntscChroma[4][3];
int prevRed, prevBlu, prevGrn;
unsigned char gammaRed[256]; // RED gamma correction
unsigned char gammaGrn[256]; // GREEN gamma correction
unsigned char gammaBlu[256]; // BLUE gamma correction
int phase[3] = RED_PHASE_NTSC, GREEN_PHASE_NTSC, BLUE_PHASE_NTSC;
unsigned char gamut[3]  = 128, 128, 128; // Gamut
int gamma      = 1 ;// Gamma correction
int brightness = 0;
int saturation = 255; // 1.0
int tint       = 22;  // = 45/2 deg
int errDiv     = 4;
char flags     = 0;
int rgbErr[X_RES + 1] // Running color error array
var arg

long int dist(int dr, int dg, int db)
  long int rr, gg, bb;

  rr = dr * dr;
  gg = dg * dg;
  bb = db * db;
  return rr + gg + bb;
end

def calcChroma(angle)#0
  int r, g, b, i;

  for i = 0 to 3
    // Calculate RGB for this DHGR pixel
    r = saturation + (cos(angle - phase[RED]) >> 7)
    g = saturation + (cos(angle - phase[GRN]) >> 7)
    b = saturation + (cos(angle - phase[BLU]) >> 7)
    // Make chroma add up to white
    ntscChroma[i][RED] = (r + 2) / 4
    ntscChroma[i][GRN] = (g + 2) / 4
    ntscChroma[i][BLU] = (b + 2) / 4
    // Next NTSC chroma pixel
    angle = angle - 90
  next
end

def rgbMatch(r, g, b, errptr, cx)#1
  var cr, cg, cb
  var er, eg, eb
  byte i,
  res[t_i32] pd

  // Previous RGB minus current chroma cycle
  prevRed = (prevRed * 3) / 4
  prevGrn = (prevGrn * 3) / 4
  prevBlu = (prevBlu * 3) / 4
  // Current potential RGB
  i = cx * 3
  cr = prevRed + ntscChroma[i+RED]
  cg = prevGrn + ntscChroma[i+GRN]
  cb = prevBlu + ntscChroma[i+BLU]
  // Match next chroma subcycle
  pd:[0], pd:[1] = dist(r - prevRed, g - prevGrn, b - prevBlu)
  dist(r - cr, g - cg, b - cb)
  if islt32(@pd)
    // RGB better matched with next chroma color
    prevRed = cr
    prevGrn = cg
    prevBlu = cb
    i = 1
  else
    i = 0
  fin
  // Propogate error down and forward
  er          = r - prevRed
  errptr[RED] = er
  eg          = g - prevGrn
  errptr[GRN] = eg
  eb          = b - prevBlu
  errptr[BLU] = eb
  errptr += 3
  errptr[RED] = er + errptr[RED]
  errptr[GRN] = eg + errptr[GRN]
  errptr[BLU] = eb + errptr[BLU]
  return i
end

def rgbInit#0
  var i
  res[t_i32] g32

  if flags & DUMP_STATE
    puts("Gamma = "); puti(sext(gamma)); putln
  fin
  when gamma
    is 255 // (i + 1 / i^2) / 2
      for i = 0 to 255
        loadi16(i)
        muli16(i)
        addi16(127)
        divi16(255)
        neg32
        addi16(255)
        addi16(255 - i)
        divi16(2)
        store32(@g32)
        gammaRed[255 - i] = g32
        gammaGrn[255 - i] = g32
        gammaBlu[255 - i] = g32
      next
      break
    is 254 // 1 - i^2
      for i = 0 to 255
        loadi16(i)
        muli16(i)
        addi16(127)
        divi16(255)
        neg32
        addi16(255)
        store32(@g32)
        gammaRed[255 - i] = g32
        gammaGrn[255 - i] = g32
        gammaBlu[255 - i] = g32
      next
      break
    is 2 // 1/(i^2)
      for i = 0 to 255
        loadi16(i)
        muli16(i)
        addi16(127)
        divi16(255)
        store32(@g32)
        gammaRed[i] = g32
        gammaGrn[i] = g32
        gammaBlu[i] = g32
      next
      break
    is 1 // (i + i^2) / 2
      for i = 0 to 255
        loadi16(i)
        muli16(i)
        addi16(127)
        divi16(255)
        addi16(i)
        divi16(2)
        store32(@g32)
        gammaRed[i] = g32
        gammaGrn[i] = g32
        gammaBlu[i] = g32
      next
      break
    otherwise // i
      for i = 0 to 255
        gammaRed[i] = i
        gammaGrn[i] = i
        gammaBlu[i] = i
      next
  wend
  if gamut[RED] <> 128
    for i = 0 to 255
      gammaRed[i] = max(0, min(255, (gammaRed[i] * 128) / gamut[RED]))
    next
  fin
  if gamut[GRN] <> 128
    for i = 0 to 255
      gammaGrn[i] = max(0, min(255, (gammaGrn[i] * 128) / gamut[GRN]))
    next
  fin
  if gamut[BLU] <> 128
    for i = 0 to 255
      gammaBlu[i] = max(0, min(255, (gammaBlu[i] * 128) / gamut[BLU]))
    next
  fin
  if brightness
    for i = 0 to 255
      gammaRed[i] = max(0, min(255, gammaRed[i] + brightness))
      gammaGrn[i] = max(0, min(255, gammaGrn[i] + brightness))
      gammaBlu[i] = max(0, min(255, gammaBlu[i] + brightness))
    next
  fin
  calcChroma(tint)
  if flags & DUMP_STATE
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
  fin
  dhgrMode(DHGR_COLOR_MODE)
end

def rgbExit#0
  dhgrMode(DHGR_TEXT_MODE)
end

def pnmReadElement(refnum, bufptr)#1
  var lenptr

  lenptr = bufptr
  repeat
    ^lenptr = 0
    bufptr  = lenptr + 1
    if fileio:read(refnum, bufptr, 1) == 1 and ^bufptr == '#' // Comment
      ^lenptr++
      bufptr++
      while fileio:read(refnum, bufptr, 1) == 1 and ^bufptr >= ' '
      loop
    else
      repeat // Read white space seperated element
        ^lenptr++
        bufptr++
      until fileio:read(refnum, bufptr, 1) <> 1 or ^bufptr <= ' ' or ^lenptr > 32
    fin
  until ^lenptr and ^(lenptr + 1) <> '#' // Repeat until not comment
  if flags & DUMP_STATE; puts(lenptr); putc(' '); fin
  return lenptr
end

def pnmVerifyHeader(refnum)#1
  byte[128] buf
  if flags & DUMP_STATE; puts("PNM = "); fin
  pnmReadElement(refnum, @buf)
  if buf[0] <> 2 and buf[1] <> 'P' and buf[2] <> '6'
    puts("Invalid PNM magic #: "); putc(buf[1]); putc(buf[2]); putln
    return FALSE
  fin
  if atoi(pnmReadElement(refnum, @buf)) <> 560
    puts("Width not 560: "); puts(@buf); putln
    return FALSE
  fin
  if atoi(pnmReadElement(refnum, @buf)) <> 192
    puts("Height not 192: "); puts(@buf); putln
    return FALSE
  fin
  if atoi(pnmReadElement(refnum, @buf)) <> 255
    puts("Depth not 255: "); puts(@buf); putln
    return FALSE
  fin
  if flags & DUMP_STATE; putln; fin
  return TRUE
end

def rgbImportExport(rgbfile, dhgrfile)#0
  byte refnum, chromaBits
  var i, j, r, g, b
  var rgbScanline, rgbptr, errptr

  refnum = fileio:open(rgbfile)
  if refnum
    if not (flags & RAW_INFILE)
      if not pnmVerifyHeader(refnum)
        fileio:close(refnum)
        return
      fin
    fin
    rgbInit
    rgbScanline = heapalloc(560 * 3)
    rgbErr      = heapalloc(561 * 3 * 2)
    if rgbErr and rgbScanline
      // Init error propogation array
      memset(rgbErr, 0, 560 * 3 * 2)
      memset(rgbScanline, 0, 560 * 3)
      for j = 0 to 191
        fileio:read(refnum, rgbScanline,  560 * 3)
        memset(@ntscCycle, GREY_CHROMA, 24) // Reset chroma cycle
        prevRed, prevGrn, prevBLu = 96, 96, 96 // Reset prev RGB
        rgbptr = rgbScanline
        errptr = rgbErr
        for i = 0 to 559
          // Calc best match
          chromaBits = chromaBits >> 1
          r = gammaRed[rgbptr->[RED]]
          g = gammaGrn[rgbptr->[GRN]]
          b = gammaBlu[rgbptr->[BLU]]
          if errDiv
            r = r + errptr=>[RED] / errDiv
            g = g + errptr=>[GRN] / errDiv
            b = b + errptr=>[BLU] / errDiv
          fin
          if rgbMatch(r, g, b, errptr, i & 3)
            dhgrSet(i, j)
            chromaBits = chromaBits | $08
          fin
          // Map GREY1 -> GREY2
          if (i & 3) == 3
            if chromaBits == $0A // Bits are in reverse order from DCGR color value
              dhgrOp(OP_SRC)
              dcgrColor(CLR_GREY2)
              dcgrPixel(i >> 2, j)
              memset(@ntscCycle, GREY_CHROMA, 24) // Grey chroma cycle
            elsif chromaBits == $05
              memset(@ntscCycle, GREY_CHROMA, 24) // Grey chroma cycle
            fin
          fin
          rgbptr = rgbptr + 3
          errptr = errptr + 3 * 2
        next
        if flags & MEM_MODE; putc('.'); fin
        if ^$C000 == $83
          break
        fin
      next
      fileio:close(refnum)
      if not (flags & MEM_MODE); getc; fin
      rgbExit
    fin
  else
    puts("Unable to open "); puts(rgbfile); putln
  fin
end

int main(int argc, char **argv)
{
  puts("CGA RGB converter 1.1\n")
  arg = argNext(argFirst)
  if ^arg
    while ^(arg + 1) == '-'
      when toupper(^(arg + 2))
        is 'B' // Set brightness
          if ^arg > 2
            ^(arg + 2) = ^arg - 2
            brightness = atoi(arg + 2)
          fin
          break
        is 'D' // Dump internal staet
          flags = flags | DUMP_STATE
          break
        is 'E' // Set error strength
          if ^arg > 2
            errDiv = ^(arg + 3) - '0'
            if ^arg > 3
              errDiv = errDiv * 10 + ^(arg + 4) - '0'
            fin
          fin
          break
        is 'G' // Set gamma amount
          if ^arg > 2
            ^(arg + 2) = ^arg - 2
            gamma = atoi(arg + 2)
          fin
          break
        is 'P' // RGB phase angle
          when toupper(^(arg + 3))
            is 'I' // Use ideal 4 sub-phase angles
              phase[RED] = RED_PHASE_IDEAL
              phase[GRN] = GREEN_PHASE_IDEAL
              phase[BLU] = BLUE_PHASE_IDEAL
              break
            is 'E' // Use equal 120 deg phase angles
              phase[RED] = RED_PHASE_EQUAL
              phase[GRN] = GREEN_PHASE_EQUAL
              phase[BLU] = BLUE_PHASE_EQUAL
              break
            is 'S' // Use simplified 90 degree and opposite phase angles
              phase[RED] = RED_PHASE_SIMPLE
              phase[GRN] = GREEN_PHASE_SIMPLE
              phase[BLU] = BLUE_PHASE_SIMPLE
              break
            //is 'N' // Use theoretical NTSC phase angles
            otherwise
              phase[RED] = RED_PHASE_NTSC
              phase[GRN] = GREEN_PHASE_NTSC
              phase[BLU] = BLUE_PHASE_NTSC
              break
          wend
          break
        is 'S' // Adjust saturation
          if ^arg > 2
            ^(arg + 2) = ^arg - 2
            saturation = saturation - atoi(arg + 2)
          fin
          break
        is 'T' // Adjust tint
          if ^arg > 2
            ^(arg + 2) = ^arg - 2
            tint = tint + atoi(arg + 2)
          fin
          break
        is 'U' // Adjust gamut
          if ^arg > 3
            when toupper(^(arg + 3))
              is 'R'
                ^(arg + 1) = RED
                break
              is 'G'
                ^(arg + 1) = GRN
                break
              otherwise // B
                ^(arg + 1) = BLU
                break
            wend
            ^(arg + 3) = ^arg - 3
            gamut[^(arg + 1)] = gamut[^(arg + 1)] - atoi(arg + 3)
          fin
          break
        is 'V' // No video output, memory mode only (for portable VM)
          flags = flags | MEM_MODE
          break
        otherwise
          puts("? option:"); putc(^(arg + 2)); putln
      wend
      arg = argNext(arg)
    loop
    if ^arg
      rgbImportExport(arg, argNext(arg))
    fin
    return 0
  fin
  puts("Usage:\n")
  puts(" DHGRRGB\n")
  puts("  [-B#]  = Brightness: -255..255\n")
  puts("  [-D]   = Dump state\n")
  puts("  [-E#]  = Error strength: 1..255\n")
  puts("                           0 = no err\n")
  puts("  [-G#]  = Gamma: 2, 1, 0, -1, -2\n")
  puts("  [-P<I,E,S,N>] = Phase: Ideal, Equal\n")
  puts("                         Simple, NTSC\n")
  puts("  [-S#]  = Saturation: -255..255\n")
  puts("  [-T#]  = Tint: -360..360 (in degrees)\n")
  puts("  [-U<R,G,B>#]  = gammUt: Red, Grn, Blu\n")
  puts("                          -255..255\n")
  puts("  [-V]   = no Video output, mem only\n")
  puts(" IMAGEFILE [DHGRFILE]\n")
  return 0; 
}
