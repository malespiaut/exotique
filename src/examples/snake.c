/* Coded by DosX */
/* GitHub: https://github.com/DosX-dev */

#include "exotique.h"

/* Screen size */

const i32 kScreenWidth = 25;
const i32 kScreenHeight = 25;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* Defines */

enum color_e
{
  eColorTransparent,
  eColorRed,
  eColorBlue,
  eColorPeach,
  eColorBean,
  eColor__COUNT
};
typedef enum color_e color_t;

#define true 1
#define false 0

/* Data structures */

typedef struct vec2_s vec2_t;
struct vec2_s
{
  i32 x;
  i32 y;
};

vec2_t food = {0};
vec2_t snake[32768] = {0};
vec2_t direction = {0};

u8 gameover = false;
i32 score = 0;
i32 snake_length = 1;

/* Drawing functions */

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

/* 32-bits PRNG - xoshiro128++ */

/* Completely arbitrary seeds */
static u32 s[4] = {0x27cb588d, 0x096379a9, 0xe81f5914, 0x2ee1c98c};

static /*inline*/ u32
rotl(const u32 x, i32 k)
{
  return (x << k) | (x >> (32 - k));
}

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

/* Function to update the game logic */
void
logic(void)
{
  i32 prevX = snake[0].x;
  i32 prevY = snake[0].y;
  i32 prev2X = 0;
  i32 prev2Y = 0;
  i32 i = 0;
  u8 spawnOnBody = true;

  snake[0].x += direction.x;
  snake[0].y += direction.y;

  /* Teleport the snake to the opposite side of the map if it hits a wall */
  if (!snake[0].x)
  {
    snake[0].x = kScreenWidth - 2;
  }
  else if (snake[0].x == kScreenWidth - 1)
  {
    snake[0].x = 1;
  }

  if (!snake[0].y)
  {
    snake[0].y = kScreenHeight - 2;
  }
  else if (snake[0].y == kScreenHeight - 1)
  {
    snake[0].y = 1;
  }

  for (i = 1; i < snake_length; i++)
  {
    prev2X = snake[i].x;
    prev2Y = snake[i].y;
    snake[i].x = prevX;
    snake[i].y = prevY;
    prevX = prev2X;
    prevY = prev2Y;
  }

  /* Check if the snake eats the food */
  if (snake[0].x == food.x && snake[0].y == food.y)
  {
    /* Generate new random coordinates for food within the game field (excluding the borders) */
    do
    {
      food.x = (i32)(1 + next() % (u32)(kScreenWidth - 2));
      food.y = (i32)(1 + next() % (u32)(kScreenHeight - 2));
    } while (food.x == snake[0].x && food.y == snake[0].y);

    /* Check if the food spawns on the snake's body */
    spawnOnBody = true;
    for (i = 1; i < snake_length; i++)
    {
      if (food.x == snake[i].x && food.y == snake[i].y)
      {
        spawnOnBody = true;
        break;
      }
    }

    /* If the food spawns on the snake's body, try generating new coordinates again */
    while (spawnOnBody)
    {
      food.x = (i32)(1 + next() % (u32)(kScreenWidth - 2));
      food.y = (i32)(1 + next() % (u32)(kScreenHeight - 2));

      spawnOnBody = false;
      for (i = 0; i < snake_length; i++)
      {
        /* If the coordinates of the Apple are on the body of the snake, then the cycle continues */
        if (food.x == snake[i].x && food.y == snake[i].y)
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
    if (snake[i].x == snake[0].x && snake[i].y == snake[0].y)
    {
      gameover = true; /* Game over if the snake collides with itself */
      break;
    }
  }
}

/* Exotique core functions */

void
game_init(ExotiqueInterface* ei)
{
  /* Setting up the color palette */
  ei->palette[eColorTransparent] = 0x00000000;
  ei->palette[eColorRed] = 0xff004dff;
  ei->palette[eColorBlue] = 0x29adffff;
  ei->palette[eColorPeach] = 0xfff1e8ff;
  ei->palette[eColorBean] = 0x5f574fff;

  snake[0].x = kScreenWidth / 2;
  snake[0].y = kScreenHeight / 2; /* Set snake head at center of game zone */

  food.x = (i32)(1 + next() % (u32)(kScreenWidth - 2));
  food.y = (i32)(1 + next() % (u32)(kScreenHeight - 2));

  direction.x = 0;
  direction.y = 1; /* Move down */
}

void
game_update(ExotiqueInterface* ei)
{
  if (ei->input[0].up)
  {
    if (direction.x != 1)
    {
      direction.x = -1; /* Move left */
      direction.y = 0;
    }
    return;
  }

  if (ei->input[0].down)
  {
    if (direction.x != -1)
    {
      direction.x = 1; /* Move right */
      direction.y = 0;
    }
    return;
  }

  if (ei->input[0].left)
  {
    if (direction.y != 1)
    {
      direction.x = 0;
      direction.y = -1; /* Move up */
    }
    return;
  }

  if (ei->input[0].right)
  {
    if (direction.y != -1)
    {
      direction.x = 0;
      direction.y = 1; /* Move down */
    }
    return;
  }

  logic(); /* Update the game logic */
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
          border_color = eColorRed;
        }
        else
        {
          border_color = eColorBlue;
        }

        /* Draw a cyan-colored border */
        pixel_draw(ei->screen, i, j, border_color);
      }
      else if (i == snake[0].y && j == snake[0].x) /* Check if it's the snake's head cell */
      {
        /* Draw the snake's head with eyes */
        pixel_draw(ei->screen, i, j, eColorPeach);
      }
      else if (i == food.y && j == food.x) /* Check if it's the food cell */
      {
        /* Draw an apple (food) */
        pixel_draw(ei->screen, i, j, eColorRed);
      }
      else
      {
        for (k = 1; k < snake_length; k++) /* Loop through the snake's body cells */
        {
          if (snake[k].x == j && snake[k].y == i) /* Check if it's a cell in the snake's body */
          {
            /* Draw the snake's body segment */
            pixel_draw(ei->screen, i, j, eColorBean);
            /*break;*/
          }
        }
      }
    }
  }
}
