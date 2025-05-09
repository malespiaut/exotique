#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 160;
const i32 kScreenHeight = 100;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* XXX: Defines */

enum color_e
{
  eColorTransparent,
  eColorBlack,
  eColorWhite
};
typedef enum color_e color_t;

#define kSpeedDelay 100

#define true 1
#define false 0
#define NULL (void*)0

/* XXX: Structures */

typedef struct Rectangle Rectangle;
struct Rectangle
{
  i32 x;
  i32 y;
  i32 w;
  i32 h;
};

typedef struct Entity Entity;
struct Entity
{
  f32 x;
  f32 y;
  f32 vx;
  f32 vy;
  f32 frame;
};

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
};

Entity g_entity = {0};
Context g_context = {0};

/* clang-format off */

/* XXX: Data */
/*
static u8 img_sprite[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
  0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9,
  0xfe, 0x70, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd7, 0xeb,
  0xf0, 0xfe, 0x70, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x83,
  0xc1, 0xf9, 0xf9, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc7, 0xe3, 0xff, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xef, 0xf7, 0xf9, 0xf9, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xfd, 0xf9, 0xff, 0xf0, 0xff, 0xdf, 0x6f, 0xbf, 0xfb, 0xed,
  0xf7, 0xff, 0x7d, 0xbe, 0xdf, 0x7f, 0xef, 0xe0, 0x00, 0x00, 0x2f, 0xb7, 0xd0,
  0x05, 0xf4, 0x00, 0x00, 0x00, 0x5f, 0x6f, 0xa0, 0x00, 0x7f, 0x3f, 0x80, 0x00,
  0x07, 0xf0, 0x01, 0xfc, 0xfe, 0x7f, 0x00, 0x00, 0x0f, 0xe0, 0x7f, 0x3f, 0x9f,
  0xcf, 0xe7, 0xf3, 0xf9, 0xfc, 0xfe, 0x7f, 0x3f, 0x9f, 0xcf, 0xe0, 0x7f, 0x3f,
  0x9f, 0xcf, 0xe7, 0xf3, 0xf9, 0xfc, 0xfe, 0x7f, 0x3f, 0x9f, 0xcf, 0xe0, 0x7f,
  0x3f, 0x9f, 0xcf, 0xe7, 0xf3, 0xf9, 0xfc, 0xfe, 0x5d, 0x3f, 0x9f, 0xcf, 0xe0,
  0x5d, 0x2e, 0x9f, 0xcb, 0xa5, 0xd3, 0xf9, 0xfc, 0xba, 0x2a, 0x2e, 0x97, 0x4b,
  0xa0, 0x5d, 0x2e, 0x97, 0x4b, 0xa5, 0xd2, 0xe9, 0x74, 0xba, 0x7f, 0x15, 0x0a,
  0x85, 0x40, 0x7f, 0x3f, 0x97, 0x4f, 0xe7, 0xf2, 0xe9, 0x74, 0xfe, 0x7f, 0x3f,
  0x9f, 0xcf, 0xe0, 0x22, 0x11, 0x1f, 0xcf, 0xe0, 0x23, 0xf8, 0x80, 0x10, 0x22,
  0x3f, 0x9f, 0xc4, 0x40, 0xaa, 0xd5, 0x48, 0x84, 0x4f, 0xa8, 0x02, 0xbf, 0xc7,
  0xaa, 0x91, 0x08, 0x95, 0x50, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

static u8 img_sprite_mask[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
  0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x14,
  0x0f, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c,
  0x3e, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x38, 0x1c, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x10, 0x08, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x02, 0x06, 0x00, 0x00, 0x00, 0x20, 0x90, 0x40, 0x04, 0x12,
  0x08, 0x00, 0x82, 0x41, 0x20, 0x80, 0x10, 0x10, 0xff, 0xff, 0xd0, 0x48, 0x2f,
  0xfa, 0x0b, 0xff, 0xff, 0xff, 0xa0, 0x90, 0x5f, 0xf0, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xf0, 0x77, 0x3b, 0xbf, 0xff, 0xf0, 0x77, 0xfd, 0xc0, 0x38,
  0x77, 0x7f, 0xff, 0xee, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xf7, 0xff, 0xfe,
  0xfe, 0xff, 0xff, 0xff, 0xff, 0xf0
};

static u8 img_corner[] = {
  0x00, 0x1c, 0x3c, 0x7c, 0x7c, 0x7c
};

static u8 img_corner_mask[] = {
  0xfc, 0xe0, 0xc0, 0x80, 0x80, 0x80
};
*/
static u8 img_cursor[] = {
  0x3c, 0x5c, 0x6c, 0x74, 0x78, 0x74, 0x58, 0x28, 0xf4
};

static u8 img_cursor_mask[] = {
  0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xf8, 0xfc, 0xdc, 0x08
};

/* clang-format on */

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
/*#define in_bound(x, y) ((x) >= 0 && (x) < kScreenWidth && (y) >= 0 && (y) < kScreenHeight)*/

/* Function to draw a pixel on the screen if it is within bounds */
/*
static void
pixel_draw(u8* screen, i32 x, i32 y, u8 color)
{
  if (in_bound(x, y))
  {
    screen[y * kScreenWidth + x] = color;
  }
}
*/

/* XXX: Sprite drawing functions */

static void
bit_draw(ExotiqueInterface* ei, u8* sprite, u8* mask, i32 x, i32 y, i32 width, i32 height)
{
  i32 col;
  i32 row;
  for (row = 0; row < height; ++row)
  {
    for (col = 0; col < width; ++col)
    {
      if (mask[row] << col & 0x80)
      {
        /* if in bounds */
        if ((x + col) >= 0 && (x + col <= kScreenWidth) && (y + row) >= 0 && (y + row) <= kScreenHeight)
        {
          if (sprite[row] << col & 0x80)
          {
            ei->screen[(x + col) + kScreenWidth * (y + row)] = eColorWhite;
          }
          else /* if (!(sprite[row] << col & 0x80)) */
          {
            ei->screen[(x + col) + kScreenWidth * (y + row)] = eColorBlack;
          }
        }
      }
    }
  }
}

/* XXX: 32-bits PRNG - xoshiro128++ */
/*
static u32
rotl(const u32 x, i32 k)
{
  return (x << k) | (x >> (32 - k));
}
*/
/* Completely arbitrary seeds */
/*
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
*/
/*
static i32
abs(i32 x)
{
  return x < 0 ? -x : x;
}
*/

/* XXX: Maths */

f32
my_fabsf(f32 x)
{
  /* Extract the raw bits of the float into an unsigned long */
  union
  {
    f32 f;
    u64 ul;
  } u;

  u.f = x;

  /* Clear the sign bit (bit 31) by masking with all 1s except the sign bit */
  u.ul &= 0x7FFFFFFFUL;

  return u.f;
}

i32
my_abs(i32 x)
{
  if (x < 0)
  {
    return x * -1;
  }
  else
  {
    return x;
  }
}

/* Function to handle user input */
void
input(/*u32 buttons*/ void)
{
}

enum fps_e
{
  eFPS30 = (1 << 0),
  /* eFPS60 = (1 << 1), */
  eFPS144 = (1 << 2),
  eFPSINF = (1 << 3)
};
typedef enum fps_e fps_t;


static f64
step_time_init(int flags)
{
  if (flags & eFPS30)
  {
    return 1.0 / 30.0;
  }
  if (flags & eFPS144)
  {
    return 1.0 / 144.0;
  }
  if (flags & eFPSINF)
  {
    return 0;
  }
  return 1.0 / 60.0;
}


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

/* Function to update the game logic */
void
logic(ExotiqueInterface* ei)
{
  f64 dt = 0.0;

  step(ei, &dt);

  /* Entity update */
  if (g_entity.x + 5.0f < (f32)ei->mouse.xy.x)
  {
    g_entity.vx += 3.0f;
  }
  else if (g_entity.x - 5.0f > (f32)ei->mouse.xy.x)
  {
    g_entity.vx -= 3.0f;
  }
  else
  {
    g_entity.vx *= 0.9f;
  }

  if (g_entity.y + 5.0f < (f32)ei->mouse.xy.y)
  {
    g_entity.vy += 3.0f;
  }
  else if (g_entity.y - 5.0f > (f32)ei->mouse.xy.y)
  {
    g_entity.vy -= 3.0f;
  }
  else
  {
    g_entity.vy *= 0.9f;
  }

  if (my_fabsf(g_entity.vx) > 30.0f)
  {
    g_entity.vx = 30.0f * (g_entity.vx < 0.0f ? -1.0f : 1.0f);
  }

  if (my_fabsf(g_entity.vy) > 30.0f)
  {
    g_entity.vy = 30.0f * (g_entity.vy < 0.0f ? -1.0f : 1.0f);
  }

  if (ei->mouse.xy.x | ei->mouse.xy.y)
  {
    g_entity.x += g_entity.vx * (f32)dt;
    g_entity.y += g_entity.vy * (f32)dt;
  }
  g_entity.frame += (f32)dt / 0.1f;
}

/* XXX: Image draw */

void
image_draw(Context* ctx, Image* img, i32 x, i32 y, Rectangle src)
{
  // early exit on zero-sized anything
  if (!src.w || !src.w || !dst.w || !dst.h)
  {
    return;
  }
  
 Rectangle dst = {0};

  /* do scaled render */
  int cx1 = ctx->clip.x;
  int cy1 = ctx->clip.y;
  int cx2 = cx1 + ctx->clip.w;
  int cy2 = cy1 + ctx->clip.h;
  int stepx = (src.w << 10) / dst.w;
  int stepy = (src.h << 10) / dst.h;
  int sy = src.y << 10;
  
  int dy = 0;
  int ey = 0;
  int blend_fn = 0;

  kit_Rect dst = {x, y, my_abs(src.w), my_abs(src.h)};

  /* vertical clipping */
  dy = dst.y;
  if (dy < cy1)
  {
    sy += (cy1 - dy) * stepy;
    dy = cy1;
  }
  ey = kit_min(cy2, dst.y + dst.h);

  blend_fn = 1;
  if (mul_color.w != 0xffffffff)
  {
    blend_fn = 2;
  }
  if ((add_color.w & 0xffffff00) != 0xffffff00)
  {
    blend_fn = 3;
  }

  for (; dy < ey; dy++)
  {
    if (dy >= cy1 && dy < cy2)
    {
      int sx = src.x << 10;
      kit_Color* srow = &img->pixels[(sy >> 10) * img->w];
      kit_Color* drow = &ctx->screen->pixels[dy * ctx->screen->w];

      /* horizontal clipping */
      int dx = dst.x;
      if (dx < cx1)
      {
        sx += (cx1 - dx) * stepx;
        dx = cx1;
      }
      int ex = kit_min(cx2, dst.x + dst.w);

      for (; dx < ex; dx++)
      {
        kit_Color* s = &srow[sx >> 10];
        kit_Color* d = &drow[dx];
        switch (blend_fn)
        {
          case 1:
            *d = kit__blend_pixel(*d, *s);
            break;
          case 2:
            *d = kit__blend_pixel2(*d, *s, mul_color);
            break;
          case 3:
            *d = kit__blend_pixel3(*d, *s, mul_color, add_color);
            break;
        }
        sx += stepx;
      }
    }
    sy += stepy;
  }
}



/* XXX: Exotique core functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Setting up the color palette */
  ei->palette[0] = 0x00000000;
  ei->palette[1] = 0x000000ff;
  ei->palette[2] = 0xffffffff;

  /* Set entity at the center of the screen */
  g_entity.x = kScreenWidth / 2;
  g_entity.y = kScreenHeight / 2;
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
  input(/*ei->player[0].buttons*/); /* Handle user input */
  logic(ei);                        /* Update the game logic */
}

void
game_draw(ExotiqueInterface* ei)
{

  i32 col;
  i32 row;
    i32 frame = ((i32)g_entity.frame) % 4;
    Rectangle r;

  memzero(ei->screen, kScreenPixels);

  /* Clear screen */
  for (row = 0; row < kScreenWidth; ++row)
  {
    for (col = 0; col < kScreenHeight; ++col)
    {
      ei->screen[col + kScreenWidth * row] = eColorWhite;
    }
  }

  /* get animation frame and draw entity */
  
    if (my_fabsf((f32)g_entity.x - (f32)ei->mouse.x) < 7.0f && my_fabsf((f32)g_entity.y - (f32)ei->mouse.y) < 7.0f)
    {
      frame += 8;
    }
    else if (ei->mouse.x | ei->mouse.y)
    {
      frame += 4;
    }
     r.x = frame*9;
     r.y = 0;
     r.w = 9;
     r.h = 19;
  image_draw(g_context, sprite, e.x - 4, e.y - 17, r);

  /* Draw cursor */
  bit_draw(ei, img_cursor, img_cursor_mask, (i32)g_entity.x, (i32)g_entity.y, 6, 9);
  bit_draw(ei, img_cursor, img_cursor_mask, ei->mouse.x, ei->mouse.y, 6, 9);
}
