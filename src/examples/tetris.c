#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 480;
const i32 kScreenHeight = 620;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* Memory utilities */

static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
  }
}

enum color_e
{
  eColorTransparent,
  eColorBlue,
  eColorOrange,
  eColorYellow,
  eColorRed,
  eColorGreen,
  eColorPurple,
  eColorCyan,
  eColorWhite,
  eColorShadow
};
typedef enum color_e color_t;

enum shape_e
{
  eShapeJ,
  eShapeL,
  eShapeSquare,
  eShapeZ,
  eShapeS,
  eShapeT,
  eShapeLine,
  eShapeFlash
};
typedef enum shape_e shape_t;

#define BWIDTH 10 /* board width, height */
#define BHEIGHT 20
#define BS 30        /* size of one block */
#define BS2 (BS / 2) /* size of half a block */
#define PREVIEW_BOX_X (10 + BS * BWIDTH + 10 + BS2)
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < kScreenWidth && (y) >= 0 && (y) < kScreenHeight)

char shapes[] =
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

i32 center[] = {
  /* helps center shapes in preview box */
  0,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  0,
  1,
  -1,
  1,
  1,
};

u8 colors[27] = {
  0,
  0,
  0, /* unused */
  96,
  96,
  248, /* J-piece 6060f8 */
  248,
  128,
  32, /* L-piece f88020 */
  248,
  216,
  0, /* square f8d800 */
  232,
  32,
  32, /* Z e82020 */
  0,
  216,
  0, /* S 00d800 */
  200,
  64,
  200, /* T c840c8 */
  32,
  200,
  248, /* line-piece 20c8f8 */
  255,
  255,
  255, /* shine color */
};

u8 board[BHEIGHT][BWIDTH];
i32 killy_lines[BHEIGHT];

i32 falling_x;
i32 falling_y;
shape_t falling_shape;
i32 is_falling_shape;
i32 falling_rot;
shape_t next_shape;
i32 lines;
i32 score;
i32 best;
i32 idle_time;
i32 shine_time;
i32 dead_time;

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

/* check if a sub-part of the falling shape is solid at a particular rotation */
static i32
is_solid_part(shape_t shape, i32 rot, i32 i, i32 j)
{
  i32 base = ((i32)shape + rot * 7) * 16; /* 16 chars per shape (4x4 grid) */
  return shapes[base + j * 4 + i] == 'O'; /* 4 chars per row */
}

/* check if the falling piece would collide at a certain position and rotation
 */
static i32
collide(i32 x, i32 y, i32 rot)
{
  i32 i = 0;
  for (; i < 4; ++i)
  {
    i32 j = 0;
    for (; j < 4; ++j)
    {
      i32 world_i = i + x;
      i32 world_j = j + y;

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

/* make a completed line "shine" and mark it to be removed */
static void
shine_line(i32 y)
{
  i32 i = 0;
  shine_time = 50;
  killy_lines[y] = 1;
  for (; i < BWIDTH; ++i)
  {
    board[y][i] = 8; /* shiny! */
  }
}

/* bake the falling piece into the background/board */
static void
bake(void)
{
  i32 i = 0;
  for (; i < 4; ++i)
  {
    i32 j = 0;
    for (; j < 4; ++j)
    {
      i32 world_i = i + falling_x;
      i32 world_j = j + falling_y;

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

      board[world_j][world_i] = (u8)(falling_shape + 1);
    }
  }

  /* check if there are any completed horizontal lines */
  {
    i32 j = BHEIGHT - 1;
    for (; j >= 0; --j)
    {
      i32 k = 0;
      for (; k < BWIDTH && board[j][k]; ++k)
      {
        if (k == BWIDTH - 1)
        {
          shine_line(j);
        }
      }
    }
  }
}

/* move the falling piece left, right, or down */
static void
move(i32 dx, i32 dy)
{
  if (!collide(falling_x + dx, falling_y + dy, falling_rot))
  {
    falling_x += dx;
    falling_y += dy;
  }
  else if (is_falling_shape && dy)
  {
    is_falling_shape = 0;
    bake();
  }

  if (dy)
  {
    idle_time = 0;
  }
}

/* move the falling piece as far down as it will go */

static void
slam(void)
{
  for (; !collide(falling_x, falling_y + 1, falling_rot); falling_y++)
  {
    idle_time = 0;
  }
}

/* spin the falling piece left or right, if possible */
static void
spin(void)
{
  i32 new_rot = (falling_rot + 1) % 4;

  if (!collide(falling_x, falling_y, new_rot))
  {
    falling_rot = new_rot;
  }
  else if (!collide(falling_x - 1, falling_y, new_rot))
  {
    falling_x -= 1;
    falling_rot = new_rot;
  }
}

/* randomly pick a new next piece, and put the old on in play */
static void
new_piece(void)
{
  falling_shape = next_shape;
  next_shape = next() % 7; /* 7 shapes */
  is_falling_shape = 1;
}

/* remove lines that were marked to be removed by shine_line() */
static void
kill_lines(void)
{
  i32 new_lines = 0;
  i32 y = 0;
  for (; y < BHEIGHT; ++y)
  {
    if (!killy_lines[y])
    {
      continue;
    }

    ++lines;
    ++new_lines;
    killy_lines[y] = 0;
    memzero(board[0], sizeof(*board));

    {
      i32 j = y;
      for (; j > 0; --j)
      {
        i32 i = 0;
        for (; i < BWIDTH; ++i)
        {
          board[j][i] = board[j - 1][i];
        }
      }
    }
  }

  switch (new_lines)
  {
    case 1:
      score += 100;
      break;
    case 2:
      score += 250;
      break;
    case 3:
      score += 500;
      break;
    case 4:
      score += 1000;
      break;
    default:
      break;
  }
}

static void
rectangle_fill_draw(u8* screen, i32 x1, i32 y1, i32 x2, i32 y2, color_t color)
{
  if (x1 > x2)
  {
    i32 temp = x1;
    x1 = x2;
    x2 = temp;
  }

  if (y1 > y2)
  {
    i32 temp = y1;
    y1 = y2;
    y2 = temp;
  }

  {
    i32 y = y1;
    for (; y < y2; ++y)
    {
      i32 x = x1;
      for (; x < x2; ++x)
      {
        if (IN_BOUNDS(x, y))
        {
          screen[y * kScreenWidth + x] = (u8)color;
        }
      }
    }
  }
}

/* draw a single square/piece of a shape */
static void
draw_square(u8* screen, i32 x, i32 y, shape_t shape)
{
  rectangle_fill_draw(screen, x, y, x + BS, y + BS, (color_t)shape);
}

/* Function to handle user input */
static void
input(PlayerInput buttons)
{
  if (is_falling_shape)
  {
    /*if (bit_get(buttons, bUp))*/
    if (buttons.up)
    {
      slam();
    }

    if (buttons.down)
    {
      move(0, 1);
    }

    if (buttons.left)
    {
      move(-1, 0);
    }

    if (buttons.right)
    {
      move(1, 0);
    }

    if (buttons.x)
    {
      spin();
    }
  }
}

/* XXX: Exotique core functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Color palette */
  memzero((u8*)ei->palette, 256 * sizeof(*ei->palette));

  ei->palette[eColorTransparent] = 0x00000000;
  ei->palette[eColorBlue] = 0x5ba8ffff;   /* J */
  ei->palette[eColorOrange] = 0xf68f37ff; /* L */
  ei->palette[eColorYellow] = 0xffe737ff; /* square */
  ei->palette[eColorRed] = 0xe03c28ff;    /* Z */
  ei->palette[eColorGreen] = 0x58d332ff;  /* S */
  ei->palette[eColorPurple] = 0xcc69e4ff; /* T */
  ei->palette[eColorCyan] = 0x25e2cdff;   /* line */
  ei->palette[eColorWhite] = 0xffffffff;  /* flash */
  ei->palette[eColorShadow] = 0x151515ff; /* flash */

  new_piece();
  if (best < score)
  {
    best = score;
  }
  score = 0;
  lines = 0;
}

void
game_update(ExotiqueInterface* ei)
{
  /* Handle user input */
  input(ei->input[0]);

  if (!shine_time && !dead_time && !is_falling_shape)
  {
    new_piece();
    falling_x = 3;
    falling_y = -3;
    falling_rot = 0;
  }

  if (shine_time > 0)
  {
    shine_time--;
    if (shine_time == 0)
    {
      kill_lines();
    }
  }

  if (dead_time > 0)
  {
    i32 x = (dead_time) % BWIDTH;
    i32 y = (dead_time) / BWIDTH;

    if (y >= 0 && y < BHEIGHT && x >= 0 && x < BWIDTH)
    {
      board[y][x] = (u8)(next() % 7 + 1);
    }

    if (--dead_time == 0)
    {
      /*new_game();*/
    }
  }

  if (idle_time >= 30)
  {
    move(0, 1);
  }
  ++idle_time;
}

void
game_draw(ExotiqueInterface* ei)
{
  memzero(ei->screen, (u64)kScreenPixels);

  /* draw falling piece & shadow */
  {
    i32 i = 0;
    for (; i < 4; ++i)
    {
      i32 j = 0;
      for (; j < 4; ++j)
      {
        i32 world_i = i + falling_x;
        i32 world_j = j + falling_y;
        i32 shadow_j = MAX(world_j + 1, 0);

        if (!is_solid_part(falling_shape, falling_rot, i, j))
        {
          continue;
        }

        rectangle_fill_draw(ei->screen, 10 + BS * world_i, 10 + BS * shadow_j, 10 + (BS * world_i) + BS, 10 + (BS * shadow_j) + (BS * (BHEIGHT - shadow_j)), eColorShadow);

        if (world_j >= 0)
        {
          draw_square(ei->screen, 10 + BS * world_i, 10 + BS * world_j, falling_shape + 1);
        }
      }
    }
  }

  /* draw next piece, centered in the preview box */
  {
    i32 i = 0;
    for (; i < 4; ++i)
    {
      i32 j = 0;
      for (; j < 4; ++j)
      {
        if (is_solid_part(next_shape, 0, i, j))
        {
          draw_square(ei->screen, PREVIEW_BOX_X + BS * i + BS2 * center[2 * next_shape], 10 + BS * j + BS2 * center[2 * next_shape + 1], next_shape + 1);
        }
      }
    }
  }

  /* draw board pieces */
  {
    i32 i = 0;
    for (; i < BWIDTH; ++i)
    {
      i32 j = 0;
      for (; j < BHEIGHT; ++j)
      {
        if (board[j][i])
        {
          draw_square(ei->screen, 10 + BS * i, 10 + BS * j, board[j][i]);
        }
      }
    }
  }
}
