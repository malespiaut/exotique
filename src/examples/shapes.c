#include "exotique.h"

/* Screen size */

const i32 kScreenWidth = 320;
const i32 kScreenHeight = 200;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* C library reimplementation */

static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
  }
}

/* 32-bits PRNG - xoshiro128++ */

static /*inline*/ u32
rotl(const u32 x, i32 k)
{
  return (x << k) | (x >> (32 - k));
}

/* Completely arbitrary seeds */
static u32 s[4] = {0x27cb588d, 0x096379a9, 0xe81f5914, 0x2ee1c98c};

static u32
next(void)
{
  const u32 result = rotl(s[0] + s[3], 7) + s[0];

  const u32 t = s[1] << 9;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;

  s[3] = rotl(s[3], 11);

  return result;
}

static /*inline*/ i32
abs(i32 x)
{
  return x < 0 ? -x : x;
}

/* Shape functions */

/* Helper macro to ensure that pixel is within screen bounds */
#define in_bound(x, y) ((x) >= 0 && (x) < kScreenWidth && (y) >= 0 && (y) < kScreenHeight)

/* Function to draw a pixel on the screen if it is within bounds */
static void
pixel_draw(u8* screen, i32 x, i32 y, u8 color)
{
  if (in_bound(x, y))
  {
    screen[y * kScreenWidth + x] = color;
  }
}

/* Function to draw a circle using Bresenham's algorithm */
static void
circle_line_draw(u8* screen, i32 x_center, i32 y_center, i32 radius, u8 color)
{
  i32 x = 0;          /* x-coordinate offset from the center */
  i32 y = radius;     /* y-coordinate starts at the radius */
  i32 d = 1 - radius; /* Decision variable used to determine the next pixel to plot */

  /* Plot the initial points at 8 symmetric positions */
  pixel_draw(screen, x_center + x, y_center + y, color);
  pixel_draw(screen, x_center + x, y_center - y, color);
  pixel_draw(screen, x_center + y, y_center + x, color);
  pixel_draw(screen, x_center - y, y_center + x, color);
  pixel_draw(screen, x_center - x, y_center - y, color);
  pixel_draw(screen, x_center - x, y_center + y, color);
  pixel_draw(screen, x_center - y, y_center - x, color);
  pixel_draw(screen, x_center + y, y_center - x, color);

  /* Iterate through the circle and calculate points using integer operations */
  while (x < y)
  {
    ++x;

    if (d < 0)
    {
      /* Select the pixel straight across (midpoint is inside the circle) */
      d += 2 * x + 1;
    }
    else
    {
      /* Select the diagonal pixel (midpoint is outside the circle) */
      --y;
      d += 2 * (x - y) + 1;
    }

    /* Plot the 8 symmetrical points for each calculated (x, y) pair */
    pixel_draw(screen, x_center + x, y_center + y, color);
    pixel_draw(screen, x_center - x, y_center + y, color);
    pixel_draw(screen, x_center + x, y_center - y, color);
    pixel_draw(screen, x_center - x, y_center - y, color);

    pixel_draw(screen, x_center + y, y_center + x, color);
    pixel_draw(screen, x_center - y, y_center + x, color);
    pixel_draw(screen, x_center + y, y_center - x, color);
    pixel_draw(screen, x_center - y, y_center - x, color);
  }
}

/* Draw a horizontal line on the screen from x1 to x2 at y */
void
hline_draw(u8* screen, i32 x1, i32 x2, i32 y, u8 color)
{
  i32 i = 0;
  i32 width = 0;
  /* Ensure y is within screen height bounds */
  if (y < 0 || y >= kScreenHeight)
  {
    return; /* Do nothing if y is out of bounds */
  }

  /* Ensure x1 is less than or equal to x2 */
  if (x1 > x2)
  {
    i32 temp = x1;
    x1 = x2;
    x2 = temp; /* Swap x1 and x2 if necessary */
  }

  /* Clamp x1 and x2 within screen width bounds */
  if (x1 < 0)
  {
    x1 = 0;
  }
  if (x2 >= kScreenWidth)
  {
    x2 = kScreenWidth - 1;
  }

  /* Calculate the width of the line */
  width = x2 - x1 + 1;

  /* Draw the horizontal line if width is valid */
  if (width > 0)
  {
    u8* pixel = &screen[y * kScreenWidth + x1]; /* Get starting pixel address */
    for (; i < width; ++i)
    {
      pixel[i] = color; /* Set each pixel to the specified color */
    }
  }
}

/* Draw a vertical line on the screen from y1 to y2 at x */
void
vline_draw(u8* screen, i32 y1, i32 y2, i32 x, u8 color)
{
  i32 i = 0;
  i32 height = 0;
  /* Ensure x is within screen width bounds */
  if (x < 0 || x >= kScreenWidth)
  {
    return; /* Do nothing if x is out of bounds */
  }

  /* Ensure y1 is less than or equal to y2 */
  if (y1 > y2)
  {
    i32 temp = y1;
    y1 = y2;
    y2 = temp; /* Swap y1 and y2 if necessary */
  }

  /* Clamp y1 and y2 within screen height bounds */
  if (y1 < 0)
  {
    y1 = 0;
  }
  if (y2 >= kScreenHeight)
  {
    y2 = kScreenHeight - 1;
  }

  /* Calculate the height of the line */
  height = y2 - y1 + 1;

  /* Draw the vertical line if height is valid */
  if (height > 0)
  {
    u8* pixel = &screen[y1 * kScreenWidth + x]; /* Get starting pixel address */
    for (; i < height; ++i)
    {
      pixel[i * kScreenWidth] = color; /* Set each pixel to the specified color */
    }
  }
}

static void
line_draw(u8* screen, i32 x1, i32 y1, i32 x2, i32 y2, u8 color)
{
  if (y1 == y2)
  {
    hline_draw(screen, x1, x2, y1, color);
  }
  else if (x1 == x2)
  {
    vline_draw(screen, y1, y2, x1, color);
  }
  else
  {
    i32 dx = abs(x2 - x1);
    i32 sx = x1 < x2 ? 1 : -1;
    i32 dy = -abs(y2 - y1);
    i32 sy = y1 < y2 ? 1 : -1;
    i32 error = dx + dy;

    for (;;)
    {
      i32 e2 = 0;
      /*ei->screen[kScreenWidth * y1 + x1] = color;*/
      pixel_draw(screen, x1, y1, color);
      if ((x1 == x2) && (y1 == y2))
      {
        break;
      }

      e2 = 2 * error;
      if (e2 >= dy)
      {
        error += dy;
        x1 += sx;
      }
      if (e2 <= dx)
      {
        error += dx;
        y1 += sy;
      }
    }
  }
}

static void
triangle_line_draw(u8* screen, i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, u8 color)
{
  line_draw(screen, x1, y1, x2, y2, color);
  line_draw(screen, x2, y2, x3, y3, color);
  line_draw(screen, x3, y3, x1, y1, color);
}

static void
rectangle_line_draw(u8* screen, i32 x1, i32 y1, i32 x2, i32 y2, u8 color)
{
  line_draw(screen, x1, y1, x2, y1, color);
  line_draw(screen, x1, y1, x1, y2, color);
  line_draw(screen, x2, y2, x2, y1, color);
  line_draw(screen, x2, y2, x1, y2, color);
}

/* Exotique core functions */

void
game_init(ExotiqueInterface* ei)
{
  memzero((u8*)ei->palette, 256 * sizeof(*ei->palette));
  /* Setting up the color palette */
  ei->palette[0] = 0x00000000;
  ei->palette[1] = 0x430067ff;
  ei->palette[2] = 0x94216aff;
  ei->palette[3] = 0xff004dff;
  ei->palette[4] = 0xff8426ff;
  ei->palette[5] = 0xffdd34ff;
  ei->palette[6] = 0x50e112ff;
  ei->palette[7] = 0x3fa66fff;
  ei->palette[8] = 0x365987ff;
  ei->palette[9] = 0x000000ff;
  ei->palette[10] = 0x0033ffff;
  ei->palette[11] = 0x29adffff;
  ei->palette[12] = 0x00ffccff;
  ei->palette[13] = 0xfff1e8ff;
  ei->palette[14] = 0xc2c3c7ff;
  ei->palette[15] = 0xab5236ff;
  ei->palette[16] = 0x5f574fff;
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
}

void
game_draw(ExotiqueInterface* ei)
{
  memzero(ei->screen, (u64)kScreenPixels);
  {
    i32 x1 = (i32)(next() % (u32)kScreenWidth);
    i32 y1 = (i32)(next() % (u32)kScreenHeight);
    i32 x2 = (i32)(next() % (u32)kScreenWidth);
    i32 y2 = (i32)(next() % (u32)kScreenHeight);
    u8 color = (u8)((next() % 16) + 1);
    line_draw(ei->screen, x1, y1, x2, y2, color);
  }

  /* Drawing circles */
  /* NOTE: The convoluted calculation below is to avoid drawing circles outside of the screen. */
  {
    i32 radius = (i32)(next() % 90) + 1;
    i32 x = (i32)(next() % (u32)kScreenWidth);
    i32 y = (i32)(next() % (u32)kScreenHeight);
    u8 color = (u8)((next() % 16) + 1);
    circle_line_draw(ei->screen, x, y, radius, color);
  }

  {
    i32 x1 = (i32)(next() % (u32)kScreenWidth);
    i32 y1 = (i32)(next() % (u32)kScreenHeight);
    i32 x2 = (i32)(next() % (u32)kScreenWidth);
    i32 y2 = (i32)(next() % (u32)kScreenHeight);
    i32 x3 = (i32)(next() % (u32)kScreenWidth);
    i32 y3 = (i32)(next() % (u32)kScreenHeight);
    u8 color = (u8)((next() % 16) + 1);
    triangle_line_draw(ei->screen, x1, y1, x2, y2, x3, y3, color);
  }

  {
    i32 x1 = (i32)(next() % (u32)kScreenWidth);
    i32 y1 = (i32)(next() % (u32)kScreenHeight);
    i32 x2 = (i32)(next() % (u32)kScreenWidth);
    i32 y2 = (i32)(next() % (u32)kScreenHeight);
    u8 color = (u8)((next() % 16) + 1);
    rectangle_line_draw(ei->screen, x1, y1, x2, y2, color);
  }
}
