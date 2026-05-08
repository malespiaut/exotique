#include "exotique.h"

/* Screen dimensions */
const i32 kScreenWidth = 160;
const i32 kScreenHeight = 100;
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

/* Game data structures */

enum game_constants
{
  kBallWidth = 2,
  kBallHeight = 2,
  kPaddleWidth = 16,
  kPaddleHeight = 2,
  kBrickWidth = 8,
  kBrickHeight = 3,
  kBrickColumns = 18,
  kBrickRows = 6,
  kBricks = 108,
  kBoundaryLeft = 8,
  kBoundaryRight = 152,
  kBoundaryTop = 16
};

enum color_e
{
  eColorTransparent,
  eColorWhite,
  eColorRed,
  eColorOrange,
  eColorYellow,
  eColorGreen,
  eColorBlue,
  eColorPurple,
  eColorBlack,
  eColor__COUNT
};
typedef enum color_e color_t;

typedef struct ball_s ball_t;
struct ball_s
{
  vec2i_t position;
  vec2i_t slope;
  vec2i_t speed;
  i32 size;
};

typedef struct brick_s brick_t;
struct brick_s
{
  vec2i_t position;
  vec2i_t size;
  color_t color;
  i32 hit;
};

typedef struct player_s player_t;
struct player_s
{
  vec2i_t position;
  i32 score;
  i32 lives;
};

typedef struct game_s game_t;
struct game_s
{
  player_t player;
  ball_t ball;
  brick_t brick[108];
  i32 difficulty;
  i32 bricks;
  i32 shoot;
  i32 start;
};

static game_t g_game = {0};

/* Drawing functions */

#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < kScreenWidth && (y) >= 0 && (y) < kScreenHeight)

static void
rectangle_fill_draw(u8* screen, i32 x1, i32 y1, i32 x2, i32 y2, color_t color)
{
  if (x1 > x2)
  {
    i32 tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  if (y1 > y2)
  {
    i32 tmp = y1;
    y1 = y2;
    y2 = tmp;
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

/* Game logic functions */

static i32
rectangles_collide(i32 x1, i32 y1, i32 w1, i32 h1, i32 x2, i32 y2, i32 w2, i32 h2)
{
  return !((y1 + h1 <= y2) || (y1 >= y2 + h2) ||
           (x1 + w1 <= x2) || (x1 >= x2 + w2));
}

static void
ball_collision_walls(game_t* g)
{
  if (g->ball.position.xy.x + kBallWidth > kBoundaryRight)
  {
    g->ball.slope.xy.x = -g->ball.slope.xy.x;
    g->ball.position.xy.x += g->ball.slope.xy.x;
  }

  if (g->ball.position.xy.x < kBoundaryLeft)
  {
    g->ball.slope.xy.x = -g->ball.slope.xy.x;
    g->ball.position.xy.x += g->ball.slope.xy.x;
  }

  if (g->ball.position.xy.y < kBoundaryTop)
  {
    g->ball.slope.xy.y = -g->ball.slope.xy.y;
    g->ball.position.xy.y += g->ball.slope.xy.y;
  }

  if (g->ball.position.xy.y > kScreenHeight - kBallHeight)
  {
    g->shoot = 1;
    --g->player.lives;
  }
}

static void
ball_collision_paddle(game_t* g)
{
  i32 ball_bottom = g->ball.position.xy.y + kBallHeight;
  i32 paddle_left = g->player.position.xy.x;
  i32 paddle_right = paddle_left + kPaddleWidth;

  if ((ball_bottom == g->player.position.xy.y) &&
      (g->ball.position.xy.x >= paddle_left) &&
      (g->ball.position.xy.x <= paddle_right))
  {
    g->ball.slope.xy.y = -g->ball.slope.xy.y;
  }
}

static void
ball_collision_bricks(game_t* g)
{
  i32 i = 0;
  for (; i < kBricks; ++i)
  {
    if (!g->brick[i].hit)
    {
      i32 next_x = g->ball.position.xy.x + g->ball.slope.xy.x;
      i32 next_y = g->ball.position.xy.y + g->ball.slope.xy.y;

      if (rectangles_collide(next_x, g->ball.position.xy.y, kBallWidth, kBallHeight, g->brick[i].position.xy.x, g->brick[i].position.xy.y, g->brick[i].size.wh.width, g->brick[i].size.wh.height))
      {
        g->brick[i].hit = 1;
        g->ball.slope.xy.x = -g->ball.slope.xy.x;
        --g->bricks;
        g->player.score += 10;
      }
      else if (rectangles_collide(g->ball.position.xy.x, next_y, kBallWidth, kBallHeight, g->brick[i].position.xy.x, g->brick[i].position.xy.y, g->brick[i].size.wh.width, g->brick[i].size.wh.height))
      {
        g->brick[i].hit = 1;
        g->ball.slope.xy.y = -g->ball.slope.xy.y;
        --g->bricks;
        g->player.score += 10;
      }
    }
  }
}

static void
ball_reset(game_t* g)
{
  g->ball.position.xy.x = kScreenWidth / 2;
  g->ball.position.xy.y = kScreenHeight / 2;
  g->shoot = 0;
}

static void
ball_move(game_t* g)
{
  if (!g->shoot && g->player.lives)
  {
    g->ball.position.xy.x += g->ball.speed.xy.x * g->ball.slope.xy.x;
    g->ball.position.xy.y += g->ball.speed.xy.y * g->ball.slope.xy.y;
  }
  else
  {
    ball_reset(g);
  }
}

static void
ball_update(game_t* g)
{
  ball_collision_walls(g);
  ball_collision_paddle(g);
  ball_collision_bricks(g);
  ball_move(g);
}

static void
bricks_draw(u8* screen, game_t* g)
{
  i32 i = 0;
  for (; i < kBricks; ++i)
  {
    if (!g->brick[i].hit)
    {
      rectangle_fill_draw(screen,
                          g->brick[i].position.xy.x,
                          g->brick[i].position.xy.y,
                          g->brick[i].position.xy.x + g->brick[i].size.wh.width,
                          g->brick[i].position.xy.y + g->brick[i].size.wh.height,
                          g->brick[i].color);
    }
  }
}

static void
paddle_draw(u8* screen, game_t* g)
{
  rectangle_fill_draw(screen,
                      g->player.position.xy.x,
                      g->player.position.xy.y,
                      g->player.position.xy.x + kPaddleWidth,
                      g->player.position.xy.y + kPaddleHeight,
                      eColorRed);
}

static void
walls_draw(u8* screen)
{
  /* Right wall */
  rectangle_fill_draw(screen, 0, 8, 8, 98, eColorWhite);
  /* Left wall */
  rectangle_fill_draw(screen, 152, 8, 160, 98, eColorWhite);
  /* Top wall */
  rectangle_fill_draw(screen, 8, 8, 152, 16, eColorWhite);
}

static void
ball_draw(u8* screen, game_t* g)
{
  rectangle_fill_draw(screen,
                      g->ball.position.xy.x,
                      g->ball.position.xy.y,
                      g->ball.position.xy.x + kBallWidth,
                      g->ball.position.xy.y + kBallHeight,
                      eColorRed);
}

static void
sprite_draw(u8* screen, i32 x, i32 y, i32 n, color_t color)
{
  static const u64 font[10] = {
    0xffff0ff0ff0ffff0, /* 0 */
    0x00f00f00f00f00f0, /* 1 */
    0xfff00fffff00fff0, /* 2 */
    0xfff00ffff00ffff0, /* 3 */
    0xf0ff0ffff00f00f0, /* 4 */
    0xffff00fff00ffff0, /* 5 */
    0xffff00ffff0ffff0, /* 6 */
    0xfff00f00f00f00f0, /* 7 */
    0xffff0fffff0ffff0, /* 8 */
    0xffff0ffff00ffff0  /* 9 */
  };

  if (n < 0 || n > 9)
  {
    return;
  }

  {
    i32 row = 0;
    i32 pix = 0;
    u64 sprite = font[n];
    for (; pix < 64; ++pix)
    {
      if (sprite & (0x8000000000000000 >> pix))
      {
        i32 px = x + (pix % 12);
        i32 py = y + row;
        if (IN_BOUNDS(px, py))
        {
          screen[py * kScreenWidth + px] = (u8)color;
        }
      }
      if (!((pix + 1) % 12))
      {
        ++row;
      }
    }
  }
}

static void
numbers_draw(u8* screen, i32 x, i32 y, i32 number, i32 digits, color_t color)
{
  i32 offset = 0;
  i32 i = 0;
  for (; i < digits; ++i)
  {
    i32 num = number;
    i32 div = 1;
    i32 n = i + 1;
    for (; n < digits; ++n)
    {
      div *= 10;
    }
    num /= div;
    num %= 10;
    offset += 16;
    sprite_draw(screen, x + offset, y, num, color);
  }
}

/* Exotique interface functions */

void
game_init(ExotiqueInterface* ei)
{
  /* Color palette */
  memzero((u8*)ei->palette, 256 * sizeof(*ei->palette));

  ei->palette[eColorTransparent] = 0x00000000;
  ei->palette[eColorWhite] = 0xfdfdf8ff;
  ei->palette[eColorRed] = 0xd32734ff;
  ei->palette[eColorOrange] = 0xda7d22ff;
  ei->palette[eColorYellow] = 0xe6da29ff;
  ei->palette[eColorGreen] = 0x28c641ff;
  ei->palette[eColorBlue] = 0x2d93ddff;
  ei->palette[eColorPurple] = 0x7b53adff;
  ei->palette[eColorBlack] = 0x1b1c33ff;

  /* Ball init */
  g_game.ball.position.xy.x = kScreenWidth / 2;
  g_game.ball.position.xy.y = kScreenHeight / 2;
  g_game.ball.slope.xy.x = -1;
  g_game.ball.slope.xy.y = -1;
  g_game.ball.speed.xy.x = 1;
  g_game.ball.speed.xy.y = 1;

  /* Player init */
  g_game.player.lives = 3;
  g_game.player.position.xy.x = kScreenWidth / 2 - kPaddleWidth / 2;
  g_game.player.position.xy.y = kScreenHeight - 6;
  g_game.player.score = 0;

  /* Game init */
  g_game.difficulty = 1;
  g_game.shoot = 1;
  g_game.start = 0;

  /* Bricks init */
  g_game.bricks = kBrickColumns * kBrickRows;
  /* Bricks block is set at position (8, 28) */
  {
    i32 n = 0;
    i32 color = 0;

    i32 i = 0;
    for (; i < kBrickRows; ++i)
    {
      i32 j = 0;
      for (; j < kBrickColumns; ++j)
      {
        g_game.brick[n].position.xy.x = 8 + (j * kBrickWidth);
        g_game.brick[n].position.xy.y = 28 + (i * kBrickHeight);
        g_game.brick[n].size.wh.width = kBrickWidth;
        g_game.brick[n].size.wh.height = kBrickHeight;
        g_game.brick[n].color = (color_t)((color % 6) + 2);
        g_game.brick[n].hit = 0;
        ++n;
      }
      ++color;
    }
  }
}

static void
paddle_update(game_t* g, i32 mouse_x)
{
  g->player.position.xy.x = mouse_x;

  /* Keeping the paddle within the boundaries of the game area */
  if (g->player.position.xy.x < 8)
  {
    g->player.position.xy.x = 8;
  }
  if (g->player.position.xy.x > 136)
  {
    g->player.position.xy.x = 136;
  }
}

void
game_update(ExotiqueInterface* ei)
{
  if (ei->mouse_click)
  {
    g_game.start = 1;
  }

  if (g_game.start && g_game.player.lives)
  {
    ball_update(&g_game);
  }

  paddle_update(&g_game, ei->mouse.xy.x);
}

void
game_draw(ExotiqueInterface* ei)
{
  memzero(ei->screen, (u64)kScreenPixels);

  walls_draw(ei->screen);
  bricks_draw(ei->screen, &g_game);
  paddle_draw(ei->screen, &g_game);
  ball_draw(ei->screen, &g_game);

  /* Draw score */
  numbers_draw(ei->screen, 36, 2, g_game.player.score, 3, eColorWhite);
  /* Draw lives */
  numbers_draw(ei->screen, 100, 2, g_game.player.lives, 1, eColorWhite);
}
