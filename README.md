# Bresen-Span: A Span based Bresenham line routine

There are many computer graphics papers written about improving upon the standard Bresenham's line drawing algorithm to speed it up using horizontal and vertical spans of pixels, instead of the one-pixel-at-a-time approach. Often, the resulting algorithm seems more complicated than it needs to be. So I went back and wrote my own making as few changes to the original to keep it simple.

## Pixel-At-A-Time Bresenham
To start with, here is the simple, clean, and elegant algorithm as implemeted by Bresenham:

```
void line(int x1, int y1, int x2, int y2)
{
    int dx2, dy2, err, sx, sy, ps;

    sx = sy = 1;
    if ((dx2 = (x2 - x1) * 2) < 0)
    {
        sx  = -1;
        dx2 = -dx2;
    }
    if ((dy2 = (y2 - y1) * 2) < 0)
    {
        sy  = -1;
        dy2 = -dy2;
    }
    if (dx2 >= dy2)
    {
        if (sx < 0)
        {
            ps = x1; x1 = x2; x2 = ps;
            ps = y1; y1 = y2; y2 = ps;
            sy = -sy;
        }
        err = dy2 - dx2 / 2;
        while (x1 < x2)
        {
            pixel(x1, y1);
            if (err >= 0)
            {
                err -= dx2;
                y1  += sy;
            }
            err += dy2;
            x1++;
        }
    }
    else
      ... // Y major axis
    pixel(x2, y2);
}

```
I'm only going to show the X major axis code here, the Y axis is just reflected and present in the source file.

I will assume there are no questions about the baseline Bresenham line algorithm. However, take note that the algorithm can be viewed as the long division of delta-major/delta-minor. The error term is really the running remainder, and every step results in a pixel along the major axis until the division completes with a remainder. The division restarts by moving along the minor axis and adding the dividend back in to the running remainder (error term). This is a bit of a simplification, but the concept is that the long division will only result in two integral spans of pixels, depending on the value of the running remainder (error term). We will take this in to account to write a routine that outputs spans based on the two span lengths: a short-span and a long-span.

## Span-At-A-Time Bresenham

We will start off looking very much like the standard algorithm:
```
void fast_line(int x1, int y1, int x2, int y2)
{
    int dx2, dy2, err, sx, sy, ps;
    int shorterr, shortlen, longerr, longlen, halflen;

    sx = sy = 1;
    if ((dx2 = (x2 - x1) * 2) < 0)
    {
        sx  = -1;
        dx2 = -dx2;
    }
    if ((dy2 = (y2 - y1) * 2) < 0)
    {
        sy  = -1;
        dy2 = -dy2;
    }
    if (dx2 >= dy2)
    {
        if (sx < 0)
        {
            ps = x1; x1 = x2; x2 = ps;
            ps = y1; y1 = y2; y2 = ps;
            sy = -sy;
        }
        if (dy2 == 0)
        {
            hspan(x1, x2, y1);
            return;
        }
```
We have defined a few more variables that will be needed and there is an explicit check for a horizontal line. This avoid a division by zero in the next step:
```
        ps  = x1;
#ifdef FAST_MULDIV
        halflen = (dx2 / 2) / dy2; // Find first half-span length and error
        err     = (halflen + 1) * dy2 - dx2 / 2;
        x1     += halflen;
#else
        err = dy2 - dx2 / 2;
        while (err < 0) // Find first half-span length and error
        {
            err += dy2;
            x1++;
        }
#endif
```
A little explanation is in order. The Bresenham algorithm is quite clever in that it is symetrical. Remember when I said there were only two lengths of spans? I lied. The first and last spans are half length. The initial error term for the pixel-at-a-time Bresenham algorithm looks like this: `err = dy2 - dx2 / 2;` which initializes the error term to 0.5, thus starting half-way through the first span. In order to replicate this, the span-at-a-time algorithm calculates this half-span length with either division & multiplication if you have fast hardware for those operations, or a long division version if you don't. Once the half-span length has been calculated, time to figure out the long and short span values based on the half-span:
```
        longlen = (x1 - ps + 1) * 2; // Long-span length = half-span length * 2
        longerr = err * 2;
        if (longerr >= dy2)
        {
            longerr -= dy2;
            longlen--;
        }
        shortlen = longlen - 1; // Short-span length = long-span length - 1
        shorterr = longerr - dy2;
        err     += shorterr; // Do a short-span step
```
Yes, the span-at-a-time algorithm requires a bit more initial setup. This code was written for clarity, not flat out speed. If you were going to rewrite the algorithm, there are many tricks to be used to make it more efficient. An assembly language version could really improve upon the speed of this routine.

But now the initialization has been completed. Look at what happens next in the main loop:
```
        while (x1 < x2)
        {
            hspan(ps, x1, y1);
            y1 += sy;     // Move to next span
            ps  = x1 + 1; // Start of next span = end of previous span + 1
            if (err >= 0) // Short span
            {
                err += shorterr;
                x1  += shortlen;
            }
            else          // Long span
            {
                err += longerr;
                x1  += longlen;
            }
        }
        hspan(ps, x2, y2); // Final span
```
Bam! That's it.

You will find the complete routines and some sample routines in the form of an 8BPP framebuffer library in the source directory. Real-mode DOS binaries (VGA required) can be run inside DOSBox to see it in action with some timing. Depending on what hardware/emulation you use, the differences between the normal and fast lines may not look like much. A few reasons for this discrepency are that the span drawing routines are not incredibly optimized and are actually slower than the single pixel drawing for slopes near 1.0 due to setup overhead. Try replacing the drawing routines with empty, dummy routines to compare the actual speed of the line algorithms.

The LINETEST.EXE and FILLTRI.EXE programs take an argument to disable the dithering '-d0' and the dithered brush routines will be replaced with the solid color. The solid color routines are much faster than the dithered color code. The FILLTRI.EXE program also takes a '-f0' argument to disable the triangle fill code and use the default span routines. Check out how the same line and span routines can be repurposed to implement a polygon fill routine.
