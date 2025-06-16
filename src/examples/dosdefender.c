#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 160;
const i32 kScreenHeight = 100;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* XXX: C library reimplementation */

static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
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

/* XXX: Color palette */

enum ePaletteIndex
{
  eColorTransparent,
  eColorBlack,
  eColorDarkBlue,
  eColorBordeaux,
  eColorDarkGreen,
  eColorBrown,
  eColorDarkGray,
  eColorLightGray,
  eColorWhite,
  eColorRed,
  eColorOrange,
  eColorYellow,
  eColorGreen,
  eColorCyan,
  eColorMauve,
  eColorPink,
  eColorFlesh
};
typedef enum ePaletteIndex ePaletteIndex;

/* XXX: DOS Defender data structures */

#define kScale 1000

/* typedef void (*power_t)(i32 id); */

struct ship
{
  i32 x;
  i32 y;
  i32 dx;
  i32 dy;
  u32 last_fire;
  /* struct sample *fx_fire; */
  u16 score;
  u16 hp;
  u16 hp_max;
  u8 radius;
  u8 fire_delay;
  u8 fire_damage;
  u8 drop_rate;
  u8 color_a;
  u8 color_b;
  i8 is_player; /* bool */
  union
  {
    i32 target_ship;
    struct
    {
      u32 x;
      u32 y;
    } target_position;
  } u;
};

struct bullet
{
  i32 x;
  i32 y;
  i32 dx;
  i32 dy;
  u32 birthtick;
  u8 color;
  u8 damage;
  i8 alive; /* bool */
};

struct particle
{
  i32 x;
  i32 y;
  u32 birthtick;
  i8 alive; /* bool */
};

struct powerup
{
  i32 x;
  i32 y;
  u32 birthtick;
  u8 color;
  i8 alive; /* bool */
  /* power_t power; */
};

static struct bullet* bullets;
static u16 bullets_max = 32;

static struct particle* particles;
static u16 particles_max = 64;

static struct ship* ships;
static u16 ships_max = 12;

/*static struct powerup* powerups;*/
/*static u16 powerups_max = 8;*/

/* XXX: DOS Defender functions */

static i32
spawn(i32 hp)
{
  i32 choice = -1;

  i32 i = 0;
  for (i = 0; i < ships_max; ++i)
  {
    if (!ships[i].is_player && !ships[i].hp)
    {
      choice = i;
      break;
    }
  }

  if (choice > -1)
  {
    ships[choice].is_player = false;
    ships[choice].hp = hp;
    ships[choice].dx = 0;
    ships[choice].dy = 0;

    if (next() % 2)
    {
      ships[choice].x = (next() % 2) * kScreenWidth * kScale;
      ships[choice].y = next() % (kScreenHeight * kScale);
    }
    else
    {
      ships[choice].x = next() % (kScreenWidth * kScale);
      ships[choice].y = (next() % 2) * kScreenHeight * kScale;
    }
  }

  return choice;
}

static void
try_spawn(void)
{
  i32 id = spawn(0);
  if (id <= 0)
  {
    return;
  }
  /*TO FINISH*/
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
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
  if (!(next() % 50))
  {
    try_spawn();
  }
  /*
  i32 i;
  for (i = 0; i < kScreenPixels; ++i)
  {
    ei->screen[i] = next() % 17;
  }
  */
}

void
game_draw(ExotiqueInterface* ei)
{
  (void)ei;
}
