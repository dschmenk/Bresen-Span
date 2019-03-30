/*
 * Copyright 2019, David Schmenk
 */

void line(int x1, int y1, int x2, int y2)
{
    int dx2, dy2, err, sx, sy, ps;
    int shorterr, shortlen, longerr, longlen;

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
    {
        if (sy < 0)
        {
            ps = x1; x1 = x2; x2 = ps;
            ps = y1; y1 = y2; y2 = ps;
            sx = -sx;
        }
        err = dx2 - dy2 / 2;
        while (y1 < y2)
        {
            pixel(x1, y1);
            if (err >= 0)
            {
                err -= dy2;
                x1  += sx;
            }
            err += dx2;
            y1++;
        }
    }
    pixel(x2, y2);
}

