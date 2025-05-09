#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 160;
const i32 kScreenHeight = 100;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* XXX: Structures and types */

enum color_e
{
  eColorTransparent,
  eColorBlack,
  eColorWhite,
  eColorRed,
  eColorGreen
};
typedef enum color_e color_t;

/* XXX: Data */
static u8 img_cursor[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xe0, 0x10, 0x00};

/* XXX: Drawing functions */

static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
  }
}


static void
pixel_draw(u8* screen, i32 x, i32 y, u8 color)
{
  i32 position = x + (y * kScreenWidth);
  if ((position >= 0) && (position < kScreenPixels))
  {
    screen[position] = color;
  }
}


void
bit_draw(ExotiqueInterface* ei, u8* sprite, i32 x, i32 y, i32 width, i32 height, u8 color)
{
  i32 col;
  i32 row;
  for (row = 0; row < height; ++row)
  {
    for (col = 0; col < width; ++col)
    {
      if (sprite[row] << col & 0x80)
      {
        if ((x + col) >= 0 && (x + col <= kScreenWidth) && (y + row) >= 0 && (y + row) <= kScreenHeight)
        {
          ei->screen[(x + col) + kScreenWidth * (y + row)] = color;
        }
      }
    }
  }
}

/* XXX: Shapes draw */

/* Draw a horizontal line on the screen from x1 to x2 at y */
static void
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
static void
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



/* XXX: UI draw */


static void
button_draw(u8* screen, i32 x, i32 y, u8 color)
{
  hline_draw(screen, x+2, x+12, y, color);
  hline_draw(screen, x+2, x+12, y+14, color);
 
  vline_draw(screen, y+2, y+12, x, color);
  vline_draw(screen, y+2, y+12, x+14, color);

  pixel_draw(screen, x+1, y+1, color); /* top left corner */
pixel_draw(screen, x+13, y+1, color); /* top right corner */
pixel_draw(screen, x+1, y+13, color); /* bottom left corner */
pixel_draw(screen, x+13, y+13, color); /* bottom right corner */
}

static void
ui_draw(u8* screen)
{
  button_draw(screen, 40, 104, eColorWhite);
  button_draw(screen, 56, 104, eColorWhite);
  button_draw(screen, 72, 104, eColorWhite);
  button_draw(screen, 88, 104, eColorGreen);

  button_draw(screen, 40, 120, eColorWhite);
  button_draw(screen, 56, 120, eColorWhite);
  button_draw(screen, 72, 120, eColorWhite);
  button_draw(screen, 88, 120, eColorGreen);

  button_draw(screen, 40, 136, eColorWhite);
  button_draw(screen, 56, 136, eColorWhite);
  button_draw(screen, 72, 136, eColorWhite);
  button_draw(screen, 88, 136, eColorGreen);

  button_draw(screen, 40, 152, eColorWhite);
  button_draw(screen, 56, 152, eColorGreen);
  button_draw(screen, 72, 152, eColorGreen);
  button_draw(screen, 88, 152, eColorGreen);

  button_draw(screen, 40, 168, eColorRed);
  button_draw(screen, 56, 168, eColorRed);
  button_draw(screen, 72, 168, eColorRed);
  button_draw(screen, 88, 168, eColorRed);

  button_draw(screen, 40, 184, eColorRed);
  button_draw(screen, 56, 184, eColorRed);
  button_draw(screen, 72, 184, eColorRed);
  button_draw(screen, 88, 184, eColorRed);

}

/* XXX: Exotique functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Setting up the color palette */
  ei->palette[0] = 0x00000000;
  ei->palette[1] = 0x000000ff;
  ei->palette[2] = 0xeeeeeeff;
  ei->palette[3] = 0xdd6655ff;
  ei->palette[4] = 0x77cc99ff;
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

  ui_draw(ei->screen);
  /* Draw cursor */
  bit_draw(ei, img_cursor, ei->mouse.xy.x, ei->mouse.xy.y, 8, 8, eColorWhite);
}
