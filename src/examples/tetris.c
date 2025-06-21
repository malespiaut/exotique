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
  eColorWhite
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

/* SDL_Event event;
SDL_Renderer* renderer;
TTF_Font* font; */

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
  /*  i32 base = shape * 5 + rot * 5 * 8 * 4; */
  /*  return shapes[base + j * 5 * 8 + i] == 'O'; */
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
/*
static void
slam(void)
{
  for (; !collide(falling_x, falling_y + 1, falling_rot); falling_y++)
  {
    idle_time = 0;
  }
}
*/

/* spin the falling piece left or right, if possible */
/*
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
}*/

/* handle a key press from the player */
/*static void
key_down(void)
{*/
/*
if (is_falling_shape)
  switch (event.key.keysym.sym)
  {
    case SDLK_a:
    case SDLK_LEFT:
      move(-1, 0);
      break;
    case SDLK_d:
    case SDLK_RIGHT:
      move(1, 0);
      break;
    case SDLK_w:
    case SDLK_UP:
      slam();
      break;
    case SDLK_s:
    case SDLK_DOWN:
      move(0, 1);
      break;
    case SDLK_q:
    case SDLK_z:
      spin();
      break;
    case SDLK_e:
    case SDLK_x:
      spin();
      break;
    default:
      break;
  }
  */
/*}*/

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
    /*memset(board[0], 0, sizeof *board);*/
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

/* reset score and pick one extra random piece */
static void
new_game(void)
{
}

/* set the current draw color to the color assoc. with a shape */
/*
static void
set_color_from_shape(i32 shape, i32 shade)
{
  i32 r = MAX(colors[shape * 3 + 0] + shade, 0);
  i32 g = MAX(colors[shape * 3 + 1] + shade, 0);
  i32 b = MAX(colors[shape * 3 + 2] + shade, 0);
  SDL_SetRenderDrawColor(renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
}*/

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
  /*
  set_color_from_shape(shape, -25);
  SDL_RenderDrawRect(renderer, &(SDL_Rect){x, y, BS, BS});
  set_color_from_shape(shape, 0);
  SDL_RenderFillRect(renderer, &(SDL_Rect){1 + x, 1 + y, BS - 2, BS - 2});
  */
  rectangle_fill_draw(screen, x, y, x + BS, y + BS, (color_t)shape);
}

/* render a centered line of text optionally with a %d value in it */
/*
static void
text(const char* fstr, i32 value, i32 x, i32 y)
{
  (void)fstr;
  (void)value;
  (void)x;
  (void)y;*/
/*
if (!font)
{
  return;
}
i32 w, h;
char msg[80];
snprintf(msg, 80, fstr, value);
TTF_SizeText(font, msg, &w, &h);
SDL_Surface* msgsurf = TTF_RenderText_Blended(font, msg, (SDL_Color){80, 90, 85, 255});
SDL_Texture* msgtex = SDL_CreateTextureFromSurface(renderer, msgsurf);
SDL_Rect fromrec = {0, 0, msgsurf->w, msgsurf->h};
SDL_Rect torec = {x, y, msgsurf->w, msgsurf->h};
SDL_RenderCopy(renderer, msgtex, &fromrec, &torec);
SDL_DestroyTexture(msgtex);
SDL_FreeSurface(msgsurf);
*/
/*}*/

/* XXX: Exotique core functions */

void
game_load(ExotiqueInterface* ei)
{
  (void)ei;
  /*srand((unsigned int)time(NULL));
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* win = SDL_CreateWindow("Tet", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 10 + BWIDTH * BS + 10 + 5 * BS + 10, 10 + BHEIGHT * BS + 10, SDL_WINDOW_SHOWN);

  renderer = SDL_CreateRenderer(win, -1, 0);
  if (!renderer)
  {
    fpr
    intf(stderr, "Could not create SDL renderer for some reason\n");
    exit(-1);
  }

  TTF_Init();
  font = TTF_OpenFont("res/LiberationSans-Regular.ttf", 28);
  memset(board, 0, sizeof board);
  */
  /* is_falling_shape = 0; */

  /* Color palette */
  memzero((u8*)ei->palette, 256 * sizeof(*ei->palette));

  ei->palette[eColorTransparent] = 0x00000000;
  ei->palette[eColorBlue] = 0x6060f8ff;   /* J */
  ei->palette[eColorOrange] = 0xf88020ff; /* L */
  ei->palette[eColorYellow] = 0xf8d800ff; /* square */
  ei->palette[eColorRed] = 0xe82020ff;    /* Z */
  ei->palette[eColorGreen] = 0x00d800ff;  /* S */
  ei->palette[eColorPurple] = 0xc840c8ff; /* T */
  ei->palette[eColorCyan] = 0x20c8f8ff;   /* line */
  ei->palette[eColorWhite] = 0xffffffff;  /* flash */

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
  (void)ei;
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
      new_game();
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
  (void)ei;
  /* draw background, black boxes */
  /*
  SDL_SetRenderDrawColor(renderer, 25, 40, 35, 255);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(renderer, &(SDL_Rect){10, 10, BS * BWIDTH, BS * BHEIGHT});
  SDL_RenderFillRect(renderer, &(SDL_Rect){10 + BS * BWIDTH + 10, 10, BS * 5, BS * 5});
  */

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
        /*i32 shadow_j = MAX(world_j + 1, 0);*/

        if (!is_solid_part(falling_shape, falling_rot, i, j))
        {
          continue;
        }

        /*
        SDL_SetRenderDrawColor(renderer, 8, 13, 12, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){10 + BS * world_i, 10 + BS * shadow_j, BS, BS * (BHEIGHT - shadow_j)});
        */

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

  /* draw counters and instructions */
  /*
  text("Lines:", 0, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 0);
  text("%d", lines, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 30);
  text("Score:", 0, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 70);
  text("%d", score, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 100);
  text("Best:", 0, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 140);
  text("%d", best, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 170);
  text("Controls:", 0, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 370);
  text("arrows, z, x", 0, 10 + BS * BWIDTH + 10, 10 + BS * 5 + 10 + 400);

  SDL_RenderPresent(renderer);
  */
}
