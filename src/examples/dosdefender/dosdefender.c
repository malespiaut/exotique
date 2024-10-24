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

/* XXX: DOS Defender data structures */

#define kScale 1000

/* typedef void (*power_t)(int id); */

struct ship
{
  long int x;
  long int y;
  long int dx;
  long int dy;
  unsigned int last_fire;
  /* struct sample *fx_fire; */
  unsigned short score;
  unsigned short hp;
  unsigned short hp_max;
  unsigned char radius;
  unsigned char fire_delay;
  unsigned char fire_damage;
  unsigned char drop_rate;
  unsigned char color_a;
  unsigned char color_b;
  char is_player; /* bool */
  union
  {
    int target_ship;
    struct
    {
      unsigned long int x;
      unsigned long int y;
    } target_position;
  };
};

struct bullet
{
  long int x;
  long int y;
  long int dx;
  long int dy;
  unsigned int birthtick;
  unsigned char color;
  unsigned char damage;
  char alive; /* bool */
};

struct particle
{
  long int x;
  long int y;
  unsigned int birthtick;
  char alive; /* bool */
};

struct powerup
{
  long int x;
  long int y;
  unsigned int birthtick;
  unsigned char color;
  char alive; /* bool */
  /* power_t power; */
};

static struct bullet* bullets;
static unsigned short bullets_max = 32;

static struct particle* particles;
static unsigned short particles_max = 64;

static struct ship* ships;
static unsigned short ships_max = 12;

static struct powerup* powerups;
static unsigned short powerups_max = 8;

/* XXX: 32-bits PRNG - xoshiro128++ */

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

/* XXX: DOS Defender functions */

static int
spawn(int hp)
{
  int choice = -1;

  int i = 0;
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
  int id = spawn(0);
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
  if (!(next() % 50))
  {
    try_spawn();
  }
  /*
  int i;
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
