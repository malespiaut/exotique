#if ARCH == 16
typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef long i32;
typedef unsigned long u32;
typedef long long i64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#elif ARCH == 32
typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#elif ARCH == 64
typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64;
typedef float f32;
typedef double f64;
#else
/* PLEASE READ ME!!! */
/* This piece of code IS DESIGNED TO TRIGGER A COMPILTION ERROR! */
/* You MUST define ARCH on the line above (`#define ARCH 64`),
 * or using the compiler argument `-D` (for instance `-D ARCH=64`)
 * to avoid this compilation error. */
extern int ERROR_ARCH_IS_NOT_DEFINED[-1];
#endif

#ifndef kScreenWidth
#define kScreenWidth 160
#endif
#ifndef kScreenHeight
#define kScreenHeight 100
#endif
#define kScreenPixels (kScreenWidth * kScreenHeight)

typedef union vec2i_u vec2i_t;
union vec2i_u
{
  struct
  {
    i32 x;
    i32 y;
  } xy;
  struct
  {
    i32 width;
    i32 height;
  } wh;
};

typedef struct PlayerInput PlayerInput;
struct PlayerInput
{
  u32 buttons;
  vec2i_t joystick;
};

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  u8* screen;   /* [kScreenPixels] */
  u32* palette; /* [255] */

  vec2i_t mouse;
  PlayerInput player[4];

  u64 ticks;
};

void game_load(ExotiqueInterface* ei);
void game_update(ExotiqueInterface* ei);
void game_draw(ExotiqueInterface* ei);

static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
  }
}

/* XXX: 32-bits PRNG - xoshiro128++ */

#if 0
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
#endif

/* XXX: Game structures */

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

/* XXX: Shape functions */

/* Helper macro to ensure that pixel is within screen bounds */
#define in_bound(x, y) ((x) >= 0 && (x) < kScreenWidth && (y) >= 0 && (y) < kScreenHeight)

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
        if (in_bound(x, y))
        {
          screen[y * kScreenWidth + x] = (u8)color;
        }
      }
    }
  }
}

/* XXX: Game functions */

static i32
rectangle_collision(i32 x1, i32 y1, i32 w1, i32 h1, i32 x2, i32 y2, i32 w2, i32 h2)
{
  if ((y1 + h1 < y2) ||  (y1 > y2 + h2) || (x1 + w1 < x2) || (x1 > x2 + w2))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

static void
ball_move(void)
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

  if ((g_game.ball.position.xy.y + eBallHeight == g_game.player.position.xy.y) && (g_game.ball.position.xy.x >= g_game.player.position.xy.x && g_game.ball.position.xy.x <= g_game.player.position.xy.x + ePaddleWidth))
  {
    /*g_game.ball.slope.xy.y *= -1;*/
    g_game.ball.slope.xy.y = -g_game.ball.slope.xy.y; /* ??? */
  }

  {
    i32 i;

    for (i = 0; i < eBricks; ++i)
    {
      if (g_game.brick[i].hit == 0)
      {
        if (rectangle_collision(g_game.ball.position.xy.x + g_game.ball.slope.xy.x, g_game.ball.position.xy.y, eBallWidth, eBallHeight, g_game.brick[i].position.xy.x, g_game.brick[i].position.xy.y, g_game.brick[i].size.wh.width, g_game.brick[i].size.wh.height))
        {
          g_game.brick[i].hit = 1;
          g_game.ball.slope.xy.x = -g_game.ball.slope.xy.x;
          --g_game.bricks;
          g_game.player.score += 10;
        }

        if (rectangle_collision(g_game.ball.position.xy.x, g_game.ball.position.xy.y + g_game.ball.slope.xy.y, eBallWidth, eBallHeight, g_game.brick[i].position.xy.x, g_game.brick[i].position.xy.y, g_game.brick[i].size.wh.width, g_game.brick[i].size.wh.height))
        {
          g_game.brick[i].hit = 1;
          g_game.ball.slope.xy.y = -g_game.ball.slope.xy.y;
          --g_game.bricks;
          g_game.player.score += 10;
        }
      }
    }

    if (!g_game.shoot)
    {
      g_game.ball.position.xy.x += g_game.ball.speed.xy.x * g_game.ball.slope.xy.x;
      g_game.ball.position.xy.y += g_game.ball.speed.xy.y * g_game.ball.slope.xy.y;
    }
    else
    {
      /*ball_stop();*/
      g_game.ball.position.xy.x = kScreenWidth / 2;
      g_game.ball.position.xy.y = kScreenHeight / 2;
      g_game.shoot = 0;
    }
  }
}

/* XXX: Exotique core functions */

void
game_load(ExotiqueInterface* ei)
{
  memzero((u8*)ei->palette, 256 * sizeof(*ei->palette));
  /* Setting up the color palette */
  /* Endesga 8 Palette */
  ei->palette[eColorTransparent] = 0x00000000;
  ei->palette[eColorWhite] = 0xfdfdf8ff;
  ei->palette[eColorRed] = 0xd32734ff;
  ei->palette[eColorOrange] = 0xda7d22ff;
  ei->palette[eColorYellow] = 0xe6da29ff;
  ei->palette[eColorGreen] = 0x28c641ff;
  ei->palette[eColorBlue] = 0x2d93ddff;
  ei->palette[eColorPurple] = 0x7b53adff;
  ei->palette[eColorBlack] = 0x1b1c33ff;

  /* game init */
  g_game.difficulty = 1;
  g_game.player.lives = 3;
  g_game.bricks = eBrickColumns * 6;

  {
    i32 i;
    i32 j;
    i32 n = 0;
    i32 color = 0;

    for (i = 0; i < 6; ++i)
    {
      for (j = 0; j < 16; ++j)
      {
        g_game.brick[n].position.xy.x = j * eBrickWidth;
        g_game.brick[n].position.xy.y = i * eBrickHeight;
        g_game.brick[n].size.wh.width = eBrickWidth;
        g_game.brick[n].size.wh.height = eBrickHeight;
        g_game.brick[n].color = (color_t)((color % 6) + 2);
        ++n;
      }
      ++color;
    }
  }

  /* game reset */
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
game_update(ExotiqueInterface* ei)
{
  (void)ei;
  ball_move();
  g_game.player.position.xy.x = ei->mouse.xy.x;
}

void
game_draw(ExotiqueInterface* ei)
{
  /*(void)ei;*/
  memzero(ei->screen, kScreenPixels);
  {
    i32 i;

    for (i = 0; i < eBricks; ++i)
    {
      if (g_game.brick[i].hit == 0)
      {
        rectangle_fill_draw(ei->screen, g_game.brick[i].position.xy.x, g_game.brick[i].position.xy.y, g_game.brick[i].position.xy.x + g_game.brick[i].size.wh.width, g_game.brick[i].position.xy.y + g_game.brick[i].size.wh.height, g_game.brick[i].color);
      }
    }
  }

  /* Drawing paddle */
  rectangle_fill_draw(ei->screen, g_game.player.position.xy.x, g_game.player.position.xy.y, g_game.player.position.xy.x + ePaddleWidth, g_game.player.position.xy.y + ePaddleHeight, eColorWhite);

  /* Drawing ball */
  rectangle_fill_draw(ei->screen, g_game.ball.position.xy.x, g_game.ball.position.xy.y, g_game.ball.position.xy.x + eBallWidth, g_game.ball.position.xy.y + eBallHeight, eColorWhite);
}
