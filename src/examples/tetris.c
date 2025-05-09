#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 320;
const i32 kScreenHeight = 200;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* XXX: Game data */

#define kBoardWidth 10
#define kBoardHeight 20
#define kBlockSize 30
#define kBlockSizeHalf (kBlockSize / 2)
#define KPreviewBoxX (10 + (kBlockSize * bBoardWidth) + 10 + kBlockSizeHalf)

char g_shapes[] =
/* rotation 0 */
  "...."
  ".OO."
  ".O.."
  ".O.."

  "...."
  ".OO."
  "..O."
  "..O."

  "...."
  ".OO."
  ".OO."
  "...."

  "...."
  "..O."
  ".OO."
  ".O.."

  "...."
  ".O.."
  ".OO."
  "..O."

  "...."
  "...."
  "OOO."
  ".O.."

  ".O.."
  ".O.."
  ".O.."
  ".O.."

/* rotation 1 */
  "...."
  ".O.."
  ".OOO"
  "...."

  "...."
  "..O."
  "OOO."
  "...."

  "...."
  ".OO."
  ".OO."
  "...."

  "...."
  "OO.."
  ".OO."
  "...."

  "...."
  ".OO."
  "OO.."
  "...."

  "...."
  ".O.."
  ".OO."
  ".O.."

  "...."
  "...."
  "OOOO"
  "...."

/* rotation 2 */
  "...."
  "..O."
  "..O."
  ".OO."

  "...."
  ".O.."
  ".O.."
  ".OO."

  "...."
  ".OO."
  ".OO."
  "...."

  "...."
  "..O."
  ".OO."
  ".O.."

  "...."
  ".O.."
  ".OO."
  "..O."

  "...."
  ".O.."
  "OOO."
  "...."

  ".O.."
  ".O.."
  ".O.."
  ".O.."

/* rotation 3 */
  "...."
  ".OOO"
  "...O"
  "...."

  "...."
  "OOO."
  "O..."
  "...."

  "...."
  ".OO."
  ".OO."
  "...."

  "...."
  "OO.."
  ".OO."
  "...."

  "...."
  ".OO."
  "OO.."
  "...."

  "...."
  ".O.."
  "OO.."
  ".O.."

  "...."
  "...."
  "OOOO"
  "....";

/*
char g_shapes[][41] =
  ".... .... .... .... .... .... .... .O.. "
  ".... .OO. .OO. .OO. ..O. .O.. .... .O.. "
  ".... .O.. ..O. .OO. .OO. .OO. OOO. .O.. "
  ".... .O.. ..O. .... .O.. ..O. .O.. .O.. "
  ".... .... .... .... .... .... .... .... "
  ".... .O.. ..O. .OO. OO.. .OO. .O.. .... "
  ".... .OOO OOO. .OO. .OO. OO.. .OO. OOOO "
  ".... .... .... .... .... .... .O.. .... "
  ".... .... .... .... .... .... .... .O.. "
  ".... ..O. .O.. .OO. ..O. .O.. .O.. .O.. "
  ".... ..O. .O.. .OO. .OO. .OO. OOO. .O.. "
  ".... .OO. .OO. .... .O.. ..O. .... .O.. "
  ".... .... .... .... .... .... .... .... "
  ".... .OOO OOO. .OO. OO.. .OO. .O.. .... "
  ".... ...O O... .OO. .OO. OO.. OO.. OOOO "
  ".... .... .... .... .... .... .O.. .... ";
*/

i32 g_shapes_center[] = {
  /* helps center shapes in preview box */
  0,   0,  0,  0,
  0,   0,  0,  1,
  0,   0,  0,  0,
  1,  -1,  1,  1,
};

u8 g_shapes_colors[] = {
    0,   0,   0, /* unused */
  242, 245, 237, /* J-piece */
  255, 194,   0, /* L-piece */
   15, 127, 127, /* square */
  255,  91,   0, /* Z */
  184,   0,  40, /* S */
   74, 192, 242, /* T */
  132,   0,  46, /* line-piece */
  255, 255, 255, /* shine color */
};


typedef struct Tetris Tetris;
struct Tetris
{
u8 board[kBoardHeight][kBoardWidth];
i32 killy_lines[kBoardHeight];
i32 falling_x;
i32 falling_y;
i32 falling_shape;
i32 falling_rot;
i32 next_shape;
i32 lines;
i32 score;
i32 best;
i32 idle_time;
i32 shine_time;
i32 dead_time;
i32 is_falling_shape;
};

Tetris g_tetris = {0};

/* XXX: System functions */

static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
  }
}

/* XXX: 32-bits PRNG - xoshiro128++ */

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

/* XXX: Shape functions */

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

/* XXX: Tetris functions */

static void
piece_new(void)
{
  g_tetris.is_falling_shape = 1;
  g_tetris.falling_shape = g_tetris.next_shape;
  g_tetris.next_shape = (i32)(next() % 7);
}

/* remove lines that were marked to be removed by shine_line() */
static void
kill_lines(void)
{
  i32 y;
  i32 i;
  i32 j;
  i32 new_lines = 0;

  for (y = 0; y < kBoardHeight; y++)
  {
    if (!g_tetris.killy_lines[y])
    {
      continue;
    }

    g_tetris.lines++;
    new_lines++;
    g_tetris.killy_lines[y] = 0;
    memzero(g_tetris.board[0], sizeof *g_tetris.board);

    for (j = y; j > 0; j--)
    {
      for (i = 0; i < kBoardWidth; i++)
      {
        g_tetris.board[j][i] = g_tetris.board[j - 1][i];
      }
    }
  }

  switch (new_lines)
  {
    case 1:
      g_tetris.score += 100;
      break;
    case 2:
      g_tetris.score += 250;
      break;
    case 3:
      g_tetris.score += 500;
      break;
    case 4:
      g_tetris.score += 1000;
      break;
    default:
      break;
  }
}

/* move the falling piece left, right, or down */
static void
move(i32 dx, i32 dy)
{
  if (!collide(g_tetris.falling_x + dx, g_tetris.falling_y + dy, g_tetris.falling_rot))
  {
    g_tetris.falling_x += dx;
    g_tetris.falling_y += dy;
  }
  else if (g_tetris.is_falling_shape && dy)
  {
    g_tetris.is_falling_shape = 0;
    bake();
  }

  if (dy)
  {
    g_tetris.idle_time = 0;
  }
}

/* check if a sub-part of the falling shape is solid at a particular rotation */
static i32 /* l0ve u bb */
is_solid_part(int shape, int rot, int i, int j)
{
/*  int base = shape * 5 + rot * 5 * 8 * 4*/
/*  return shapes[base + j * 5 * 8 + i] == 'O'; */
  int base = (shape + rot * 7) * 16;  /* 16 chars per shape (4x4 grid) */
  return shapes[base + j * 4 + i] == 'O';  /* 4 chars per row */
}

/* check if the falling piece would collide at a certain position and rotation */
int
collide(int x, int y, int rot)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      int world_i = i + x;
      int world_j = j + y;

      if (!is_solid_part(falling_shape, rot, i, j))
      {
        continue;
      }

      if (world_i < 0 || world_i >= BWIDTH || world_j >= BHEIGHT)
      {
        return 1;
      }

      if (world_j < 0)
      {
        continue;
      }

      if (board[world_j][world_i])
      {
        return 1;
      }
    }
  }
  return 0;
}

/* bake the falling piece into the background/board */
void
bake(void)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      int world_i = i + falling_x;
      int world_j = j + falling_y;

      if (!is_solid_part(falling_shape, falling_rot, i, j))
      {
        continue;
      }

      if (world_i < 0 || world_i >= BWIDTH || world_j < 0 || world_j >= BHEIGHT)
      {
        continue;
      }

      if (board[world_j][world_i]) /* already a block here? game over */
      {
        dead_time = BWIDTH * BHEIGHT;
        next_shape = 0;
      }

      board[world_j][world_i] = falling_shape + 1;
    }
  }

  /* check if there are any completed horizontal lines */
  for (int j = BHEIGHT - 1; j >= 0; j--)
  {
    for (int i = 0; i < BWIDTH && board[j][i]; i++)
    {
      if (i == BWIDTH - 1)
      {
        shine_line(j);
      }
    }
  }
}

/* make a completed line "shine" and mark it to be removed */
void
shine_line(int y)
{
  shine_time = 50;
  killy_lines[y] = 1;
  for (int i = 0; i < BWIDTH; i++)
  {
    board[y][i] = 8; /* shiny! */
  }
}




/* XXX: Exotique core functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Setting up the color palette */
  memzero((u8*)ei->palette, 256 * sizeof(*ei->palette));
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

  /* DONE setup(); */
  /* DONE new_game(); */

  /* Tetris initialisation */
  memzero((void*)g_tetris.board, kBoardWidth * kBoardHeight);
  /* DONE new_piece(); */
  piece_new();
  if (g_tetris.best < g_tetris.score)
  {
    g_tetris.best = g_tetris.score;
  }
  g_tetris.score = 0;
  g_tetris.lines = 0;
  g_tetris.falling_shape = 0;
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
  /* key_down(); */
  /* update_stuff(); */
  if (!g_tetris.shine_time && !g_tetris.dead_time && !g_tetris.is_falling_shape)
  {
    piece_new();
    g_tetris.falling_x = 3;
    g_tetris.falling_y = -3;
    g_tetris.falling_rot = 0;
  }

  if (g_tetris.shine_time > 0)
  {
    g_tetris.shine_time--;
    if (g_tetris.shine_time == 0)
    {
      kill_lines();
    }
  }

  if (g_tetris.dead_time > 0)
  {
    i32 x = (g_tetris.dead_time) % kBoardWidth;
    i32 y = (g_tetris.dead_time) / kBoardWidth;

    if (y >= 0 && y < kBoardHeight && x >= 0 && x < kBoardWidth)
    {
      g_tetris.board[y][x] = (u8)(next() % 7 + 1);
    }

    if (--g_tetris.dead_time == 0)
    {
      /*new_game();*/
      game_load(ei);
    }
  }

  if (g_tetris.idle_time >= 30)
  {
    move(0, 1);
  }
}

void
game_draw(ExotiqueInterface* ei)
{
  /*(void)ei;*/
  memzero(ei->screen, kScreenPixels);

  /* draw_stuff(); */

  {
    i32 x1 = (i32)(next() % kScreenWidth);
    i32 y1 = (i32)(next() % kScreenHeight);
    i32 x2 = (i32)(next() % kScreenWidth);
    i32 y2 = (i32)(next() % kScreenHeight);
    u8 color = (next() % 16) + 1;
    line_draw(ei->screen, x1, y1, x2, y2, color);
  }

  /* Drawing circles */
  /* NOTE: The convoluted calculation below is to avoid drawing circles outside of the screen. */
  {
    i32 radius = (i32)(next() % 90) + 1;
    i32 x = (i32)(next() % kScreenWidth);
    i32 y = (i32)(next() % kScreenHeight);
    u8 color = (next() % 16) + 1;
    circle_line_draw(ei->screen, x, y, radius, color);
  }

  {
    i32 x1 = (i32)(next() % kScreenWidth);
    i32 y1 = (i32)(next() % kScreenHeight);
    i32 x2 = (i32)(next() % kScreenWidth);
    i32 y2 = (i32)(next() % kScreenHeight);
    i32 x3 = (i32)(next() % kScreenWidth);
    i32 y3 = (i32)(next() % kScreenHeight);
    u8 color = (next() % 16) + 1;
    triangle_line_draw(ei->screen, x1, y1, x2, y2, x3, y3, color);
  }

  {
    i32 x1 = (i32)(next() % kScreenWidth);
    i32 y1 = (i32)(next() % kScreenHeight);
    i32 x2 = (i32)(next() % kScreenWidth);
    i32 y2 = (i32)(next() % kScreenHeight);
    u8 color = (next() % 16) + 1;
    rectangle_line_draw(ei->screen, x1, y1, x2, y2, color);
  }
  /* idle_time++ */ /* ??? */
}
