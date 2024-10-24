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
#define kScreenWidth 320
#endif
#ifndef kScreenHeight
#define kScreenHeight 240
#endif
#define kScreenPixels (kScreenWidth * kScreenHeight)

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  u8* screen;   /* [kScreenPixels] */
  u32* palette; /* [255] */
};

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
bit_draw(ExotiqueInterface* ei, u8* sprite, i32 x, i32 y, i32 width, i32 height, u8 color)
{
  i32 col;
  i32 row;
  for (row = 0; row < width; ++row)
  {
    for (col = 0; col < height; ++col)
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
  bit_draw(ei, tile, 20, 20, 8, 8, 5);
}

void
game_draw(ExotiqueInterface* ei)
{
  (void)ei;
}
