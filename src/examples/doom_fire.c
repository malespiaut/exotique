#include "exotique.h"

/* Screen size */

const i32 kScreenWidth = 320;
const i32 kScreenHeight = 200;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* 32-bits PRNG - xoshiro128++ */

static /*inline*/ u32
rotl(const u32 x, i32 k)
{
  return (x << k) | (x >> (32 - k));
}

/* Completely arbitrary seeds */
static u32 s[4] = {0x27cb588d, 0x096379a9, 0xe81f5914, 0x2ee1c98c};

u32
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

/* Doom fire */

static void
fire_spread(u8* screen, i32 src)
{
  i32 random_shift;
  i32 current_x;
  i32 new_x;
  i32 dst;
  u8 new_intensity;

  /* Get the current pixel's intensity */
  u8 pixel_intensity = screen[src];

  /* If the current pixel has no intensity, set the pixel above to zero*/
  if (pixel_intensity == 0)
  {
    screen[src - kScreenWidth] = 0;
    return;
  }

  /* Create a small random shift (-1, 0, or 1) */
  random_shift = (i32)((next() % 3) - 1);

  /* Calculate new x-position with the random shift */
  current_x = src % kScreenWidth;
  new_x = current_x + random_shift;

  /* Keep new_x within screen bounds */
  if (new_x < 0)
  {
    new_x = 0;
  }
  if (new_x >= kScreenWidth)
  {
    new_x = kScreenWidth - 1;
  }

  /* Calculate the destination pixel's position (one row above) */
  dst = (src - current_x) + new_x - kScreenWidth;

  /* Skip if we're trying to modify the top row */
  if (dst < kScreenWidth)
  {
    return;
  }

  /* Randomly decrease the pixel intensity by 0 or 1 */
  new_intensity = (u8)(pixel_intensity - (u8)(next() % 2));
  if (new_intensity > pixel_intensity)
  {
    new_intensity = 0; /* Handle underflow */
  }

  /* Set the new pixel intensity */
  screen[dst] = new_intensity;
}

static void
fire_draw(u8* screen)
{
  i32 x;
  for (x = 0; x < kScreenWidth; ++x)
  {
    i32 y;
    for (y = 1; y < kScreenHeight; ++y)
    {
      fire_spread(screen, (u64)(y * kScreenWidth + x));
    }
  }
}

/* Exotique interface functions */

void
game_init(ExotiqueInterface* ei)
{
  /* Setting up the color palette */
  ei->palette[0] = 0x00000000;
  ei->palette[1] = 0x070707ff;
  ei->palette[2] = 0x1f0707ff;
  ei->palette[3] = 0x2f0f07ff;
  ei->palette[4] = 0x470f07ff;
  ei->palette[5] = 0x571707ff;
  ei->palette[6] = 0x671f07ff;
  ei->palette[7] = 0x771f07ff;
  ei->palette[8] = 0x8f2707ff;
  ei->palette[9] = 0x9f2f07ff;
  ei->palette[10] = 0xaf3f07ff;
  ei->palette[11] = 0xbf4707ff;
  ei->palette[12] = 0xc74707ff;
  ei->palette[13] = 0xdf4f07ff;
  ei->palette[14] = 0xdf5707ff;
  ei->palette[15] = 0xdf5707ff;
  ei->palette[16] = 0xd75f07ff;
  ei->palette[17] = 0xd75f07ff;
  ei->palette[18] = 0xd7670fff;
  ei->palette[19] = 0xcf6f0fff;
  ei->palette[20] = 0xcf770fff;
  ei->palette[21] = 0xcf7f0fff;
  ei->palette[22] = 0xcf8717ff;
  ei->palette[23] = 0xc78717ff;
  ei->palette[24] = 0xc78f17ff;
  ei->palette[25] = 0xc7971fff;
  ei->palette[26] = 0xbf9f1fff;
  ei->palette[27] = 0xbf9f1fff;
  ei->palette[28] = 0xbfa727ff;
  ei->palette[29] = 0xbfa727ff;
  ei->palette[30] = 0xbfaf2fff;
  ei->palette[31] = 0xb7af2fff;
  ei->palette[32] = 0xb7b72fff;
  ei->palette[33] = 0xb7b737ff;
  ei->palette[34] = 0xcfcf6fff;
  ei->palette[35] = 0xdfdf9fff;
  ei->palette[36] = 0xefefc7ff;
  ei->palette[37] = 0xffffffff;

  {
    i32 x;
    i32 y = kScreenHeight - 1;
    for (x = 0; x < kScreenWidth; ++x)
    {
      ei->screen[y * kScreenWidth + x] = 37;
    }
  }
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
}

void
game_draw(ExotiqueInterface* ei)
{
  fire_draw(ei->screen);
}
