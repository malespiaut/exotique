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
  eTimeStop = 2000
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

game_t g_game = {0};

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
    i32 y;
    for (y = y1; y < y2; ++y)
    {
      i32 x;
      for (x = x1; x < x2; ++x)
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
ball_handle_wall_collision(void)
{
  if (g_game.ball.position.xy.x + eBallWidth > kScreenWidth)
  {
    g_game.ball.slope.xy.x = -g_game.ball.slope.xy.x;
    g_game.ball.position.xy.x += g_game.ball.slope.xy.x;
  }

  if (g_game.ball.position.xy.x < 0)
  {
    g_game.ball.slope.xy.x = -g_game.ball.slope.xy.x;
    g_game.ball.position.xy.x += g_game.ball.slope.xy.x;
  }

  if (g_game.ball.position.xy.y < 0)
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
ball_handle_paddle_collision(void)
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
ball_handle_brick_collisions(void)
{
  i32 i;

  for (i = 0; i < eBricks; ++i)
  {
    if (g_game.brick[i].hit == 0)
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
ball_reset_position(void)
{
  g_game.ball.position.xy.x = kScreenWidth / 2;
  g_game.ball.position.xy.y = kScreenHeight / 2;
  g_game.shoot = 0;
}

static void
ball_update_position(void)
{
  if (!g_game.shoot)
  {
    g_game.ball.position.xy.x += g_game.ball.speed.xy.x * g_game.ball.slope.xy.x;
    g_game.ball.position.xy.y += g_game.ball.speed.xy.y * g_game.ball.slope.xy.y;
  }
  else
  {
    ball_reset_position();
  }
}

static void
ball_move(void)
{
  ball_handle_wall_collision();
  ball_handle_paddle_collision();
  ball_handle_brick_collisions();
  ball_update_position();
}

/* Exotique engine interface functions */

static void
setup_color_palette(ExotiqueInterface* ei)
{
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
}

static void
initialize_bricks(void)
{
  i32 i, j, n = 0, color = 0;

  for (i = 0; i < eBrickRows; ++i)
  {
    for (j = 0; j < eBrickColumns; ++j)
    {
      g_game.brick[n].position.xy.x = j * eBrickWidth;
      g_game.brick[n].position.xy.y = i * eBrickHeight;
      g_game.brick[n].size.wh.width = eBrickWidth;
      g_game.brick[n].size.wh.height = eBrickHeight;
      g_game.brick[n].color = (color_t)((color % 6) + 2);
      g_game.brick[n].hit = 0;
      ++n;
    }
    ++color;
  }
}

static void
reset_game_state(void)
{
  g_game.shoot = 1;
  g_game.ball.position.xy.x = kScreenWidth / 2;
  g_game.ball.position.xy.y = kScreenHeight / 2;
  g_game.player.position.xy.x = kScreenWidth / 2 - ePaddleWidth / 2;
  g_game.player.position.xy.y = kScreenHeight - 6;
  g_game.ball.speed.xy.x = 1;
  g_game.ball.speed.xy.y = 1;
  g_game.ball.slope.xy.x = -1;
  g_game.ball.slope.xy.y = -1;
}

void
game_load(ExotiqueInterface* ei)
{
  setup_color_palette(ei);

  g_game.difficulty = 1;
  g_game.player.lives = 3;
  g_game.player.score = 0;
  g_game.bricks = eBrickColumns * eBrickRows;

  initialize_bricks();
  reset_game_state();
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
  ball_move();
  g_game.player.position.xy.x = ei->mouse.xy.x;
}

static void
draw_bricks(u8* screen)
{
  i32 i;

  for (i = 0; i < eBricks; ++i)
  {
    if (g_game.brick[i].hit == 0)
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
draw_paddle(u8* screen)
{
  rectangle_fill_draw(screen,
                      g_game.player.position.xy.x,
                      g_game.player.position.xy.y,
                      g_game.player.position.xy.x + ePaddleWidth,
                      g_game.player.position.xy.y + ePaddleHeight,
                      eColorWhite);
}

static void
draw_ball(u8* screen)
{
  rectangle_fill_draw(screen,
                      g_game.ball.position.xy.x,
                      g_game.ball.position.xy.y,
                      g_game.ball.position.xy.x + eBallWidth,
                      g_game.ball.position.xy.y + eBallHeight,
                      eColorWhite);
}

void
game_draw(ExotiqueInterface* ei)
{
  memzero(ei->screen, (u64)kScreenPixels);

  draw_bricks(ei->screen);
  draw_paddle(ei->screen);
  draw_ball(ei->screen);
}
