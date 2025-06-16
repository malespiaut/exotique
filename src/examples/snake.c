/* Coded by DosX */
/* GitHub: https://github.com/DosX-dev */

#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 25;
const i32 kScreenHeight = 25;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* XXX: C library reimplementation */

enum button_e
{
  bUp,
  bDown,
  bLeft,
  bRight,
  bSelect,
  bStart,
  bA,
  bB,
  bX,
  bY,
  bL1,
  bR1,
  bL2,
  bR2,
  bL3,
  bR3
};
typedef enum button_e button_t;

/* XXX: Defines */

enum color_e
{
  eColorTransparent,
  eColorRipePlum,
  eColorDisco,
  eColorTorchRed,
  eColorCrusta,
  eColorBrightSun,
  eColorLima,
  eColorOceanGreen,
  eColorChambray,
  eColorBlack,
  eColorBlueRibbon,
  eColorDodgerBlue,
  eColorBrightTurquoise,
  eColorSeashellPeach,
  eColorFrenchGray,
  eColorBrownRust,
  eColorSoyaBean
};
typedef enum color_e color_t;

#define kSpeedDelay 100

#define true 1
#define false 0

typedef struct Context Context;
struct Context
{
  /*
  bool wants_quit;
  bool hide_cursor;
  */
  /* input */
  /*
  int char_buf[32];
  uint8_t key_state[256];
  uint8_t mouse_state[16];
  struct
  {
    int x, y;
  } mouse_pos;
  struct
  {
    int x, y;
  } mouse_delta;
  */
  /* time */
  f64 step_time;
  f64 prev_time;
  /* graphics */
  /*
  kit_Rect clip;
  kit_Font* font;
  kit_Image* screen;
  */
  /* windows */
  /*
  int win_w, win_h;
  HWND hwnd;
  HDC hdc;
  */
  u32 speed;
};

Context g_context = {0};

/* XXX: Input functions */

/* bit_get() as a macro */
#define bit_get(data, n) (((n) < 32) ? (((data) >> (n)) & 1) : 0)

/* bit_get() as a function */
/*
static u8
bit_get(u32 data, u8 n)
{
  if (n < 32) // Ensure n is within 0-31
  {
    return (data >> n) & 1;
  }
  return 0; // Return 0 if n is out of range
}
*/

/* XXX: Time function */

void
step(ExotiqueInterface* ei, f64* dt)
{
  f64 now = (f64)ei->ticks / 1000.0;
  f64 wait = (g_context.prev_time + g_context.step_time) - now; /*xxx*/
  f64 prev = g_context.prev_time;

  /*
  now = kit__now();
  wait = (g_context.prev_time + g_context.step_time) - now;
  prev = g_context.prev_time;
  */

  if (wait > 0)
  {
    /*Sleep(wait * 1000);*/
    g_context.prev_time += g_context.step_time;
  }
  else
  {
    g_context.prev_time = now;
  }
  if (dt)
  {
    *dt = g_context.prev_time - prev;
  }
}

/* XXX: Drawing functions */

static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
  }
}

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

i8 gameover = false;
i32 score = 0;
i32 snakeX[32768] = {0};
i32 snakeY[32768] = {0};
i32 snake_length = 1;
i32 foodX = 0;
i32 foodY = 0;
i32 dirX = 0;
i32 dirY = 0;

/* Function to handle user input */
void
input(u32 buttons)
{
  if (bit_get(buttons, bUp))
  {
    if (dirX != 1)
    {
      dirX = -1; /* Move left */
      dirY = 0;
    }
    return;
  }

  if (bit_get(buttons, bDown))
  {
    if (dirX != -1)
    {
      dirX = 1; /* Move right */
      dirY = 0;
    }
    return;
  }

  if (bit_get(buttons, bLeft))
  {
    if (dirY != 1)
    {
      dirX = 0;
      dirY = -1; /* Move up */
    }
    return;
  }

  if (bit_get(buttons, bRight))
  {
    if (dirY != -1)
    {
      dirX = 0;
      dirY = 1; /* Move down */
    }
    return;
  }
}

/* Function to update the game logic */
void
logic(void)
{
  i32 prevX = snakeX[0];
  i32 prevY = snakeY[0];
  i32 prev2X = 0;
  i32 prev2Y = 0;
  i32 i = 0;
  i8 spawnOnBody = true;

  snakeX[0] += dirX;
  snakeY[0] += dirY;

  /* Teleport the snake to the opposite side of the map if it hits a wall */
  if (!snakeX[0])
  {
    snakeX[0] = kScreenWidth - 2;
  }
  else if (snakeX[0] == kScreenWidth - 1)
  {
    snakeX[0] = 1;
  }

  if (!snakeY[0])
  {
    snakeY[0] = kScreenHeight - 2;
  }
  else if (snakeY[0] == kScreenHeight - 1)
  {
    snakeY[0] = 1;
  }

  for (i = 1; i < snake_length; i++)
  {
    prev2X = snakeX[i];
    prev2Y = snakeY[i];
    snakeX[i] = prevX;
    snakeY[i] = prevY;
    prevX = prev2X;
    prevY = prev2Y;
  }

  /* Check if the snake eats the food */
  if (snakeX[0] == foodX && snakeY[0] == foodY)
  {
    /* Generate new random coordinates for food within the game field (excluding the borders) */
    do
    {
      foodX = (i32)(1 + next() % (u32)(kScreenWidth - 2));
      foodY = (i32)(1 + next() % (u32)(kScreenHeight - 2));
    } while (foodX == snakeX[0] && foodY == snakeY[0]);

    /* Check if the food spawns on the snake's body */
    spawnOnBody = true;
    for (i = 1; i < snake_length; i++)
    {
      if (foodX == snakeX[i] && foodY == snakeY[i])
      {
        spawnOnBody = true;
        break;
      }
    }

    /* If the food spawns on the snake's body, try generating new coordinates again */
    while (spawnOnBody)
    {
      foodX = (i32)(1 + next() % (u32)(kScreenWidth - 2));
      foodY = (i32)(1 + next() % (u32)(kScreenHeight - 2));

      spawnOnBody = false;
      for (i = 0; i < snake_length; i++)
      {
        /* If the coordinates of the Apple are on the body of the snake, then the cycle continues */
        if (foodX == snakeX[i] && foodY == snakeY[i])
        {
          spawnOnBody = true;
          break;
        }
      }
    }

    score += 10;    /* Increase the score */
    ++snake_length; /* Increase the length of the snake */
  }

  /* Check for collision with itself */
  for (i = 1; i < snake_length; i++)
  {
    if (snakeX[i] == snakeX[0] && snakeY[i] == snakeY[0])
    {
      gameover = true; /* Game over if the snake collides with itself */
      break;
    }
  }
}

/* XXX: Exotique core functions */

void
game_load(ExotiqueInterface* ei)
{
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

  snakeX[0] = kScreenWidth / 2;
  snakeY[0] = kScreenHeight / 2; /* Set snake head at center of game zone */

  foodX = (i32)(1 + next() % (u32)(kScreenWidth - 2));
  foodY = (i32)(1 + next() % (u32)(kScreenHeight - 2));

  dirX = 0;
  dirY = 1; /* Move down */

  g_context.speed = 1;
}

void
game_update(ExotiqueInterface* ei)
{
  /* (void)ei; */
  input(ei->player[0].buttons); /* Handle user input */
  logic();                      /* Update the game logic */
}

void
game_draw(ExotiqueInterface* ei)
{
  i32 i = 0;
  i32 j = 0;
  i32 k = 0;

  memzero(ei->screen, (u64)kScreenPixels);

  for (i = 0; i < kScreenHeight; i++) /* Loop through the game board rows */
  {
    for (j = 0; j < kScreenWidth; j++) /* Loop through the game board columns */
    {
      if (i == 0 || i == kScreenHeight - 1 || j == 0 || j == kScreenWidth - 1) /* Check if it's a border cell */
      {
        color_t border_color = 0;
        if (gameover)
        {
          border_color = eColorTorchRed;
        }
        else
        {
          border_color = eColorDodgerBlue;
        }

        /* Draw a cyan-colored border */
        pixel_draw(ei->screen, i, j, border_color);
      }
      else if (i == snakeY[0] && j == snakeX[0]) /* Check if it's the snake's head cell */
      {
        /* Draw the snake's head with eyes */
        pixel_draw(ei->screen, i, j, eColorSeashellPeach);
      }
      else if (i == foodY && j == foodX) /* Check if it's the food cell */
      {
        /* Draw an apple (food) */
        pixel_draw(ei->screen, i, j, eColorTorchRed);
      }
      else
      {
        for (k = 1; k < snake_length; k++) /* Loop through the snake's body cells */
        {
          if (snakeX[k] == j && snakeY[k] == i) /* Check if it's a cell in the snake's body */
          {
            /* Draw the snake's body segment */
            pixel_draw(ei->screen, i, j, eColorSoyaBean);
            /*break;*/
          }
        }
      }
    }
  }
}
