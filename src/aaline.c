void aaline(int x1, int y1, int x2, int y2)
{
    long inc, err;
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
            sx > 0 ? hspan8(x1, x2, y1) : hspan8(x2, x1, y1);
            return;
        }
        inc = ((long)dy << 16) / dx;
        err = inc - 0x00008000L;
        while (x1 != x2)
        {
            alpha = (err >> 8);
            if (alpha >= 0)
                alpha = 0xFF;
            else
                alpha &= 0xFF;
            aapixel(x1, y1, 0xFF^alpha);
            aapixel(x1, y1 + sy, alpha);
            if (err >= 0)
            {
                err -= 0x00010000L;
                y1  += sy;
            }
            err += inc;
            x1  += sx;
        }
        alpha = (err >> 8);
        if (alpha >= 0)
            alpha = 0xFF;
        else
            alpha &= 0xFF;
        aapixel(x2, y2, 0xFF^alpha);
        aapixel(x2, y2 + sy, alpha);
    }
    else
    {
        if (dx == 0)
        {
            sy > 0 ? vspan8(x1, y1, y2) : vspan8(x1, y2, y1);
            return;
        }
        inc = ((long)dx << 16) / dy;
        err = inc - 0x00008000L;
        while (y1 != y2)
        {
            alpha = (err >> 8);
            if (alpha >= 0)
                alpha = 0xFF;
            else
                alpha &= 0xFF;
            aapixel(x1, y1, 0xFF^alpha);
            aapixel(x1 + sx, y1, alpha);
            if (err >= 0)
            {
                err -= 0x00010000L;
                x1  += sx;
            }
            err += inc;
            y1  += sy;
        }
        if (alpha >= 0)
            alpha = 0xFF;
        else
            alpha &= 0xFF;
        aapixel(x2, y2, 0xFF^alpha);
        aapixel(x2 + sx, y2, alpha);
    }
}

