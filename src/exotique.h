typedef signed char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64;
typedef float f32;
typedef double f64;

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
  unsigned up : 1;
  unsigned down : 1;
  unsigned left : 1;
  unsigned right : 1;
  unsigned select : 1;
  unsigned start : 1;
  unsigned a : 1;
  unsigned b : 1;
  unsigned x : 1;
  unsigned y : 1;
  unsigned l1 : 1;
  unsigned r1 : 1;
  unsigned l2 : 1;
  unsigned r2 : 1;
  unsigned l3 : 1;
  unsigned r3 : 1;
  vec2i_t joystick;
};

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  u8* screen;   /* [kScreenPixels] */
  u32* palette; /* [255] */

  vec2i_t mouse;
  PlayerInput input[4];

  u64 ticks;
};

void game_load(ExotiqueInterface* ei);
void game_update(ExotiqueInterface* ei);
void game_draw(ExotiqueInterface* ei);
