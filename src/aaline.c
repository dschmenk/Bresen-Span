void aaline(int x1, int y1, int x2, int y2)
{
    long inc, inc_minus_one, err;
    int dx, dy, sx, sy, tmp, alpha;

    sx = sy = 1;
    if ((dx = x2 - x1) < 0)
    {
        sx = -1;
        dx = -dx;
    }
    if ((dy = y2 - y1) < 0)
    {
        sy = -1;
        dy = -dy;
    }
    if (dx >= dy)
    {
        if (dy == 0)
        {
            sx > 0 ? hspan(x1, x2, y1) : hspan(x2, x1, y1);
            return;
        }
        inc           = ((long)dy << 16) / dx;
        inc_minus_one = inc - 0x00010000L;
        err           = inc - 0x00008000L;
        alpha         = err >= 0 ? 0 : 0x7F;
        while (x1 != x2)
        {
            aapixel(x1, y1, 0xFF^alpha);
            aapixel(x1, y1 + sy, alpha);
            if (err >= 0)
            {
                alpha = 0;
                err  += inc_minus_one;
                y1   += sy;
            }
            else
            {
                alpha  = ((int)err >> 8) & 0xFF;
                err   += inc;
            }
            x1 += sx;
        }
        aapixel(x2, y2, 0xFF^alpha);
        aapixel(x2, y2 + sy, alpha);
    }
    else
    {
        if (dx == 0)
        {
            sy > 0 ? vspan(x1, y1, y2) : vspan(x1, y2, y1);
            return;
        }
        inc           = ((long)dx << 16) / dy;
        inc_minus_one = inc - 0x00010000L;
        err           = inc - 0x00008000L;
        alpha         = err >= 0 ? 0 : 0x7F;
        while (y1 != y2)
        {
            aapixel(x1, y1, 0xFF^alpha);
            aapixel(x1 + sx, y1, alpha);
            if (err >= 0)
            {
                alpha = 0;
                err  += inc_minus_one;
                x1   += sx;
            }
            else
            {
                alpha  = ((int)err >> 8) & 0xFF;
                err   += inc;
            }
            y1 += sy;
        }
        aapixel(x2, y2, 0xFF^alpha);
        aapixel(x2 + sx, y2, alpha);
    }
}

