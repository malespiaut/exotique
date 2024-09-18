#define kScreenWidth 320
#define kScreenHeight 240
#define kScreenPixels (kScreenWidth * kScreenHeight)

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  unsigned char* screen; /* [320 * 240] */
  unsigned int* palette; /* [255] */
};

enum ePaletteIndex
{
  kTransparent,
  kBlack,
  kDarkBlue,
  kBordeaux,
  kDarkGreen,
  kBrown,
  kDarkGray,
  kLightGray,
  kWhite,
  kRed,
  kOrange,
  kYellow,
  kGreen,
  kCyan,
  kMauve,
  kPink,
  kFlesh
};
typedef enum ePaletteIndex ePaletteIndex;

/* 32-bits PRNG - xoshiro128++ */

static /*inline*/ unsigned int
rotl(const unsigned int x, int k)
{
  return (x << k) | (x >> (32 - k));
}

/* Completely arbitrary seeds */
static unsigned int s[4] = {0x27cb588d, 0x096379a9, 0xe81f5914, 0x2ee1c98c};

unsigned int
next(void)
{
  const unsigned int result = rotl(s[0] + s[3], 7) + s[0];

  const unsigned int t = s[1] << 9;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;

  s[3] = rotl(s[3], 11);

  return result;
}

/* MANDATORY functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Setting up the color palette */
  ei->palette[0] = 0x00000000;
  ei->palette[1] = 0x000000ff;
  ei->palette[2] = 0x1d2b53ff;
  ei->palette[3] = 0x7e2553ff;
  ei->palette[4] = 0x008751ff;
  ei->palette[5] = 0xab5236ff;
  ei->palette[6] = 0x5f574fff;
  ei->palette[7] = 0xc2c3c7ff;
  ei->palette[8] = 0xfff1e8ff;
  ei->palette[9] = 0xff004dff;
  ei->palette[10] = 0xffa300ff;
  ei->palette[11] = 0xffec27ff;
  ei->palette[12] = 0x00e436ff;
  ei->palette[13] = 0x29adffff;
  ei->palette[14] = 0x83769cff;
  ei->palette[15] = 0xff77a8ff;
  ei->palette[16] = 0xffccaaff;
}

void
game_update(ExotiqueInterface* ei)
{
  /*
  ei->screen[0] = 0;
  ei->screen[1] = 1;
  ei->screen[2] = 2;
  ei->screen[3] = 3;
  ei->screen[4] = 4;
  ei->screen[5] = 5;
  ei->screen[6] = 6;
  ei->screen[7] = 7;
  ei->screen[8] = 8;
  ei->screen[9] = 9;
  ei->screen[10] = 10;
  ei->screen[11] = 11;
  ei->screen[12] = 12;
  ei->screen[13] = 13;
  ei->screen[14] = 14;
  ei->screen[15] = 15;
  ei->screen[16] = 16;
  */
  int i;
  for (i = 0; i < kScreenPixels; ++i)
  {
    ei->screen[i] = next() % 17;
  }
}

void
game_draw(ExotiqueInterface* ei)
{
  (void)ei;
}
