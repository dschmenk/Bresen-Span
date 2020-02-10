#define FAST_MULDIV

void line(int x1, int y1, int x2, int y2)
{
    int dx2, dy2, err, sx, sy, ps;
    int shorterr, shortlen, longerr, longlen, halflen;
    div_t result;

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
        ps  = x1;
#ifdef FAST_MULDIV
        result  =  div(dx2 / 2, dy2); // Find first half-span length and error
        halflen = result.quot;
        err     = dy2 - result.rem;
        x1     += halflen;
#else
        err = dy2 - dx2 / 2;
        while (err < 0) // Find first half-span length and error
        {
            err += dy2;
            x1++;
        }
#endif
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
    }
    else
    {
        if (sy < 0)
        {
            ps = x1; x1 = x2; x2 = ps;
            ps = y1; y1 = y2; y2 = ps;
            sx = -sx;
        }
        if (dx2 == 0)
        {
            vspan(x1, y1, y2);
            return;
        }
        ps  = y1;
#ifdef FAST_MULDIV
        result  = div(dy2 / 2, dx2);
        halflen = result.quot;
        err     = dx2 - result.rem;
        y1     += halflen;
#else
        err = dx2 - dy2 / 2;
        while (err < 0)
        {
            err += dx2;
            y1++;
        }
#endif
        longlen = (y1 - ps + 1) * 2;
        longerr = err * 2;
        if (longerr >= dx2)
        {
            longerr -= dx2;
            longlen--;
        }
        shortlen = longlen - 1;
        shorterr = longerr - dx2;
        err     += shorterr;
        while (y1 < y2)
        {
            vspan(x1, ps, y1);
            x1 += sx;
            ps  = y1 + 1;
            if (err >= 0) // Short span
            {
                err += shorterr;
                y1  += shortlen;
            }
            else          // Long span
            {
                err += longerr;
                y1  += longlen;
            }
        }
        vspan(x2, ps, y2); // Final span
    }
}

