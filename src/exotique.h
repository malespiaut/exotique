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
  u32 buttons;
  vec2i_t joystick;
};

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  u8* screen;   /* [kScreenPixels] */
  u32* palette; /* [255] */

  vec2i_t mouse;
  PlayerInput player[4];

  u64 ticks;
};

void game_load(ExotiqueInterface* ei);
void game_update(ExotiqueInterface* ei);
void game_draw(ExotiqueInterface* ei);
