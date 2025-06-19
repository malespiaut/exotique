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
  eBallWidth = 2,
  eBallHeight = 2,
  ePaddleWidth = 16,
  ePaddleHeight = 2,
  eBrickWidth = 8,
  eBrickHeight = 3,
  eBrickColumns = 18,
  eBrickRows = 6,
  eBricks = 108,
  eTimeStop = 2000,
  eBoundaryLeft = 8,
  eBoundaryRight = 152,
  eBoundaryTop = 16
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
  eColorBlack
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
};

static game_t g_game = {0};

/* Drawing functions */

#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < kScreenWidth && (y) >= 0 && (y) < kScreenHeight)

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

/* Game logic functions */

static i32
rectangles_collide(i32 x1, i32 y1, i32 w1, i32 h1, i32 x2, i32 y2, i32 w2, i32 h2)
{
  return !((y1 + h1 <= y2) || (y1 >= y2 + h2) ||
           (x1 + w1 <= x2) || (x1 >= x2 + w2));
}

static void
ball_collision_walls(void)
{
  if (g_game.ball.position.xy.x + eBallWidth > eBoundaryRight)
  {
    g_game.ball.slope.xy.x = -g_game.ball.slope.xy.x;
    g_game.ball.position.xy.x += g_game.ball.slope.xy.x;
  }

  if (g_game.ball.position.xy.x < eBoundaryLeft)
  {
    g_game.ball.slope.xy.x = -g_game.ball.slope.xy.x;
    g_game.ball.position.xy.x += g_game.ball.slope.xy.x;
  }

  if (g_game.ball.position.xy.y < eBoundaryTop)
  {
    g_game.ball.slope.xy.y = -g_game.ball.slope.xy.y;
    g_game.ball.position.xy.y += g_game.ball.slope.xy.y;
  }

  if (g_game.ball.position.xy.y > kScreenHeight - eBallHeight)
  {
    g_game.shoot = 1;
    --g_game.player.lives;
  }
}

static void
ball_collision_paddle(void)
{
  i32 ball_bottom = g_game.ball.position.xy.y + eBallHeight;
  i32 paddle_left = g_game.player.position.xy.x;
  i32 paddle_right = paddle_left + ePaddleWidth;

  if ((ball_bottom == g_game.player.position.xy.y) &&
      (g_game.ball.position.xy.x >= paddle_left) &&
      (g_game.ball.position.xy.x <= paddle_right))
  {
    g_game.ball.slope.xy.y = -g_game.ball.slope.xy.y;
  }
}

static void
ball_collision_bricks(void)
{
  i32 i = 0;
  for (; i < eBricks; ++i)
  {
    if (!g_game.brick[i].hit)
    {
      i32 next_x = g_game.ball.position.xy.x + g_game.ball.slope.xy.x;
      i32 next_y = g_game.ball.position.xy.y + g_game.ball.slope.xy.y;

      if (rectangles_collide(next_x, g_game.ball.position.xy.y, eBallWidth, eBallHeight, g_game.brick[i].position.xy.x, g_game.brick[i].position.xy.y, g_game.brick[i].size.wh.width, g_game.brick[i].size.wh.height))
      {
        g_game.brick[i].hit = 1;
        g_game.ball.slope.xy.x = -g_game.ball.slope.xy.x;
        --g_game.bricks;
        g_game.player.score += 10;
      }
      else if (rectangles_collide(g_game.ball.position.xy.x, next_y, eBallWidth, eBallHeight, g_game.brick[i].position.xy.x, g_game.brick[i].position.xy.y, g_game.brick[i].size.wh.width, g_game.brick[i].size.wh.height))
      {
        g_game.brick[i].hit = 1;
        g_game.ball.slope.xy.y = -g_game.ball.slope.xy.y;
        --g_game.bricks;
        g_game.player.score += 10;
      }
    }
  }
}

static void
ball_reset(void)
{
  g_game.ball.position.xy.x = kScreenWidth / 2;
  g_game.ball.position.xy.y = kScreenHeight / 2;
  g_game.shoot = 0;
}

static void
ball_move(void)
{
  if (!g_game.shoot)
  {
    g_game.ball.position.xy.x += g_game.ball.speed.xy.x * g_game.ball.slope.xy.x;
    g_game.ball.position.xy.y += g_game.ball.speed.xy.y * g_game.ball.slope.xy.y;
  }
  else
  {
    ball_reset();
  }
}

static void
ball_update(void)
{
  ball_collision_walls();
  ball_collision_paddle();
  ball_collision_bricks();
  ball_move();
}

static void
bricks_draw(u8* screen)
{
  i32 i = 0;
  for (; i < eBricks; ++i)
  {
    if (!g_game.brick[i].hit)
    {
      rectangle_fill_draw(screen,
                          g_game.brick[i].position.xy.x,
                          g_game.brick[i].position.xy.y,
                          g_game.brick[i].position.xy.x + g_game.brick[i].size.wh.width,
                          g_game.brick[i].position.xy.y + g_game.brick[i].size.wh.height,
                          g_game.brick[i].color);
    }
  }
}

static void
paddle_draw(u8* screen)
{
  rectangle_fill_draw(screen,
                      g_game.player.position.xy.x,
                      g_game.player.position.xy.y,
                      g_game.player.position.xy.x + ePaddleWidth,
                      g_game.player.position.xy.y + ePaddleHeight,
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
ball_draw(u8* screen)
{
  rectangle_fill_draw(screen,
                      g_game.ball.position.xy.x,
                      g_game.ball.position.xy.y,
                      g_game.ball.position.xy.x + eBallWidth,
                      g_game.ball.position.xy.y + eBallHeight,
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
game_load(ExotiqueInterface* ei)
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
  g_game.player.position.xy.x = kScreenWidth / 2 - ePaddleWidth / 2;
  g_game.player.position.xy.y = kScreenHeight - 6;
  g_game.player.score = 0;

  /* Game init */
  g_game.difficulty = 1;
  g_game.shoot = 1;

  /* Bricks init */
  g_game.bricks = eBrickColumns * eBrickRows;
  /* Bricks block is set at position (8, 28) */
  {
    i32 n = 0;
    i32 color = 0;

    i32 i = 0;
    for (; i < eBrickRows; ++i)
    {
      i32 j = 0;
      for (; j < eBrickColumns; ++j)
      {
        g_game.brick[n].position.xy.x = 8 + (j * eBrickWidth);
        g_game.brick[n].position.xy.y = 28 + (i * eBrickHeight);
        g_game.brick[n].size.wh.width = eBrickWidth;
        g_game.brick[n].size.wh.height = eBrickHeight;
        g_game.brick[n].color = (color_t)((color % 6) + 2);
        g_game.brick[n].hit = 0;
        ++n;
      }
      ++color;
    }
  }
}

static void
paddle_update(i32 mouse_x)
{
  g_game.player.position.xy.x = mouse_x;

  /* Keeping the paddle within the boundaries of the game area */
  if (g_game.player.position.xy.x < 8)
  {
    g_game.player.position.xy.x = 8;
  }
  if (g_game.player.position.xy.x > 136)
  {
    g_game.player.position.xy.x = 136;
  }
}

void
game_update(ExotiqueInterface* ei)
{
  ball_update();
  paddle_update(ei->mouse.xy.x);
}

void
game_draw(ExotiqueInterface* ei)
{
  memzero(ei->screen, (u64)kScreenPixels);

  walls_draw(ei->screen);
  bricks_draw(ei->screen);
  paddle_draw(ei->screen);
  ball_draw(ei->screen);

  numbers_draw(ei->screen, 36, 2, g_game.player.score, 3, eColorWhite);
}
