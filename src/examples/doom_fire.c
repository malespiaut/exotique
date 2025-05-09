#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 320;
const i32 kScreenHeight = 200;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* XXX: 32-bits PRNG - xoshiro128++ */

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

/* Bit draw */

static u8 tile[8] = {0x00, 0x3c, 0x42, 0x7e, 0x40, 0x42, 0x3c, 0x00};

void
sprite_draw(ExotiqueInterface* ei, u8* sprite, i32 x, i32 y, u8 color)
{
  i32 col;
  i32 row;
  for (row = 0; row < 8; ++row)
  {
    for (col = 0; col < 8; ++col)
    {
      if (sprite[row] << col & 0x80)
      {
        if ((x + col) >= 0 && (x + col <= kScreenWidth) && (y + row) >= 0 && (y + row) <= kScreenHeight)
        {
          ei->screen[(x + col) + kScreenWidth * (y + row)] = color;
        }
      }
    }
  }
}

/* XXX: Exotique interface functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Setting up the color palette */
  ei->palette[0] = 0x00000000;
  ei->palette[1] = 0x000000ff;
  ei->palette[2] = 0x430067ff;
  ei->palette[3] = 0x94216aff;
  ei->palette[4] = 0xff004dff;
  ei->palette[5] = 0xff8426ff;
  ei->palette[6] = 0xffdd34ff;
  ei->palette[7] = 0x50e112ff;
  ei->palette[8] = 0x3fa66fff;
  ei->palette[9] = 0x365987ff;
  ei->palette[10] = 0x0033ffff;
  ei->palette[11] = 0x29adffff;
  ei->palette[12] = 0x00ffccff;
  ei->palette[13] = 0xfff1e8ff;
  ei->palette[14] = 0xc2c3c7ff;
  ei->palette[15] = 0xab5236ff;
  ei->palette[16] = 0x5f574fff;
}

void
game_update(ExotiqueInterface* ei)
{
  sprite_draw(ei, tile, 20, 20, 5);
}

void
game_draw(ExotiqueInterface* ei)
{
  (void)ei;
}
